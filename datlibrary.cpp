#include "datlibrary.h"
#include <QFile>
#include <QIODevice>
#include <QtEndian>
#include <QImage>
#include <QRgb>

const QMap<DatFileType,QString> DatLibrary::s_datFileNames
{
  {DatFileType_ANIM,     DAT_FILENAME_ANIM},
  {DatFileType_CONVERSE, DAT_FILENAME_CONVERSE},
  {DatFileType_INVENT,   DAT_FILENAME_INVENT},
  {DatFileType_SAMPLES,  DAT_FILENAME_SAMPLES},
  {DatFileType_TEST,     DAT_FILENAME_TEST}
};

DatLibrary::DatLibrary()
{

}

DatLibrary::~DatLibrary()
{

}

bool DatLibrary::openData(QString pathToGameDir)
{
  bool status = true;
  foreach (DatFileType dat, s_datFileNames.keys())
  {
    const QString fullpath = pathToGameDir + "/" + s_datFileNames[dat];
    QFile datFile(fullpath);

    if (datFile.open(QIODevice::ReadOnly))
    {
      m_datContents[dat] = datFile.readAll();
    }
    else
    {
      status = false;
    }
  }

  return status;
}

void DatLibrary::closeData()
{
  foreach (DatFileType datType, s_datFileNames.keys())
  {
    m_datContents[datType].clear();
  }

  m_gameText.clear();
}

bool DatLibrary::getFileAtIndex(DatFileType dat, unsigned int index, QByteArray& decompressedFile)
{
  bool status = false;
  const unsigned long indexEntryOffset = 2 + (index * sizeof(DatFileIndex));

  if (m_datContents[dat].size() >= static_cast<int>((indexEntryOffset + sizeof(DatFileIndex))))
  {
    const char* rawDat = m_datContents[dat].constData();
    int skipUncompressedBytes = 0;

    DatFileIndex indexEntry;
    memcpy(&indexEntry, rawDat + indexEntryOffset, sizeof(DatFileIndex));

    if ((indexEntry.flags_b & 0x01) && (~indexEntry.flags_a & 0x04))
    {
      skipUncompressedBytes = 4;
    }

    // note that files with the uncompressed 4-byte header must have those
    // four bytes added to the listed compressed size when copying
    QByteArray storedFile;
    storedFile.append(rawDat + indexEntry.offset, indexEntry.compressed_size + skipUncompressedBytes);

    // if the file is stored with some form of compression
    if (indexEntry.flags_b & 0x1)
    {
      status = lzDecompress(storedFile, decompressedFile, skipUncompressedBytes);
    }
    else
    {
      // the file is not compressed, and may be copied byte-for-byte from the .DAT
      decompressedFile = storedFile;
      status = true;
    }
  }

  return status;
}

bool DatLibrary::getFileByName(DatFileType dat, QString filename, QByteArray& filedata)
{
  const char* rawdat = m_datContents[dat].constData();
  const long datsize = m_datContents[dat].size();
  bool found = false;
  bool status = false;
  long currentIndexOffset = 2;
  const DatFileIndex* index = nullptr;
  unsigned int indexNum = 0;

  while (!found && ((currentIndexOffset + sizeof(DatFileIndex)) < datsize))
  {
    // point to the index struct at the current location
    index = reinterpret_cast<const DatFileIndex*>(rawdat + currentIndexOffset);

    if (strncmp(filename.toStdString().c_str(), index->filename, INDEX_FILENAME_LEN) == 0)
    {
      found = true;
    }
    else
    {
      currentIndexOffset += sizeof(DatFileIndex);
      indexNum++;
    }
  };

  if (found)
  {
    status = getFileAtIndex(dat, indexNum, filedata);
  }

  return status;
}

