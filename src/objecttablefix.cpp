#include "objecttablefix.h"
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <QtEndian>
#include <string.h>
#include <stddef.h>
#include "invobject.h"

// SHA-256 of the decompressed pristine (retail, known-buggy) OBJECT.TAB
static const char* s_pristineObjectTabSha256 =
  "1b2b987bb7b1180caa798754d12e8ea96d2ffe426ef61a03ced95b266b8d8edc";

#define OBJECT_TAB_FILENAME "OBJECT.TAB"

// byte offsets of the Kenelm and Korok entries within an ObjectTableEntry's
// valueByRace[] array (11-byte fixed header + AlienRace enum position)
static const int s_kenelmValueOffset = offsetof(ObjectTableEntry, valueByRace) + static_cast<int>(AlienRace::Kenelm);
static const int s_korokValueOffset  = offsetof(ObjectTableEntry, valueByRace) + static_cast<int>(AlienRace::Korok);

/**
 * Returns true if the OBJECT.TAB in the opened game data is byte-identical to
 * the known-buggy pristine retail file.
 */
bool ObjectTableFix::isKnownBuggy(DatLibrary& lib)
{
  QByteArray tab;
  if (!lib.getFileByName(DatFileType::CONVERSE, OBJECT_TAB_FILENAME, tab))
  {
    return false;
  }

  const QByteArray hash = QCryptographicHash::hash(tab, QCryptographicHash::Sha256).toHex();
  return (hash == QByteArray(s_pristineObjectTabSha256));
}

/**
 * Returns a copy of the provided OBJECT.TAB data with the Kenelm and Korok
 * value bytes exchanged in every record.
 */
QByteArray ObjectTableFix::fixedTable(const QByteArray& pristine)
{
  QByteArray fixed = pristine;
  const int entrySize = sizeof(ObjectTableEntry);

  for (int base = 0; (base + entrySize) <= fixed.size(); base += entrySize)
  {
    const char kenelm = fixed[base + s_kenelmValueOffset];
    fixed[base + s_kenelmValueOffset] = fixed[base + s_korokValueOffset];
    fixed[base + s_korokValueOffset] = kenelm;
  }

  return fixed;
}

/**
 * Locates CONVERSE.DAT in the game directory, matching the filename
 * case-insensitively (same convention as DatLibrary::openData()).
 * @return Full path to the file, or an empty string if not found.
 */
QString ObjectTableFix::findConverseDat(const QString& gameDir)
{
  const QDir dir(gameDir);
  foreach (const QString& entry, dir.entryList(QDir::Files))
  {
    if (entry.compare(DAT_FILENAME_CONVERSE, Qt::CaseInsensitive) == 0)
    {
      return dir.filePath(entry);
    }
  }
  return QString();
}

/**
 * Backs up CONVERSE.DAT to CONVERSE.DAT.orig, then rewrites the container with
 * the corrected OBJECT.TAB appended uncompressed and the index entry repointed.
 * @return True on success; false (with errorMsg set) otherwise.
 */
bool ObjectTableFix::apply(const QString& gameDir, QString& errorMsg)
{
  // re-read the game data independently so this function is self-contained
  DatLibrary lib;
  if (!lib.openData(gameDir))
  {
    errorMsg = "Could not open the game data directory.";
    return false;
  }

  QByteArray pristineTab;
  if (!lib.getFileByName(DatFileType::CONVERSE, OBJECT_TAB_FILENAME, pristineTab))
  {
    errorMsg = "Could not read OBJECT.TAB from CONVERSE.DAT.";
    return false;
  }

  if (QCryptographicHash::hash(pristineTab, QCryptographicHash::Sha256).toHex() !=
      QByteArray(s_pristineObjectTabSha256))
  {
    errorMsg = "OBJECT.TAB does not match the known-buggy original file; refusing to modify it.";
    return false;
  }

  const QString datPath = findConverseDat(gameDir);
  if (datPath.isEmpty())
  {
    errorMsg = "Could not find CONVERSE.DAT in the game directory.";
    return false;
  }

  const QString backupPath = datPath + ".orig";
  if (QFile::exists(backupPath))
  {
    errorMsg = QString("Backup file already exists; refusing to overwrite it:\n%1").arg(backupPath);
    return false;
  }
  if (!QFile::copy(datPath, backupPath))
  {
    errorMsg = QString("Could not create backup file:\n%1").arg(backupPath);
    return false;
  }

  QFile datFile(datPath);
  if (!datFile.open(QIODevice::ReadOnly))
  {
    errorMsg = "Could not open CONVERSE.DAT for reading.";
    return false;
  }
  QByteArray container = datFile.readAll();
  datFile.close();

  // find the OBJECT.TAB index entry (16-bit file count, then packed
  // DatFileIndex records starting at offset 2)
  const uint16_t fileCount = qFromLittleEndian<quint16>(container.constData());
  long entryOffset = -1;
  for (unsigned int i = 0; i < fileCount; i++)
  {
    const long offset = 2 + (i * sizeof(DatFileIndex));
    if ((offset + static_cast<long>(sizeof(DatFileIndex))) > container.size())
    {
      break;
    }
    const DatFileIndex* index = reinterpret_cast<const DatFileIndex*>(container.constData() + offset);
    if (strncmp(OBJECT_TAB_FILENAME, index->filename, INDEX_FILENAME_LEN) == 0)
    {
      entryOffset = offset;
      break;
    }
  }

  if (entryOffset < 0)
  {
    errorMsg = "Could not find the OBJECT.TAB index entry in CONVERSE.DAT.";
    return false;
  }

  // append the fixed table uncompressed at the end of the container and
  // repoint the index entry at it. Flag values 0x05/0x00 are the uncompressed
  // storage mode also used by standalone_utils/dat_builder.c (flags 0x0005).
  const QByteArray fixed = fixedTable(pristineTab);

  DatFileIndex newEntry;
  memcpy(&newEntry, container.constData() + entryOffset, sizeof(DatFileIndex));
  newEntry.flags_a = 0x05;
  newEntry.flags_b = 0x00;
  newEntry.uncompressed_size = qToLittleEndian<qint32>(fixed.size());
  newEntry.compressed_size = qToLittleEndian<qint32>(fixed.size());
  newEntry.offset = qToLittleEndian<quint32>(static_cast<uint32_t>(container.size()));

  memcpy(container.data() + entryOffset, &newEntry, sizeof(DatFileIndex));
  container.append(fixed);

  QSaveFile outFile(datPath);
  if (!outFile.open(QIODevice::WriteOnly))
  {
    errorMsg = "Could not open CONVERSE.DAT for writing.";
    return false;
  }
  if ((outFile.write(container) != container.size()) || !outFile.commit())
  {
    errorMsg = "Failed to write the modified CONVERSE.DAT.";
    return false;
  }

  return true;
}
