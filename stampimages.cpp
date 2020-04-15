#include <stdint.h>
#include <string.h>
#include <QByteArray>
#include <QList>
#include <QImage>
#include <QtEndian>
#include <QVector>
#include <QRgb>
#include "stampimages.h"
#include "imageconverter.h"

/**
 * Dictionary of .stp/.rol images and their associated palette files.
 * If a file does not appear in this dictionary, it is assumed to use
 * the standard GAME.PAL.
 */
const QMap<QString,QString> StampImages::s_stpToPal =
{
  { "nomad.stp",    "backg.pal" },
  { "gtek.stp",     "backg.pal" },
  { "design.stp",   "backg.pal" },
  { "papyrus.stp",  "backg.pal" },
  { "border.stp",   "backg.pal" },
  { "guybody.stp",  "backg.pal" },
  { "smk.rol",      "backg.pal" },
  { "guyhead.rol",  "backg.pal" },
  { "guyturn.rol",  "backg.pal" },
  { "guyhead2.rol", "backg.pal" }
};

const QList<DatFileType> StampImages::s_datsToSearch =
{
  // in the standard game, the only STP/ROL images that are
  // not already handled by the inventory object code are
  // those in TEST.DAT
  DatFileType_TEST
};

StampImages::StampImages(DatLibrary& lib, Palette& pal) :
  m_lib(&lib),
  m_pal(&pal)
{
}

/**
 * Returns a map containing lists of all .stp and .rol files within the DAT archives
 * that the static dictionary used by this class is set up to search. This function
 * does not indiscriminately search all of the game's DAT archives because the .stp
 * image format is also used for all of the game's inventory items, and those are handled
 * by a separate class.
 */
QMap<DatFileType,QStringList> StampImages::getAllStampsList()
{
  QMap<DatFileType,QStringList> stampList;

  foreach (DatFileType dat, s_datsToSearch)
  {
    QStringList stampsInDat = m_lib->getFilenamesByExtension(dat, STAMP_EXTENSION);
    stampsInDat.append(m_lib->getFilenamesByExtension(dat, ROLL_EXTENSION));
    stampList[dat] = stampsInDat;
  }

  return stampList;
}

/**
 * Gets a list of QImages, each one representing an .stp image from the named file.
 * This function can process both .stp and .rol files, with the latter containing
 * multiple images.
 */
bool StampImages::getStamp(DatFileType dat, QString filename, QList<QImage>& images)
{
  bool status = false;
  QList<QImage> imagesLocal;

  if (filename.length() > 4)
  {
    const bool isRoll = (filename.right(4).toLower() == QString(ROLL_EXTENSION));
    QByteArray data;

    if (m_lib->getFileByName(dat, filename, data))
    {
      QVector<QRgb> palData;
      if (s_stpToPal.contains(filename))
      {
        status = m_pal->paletteByName(dat, s_stpToPal[filename], palData);
      }
      else
      {
        status = m_pal->gamePalette(palData);
      }

      if (status)
      {
        if (isRoll)
        {
          QList<QByteArray> stpDataFromRoll = getStpDataFromRoll(data);
          foreach (QByteArray singleStp, stpDataFromRoll)
          {
            QImage stpImage;
            if (ImageConverter::stpToImage(singleStp, palData, stpImage))
            {
              imagesLocal.append(stpImage);
              status = true;
            }
          }
        }
        else
        {
          QImage stpImage;
          if (ImageConverter::stpToImage(data, palData, stpImage))
          {
            imagesLocal.append(stpImage);
            status = true;
          }
        }
      }
    }
  }

  if (status)
  {
    images = imagesLocal;
  }

  return status;
}

/**
 * Returns a list of byte arrays, each one containing the complete data for one of the
 * .stp images contained within the provided .rol file data.
 */
QList<QByteArray> StampImages::getStpDataFromRoll(const QByteArray& roll)
{
  QList<QByteArray> stpData;
  const int rolSize = roll.size();

  const QList<int> stpStartOffsets = getStpStartOffsetListFromRoll(roll);

  for (int stpIndex = 0; stpIndex < stpStartOffsets.count(); stpIndex++)
  {
    const bool isLast = (stpIndex == (stpStartOffsets.count() - 1));
    const int startOffset = stpStartOffsets[stpIndex];
    const int byteCount = isLast ? (rolSize - startOffset) : (stpStartOffsets[stpIndex + 1] - startOffset);

    // only continue if the roll is at least big enough to contain data for this STP
    if (rolSize >= (startOffset + byteCount))
    {
      QByteArray singleStp(roll.data() + startOffset, byteCount);
      stpData.append(singleStp);
    }
  }

  return stpData;
}

/**
 * Returns a list of starting offsets of .stp data within the provided .rol
 * file data.
 */
QList<int> StampImages::getStpStartOffsetListFromRoll(const QByteArray& roll)
{
  const int count = getNumOfStampsInRoll(roll);
  const int rolSize = roll.size();
  const uint8_t* rawdata = reinterpret_cast<const uint8_t*>(roll.data());
  QList<int> startOffsets;

  for (int index = 0; index < count; index++)
  {
    const int startPosOffset = (index * 4);
    if (rolSize >= (startPosOffset + 4))
    {
      int32_t stpStartOffset = 0;
      memcpy(&stpStartOffset, rawdata + startPosOffset, sizeof(int32_t));
      stpStartOffset = qFromLittleEndian<qint32>(stpStartOffset);
      startOffsets.append(stpStartOffset);
    }
  }

  return startOffsets;
}

/**
 * Returns the number of images in the provided stamp roll (.rol) data.
 * Returns -1 if the data is not in a valid format.
 */
int StampImages::getNumOfStampsInRoll(const QByteArray& roll)
{
  int count = -1;

  if (roll.size() > 4)
  {
    const uint8_t* rawdata = reinterpret_cast<const uint8_t*>(roll.data());
    uint32_t firstStpStartOffset;
    memcpy(&firstStpStartOffset, rawdata, sizeof(uint32_t));
    firstStpStartOffset = qFromLittleEndian<quint32>(firstStpStartOffset);

    if (firstStpStartOffset % 4 == 0)
    {
      count = firstStpStartOffset / 4;
    }
  }

  return count;
}