QStringList DatLibrary::getFilenamesByExtension(DatFileType dat, QString extension)
{
  const char* rawdat = m_datContents[dat].constData();
  const long datsize = m_datContents[dat].size();
  long currentIndexOffset = 2;
  const DatFileIndex* index = nullptr;
  const uint16_t totalFileCount = qFromLittleEndian<quint16>(rawdat);
  int indexnum = 0;

  QStringList filenames;
  QString currentFilename;
  const QString extensionUcase = extension.toUpper();

  while ((indexnum < totalFileCount) && ((currentIndexOffset + sizeof(DatFileIndex)) < datsize))
  {
    // point to the index struct at the current location
    index = reinterpret_cast<const DatFileIndex*>(rawdat + currentIndexOffset);

    const QString currentFilename = index->filename;
    if (currentFilename.toUpper().endsWith(extensionUcase))
    {
      filenames.append(currentFilename);
    }
    currentIndexOffset += sizeof(DatFileIndex);
    indexnum++;
  };

  return filenames;
}

bool DatLibrary::lzDecompress(QByteArray compressedfile, QByteArray& decompressedFile, int skipUncompressedBytes)
{
  bool status = true;

  memset (m_lzRingBuffer, 0x20, LZ_RINGBUF_SIZE);

  uint16_t bufPos = 0xFEE;
  uint16_t inputPos = 0;
  uint8_t codeword[2];
  uint8_t flagByte = 0;
  uint8_t decodeByte = 0;
  uint8_t chunkIndex = 0;
  uint8_t byteIndexInChunk = 0;
  uint8_t chunkSize = 0;
  uint16_t chunkSource = 0;
  const int inputBufLen = compressedfile.size();

  decompressedFile.clear();

  if (skipUncompressedBytes > 0)
  {
    if (inputBufLen >= skipUncompressedBytes)
    {
      decompressedFile.append(compressedfile.data(), skipUncompressedBytes);
      inputPos += skipUncompressedBytes;
    }
    else
    {
      status = false;
    }
  }

  while (status && (inputPos < inputBufLen))
  {
    flagByte = static_cast<uint8_t>(compressedfile[inputPos++]);

    chunkIndex = 0;
    while ((chunkIndex < 8) && (inputPos < inputBufLen))
    {
      if ((flagByte & (1 << chunkIndex)) != 0)
      {
        // single byte literal
        decodeByte = static_cast<uint8_t>(compressedfile[inputPos++]);
        decompressedFile.append(static_cast<char>(decodeByte));

        m_lzRingBuffer[bufPos++] = decodeByte;
        if (bufPos >= LZ_RINGBUF_SIZE)
        {
          bufPos = 0;
        }
      }
      else
      {
        // two-byte reference to a sequence in the circular buffer
        codeword[0] = static_cast<uint8_t>(compressedfile[inputPos++]);
        codeword[1] = static_cast<uint8_t>(compressedfile[inputPos++]);

        chunkSize =   ((codeword[1] & 0xF0) >> 4) + 3;
        chunkSource = static_cast<uint16_t>(((codeword[1] & 0x0F) << 8) | codeword[0]);

        byteIndexInChunk = 0;
        while (byteIndexInChunk < chunkSize)
        {
          decodeByte = static_cast<uint8_t>(m_lzRingBuffer[chunkSource]);
          decompressedFile.append(static_cast<char>(decodeByte));

          if (++chunkSource >= LZ_RINGBUF_SIZE)
          {
            chunkSource = 0;
          }

          m_lzRingBuffer[bufPos] = decodeByte;
          if (++bufPos >= LZ_RINGBUF_SIZE)
          {
            bufPos = 0;
          }

          byteIndexInChunk += 1;
        }

        if (byteIndexInChunk < chunkSize)
        {
          status = false;
        }
      }

      chunkIndex += 1;
    } // end for chunkIndex 0 to 7
  }

  return status;
}

QString DatLibrary::getGameText(int offset)
{
  QString txt("");

  if (m_gameText.isEmpty())
  {
    QString filename("GAMETEXT.TXT");
    getFileByName(DatFileType_CONVERSE, filename, m_gameText);
  }

  if (offset < m_gameText.size())
  {
    const char* rawdata = m_gameText.data();
    txt = QString::fromUtf8(rawdata + offset);
  }

  return txt;
}
