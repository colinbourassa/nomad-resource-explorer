#include "datlibrary.h"
#include <QFile>
#include <QIODevice>
#include <QtEndian>
#include <QImage>
#include <QRgb>

const QMap<DatFileType,QString> DatLibrary::s_datFileNames
{
  {DatFileType_ANIM,     "ANIM.DAT"},
  {DatFileType_CONVERSE, "CONVERSE.DAT"},
  {DatFileType_INVENT,   "INVENT.DAT"},
  {DatFileType_SAMPLES,  "SAMPLES.DAT"},
  {DatFileType_TEST,     "TEST.DAT"},
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

  m_gamePalette.clear();
  m_gameText.clear();
}

bool DatLibrary::getFileAtIndex(DatFileType dat, unsigned int index, QByteArray& decompressedFile)
{
  bool status = false;
  const unsigned long indexEntryOffset = 2 + (index * sizeof(DatFileIndex));

  if (m_datContents[dat].size() >= static_cast<int>((indexEntryOffset + sizeof(DatFileIndex))))
  {
    const char* rawDat = m_datContents[dat].constData();

    DatFileIndex indexEntry;
    memcpy(&indexEntry, rawDat + indexEntryOffset, sizeof(DatFileIndex));

    QByteArray storedFile;
    storedFile.append(rawDat + indexEntry.offset, indexEntry.compressed_size);

    // if the file is stored with some form of compression
    if (indexEntry.flags_b & 0x1)
    {
      if (indexEntry.flags_a & 0x4)
      {
        status = lzDecompress(storedFile, decompressedFile);
      }
      else
      {
        // the file is stored with compression, but it's a format not yet supported here
        status = false;
      }
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

bool DatLibrary::getFileByName(DatFileType dat, QString filename, QByteArray &filedata)
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

    if (strncmp(filename.toStdString().c_str(), index->filename, 14) == 0)
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

QVector<QRgb> DatLibrary::getGamePalette()
{
  loadGamePalette();
  return m_gamePalette;
}

bool DatLibrary::loadGamePalette()
{
  // TODO: some palettes (such as GAME.PAL) have fewer than 256 colors,
  // but we'll want to fill in the remaining colors because some images
  // will index beyond this limited palette

  bool status = true;

  if (m_gamePalette.count() == 0)
  {
    QByteArray paldata;

    if (getFileByName(DatFileType_TEST, "GAME.PAL", paldata))
    {
      for (int palByteIdx = 3; palByteIdx <= paldata.size() - 3; palByteIdx += 3)
      {
        // upconvert the 6-bit palette data to 8-bit by leftshifting and
        // replicating the two high bits in the low positions
        int r = (paldata[palByteIdx] << 2)   | (paldata[palByteIdx] >> 4);
        int g = (paldata[palByteIdx+1] << 2) | (paldata[palByteIdx+1] >> 4);
        int b = (paldata[palByteIdx+2] << 2) | (paldata[palByteIdx+2] >> 4);
        m_gamePalette.append(qRgb(r, g, b));
      }
    }
    else
    {
      status = false;
    }
  }

  return status;
}

QPoint DatLibrary::getPixelLocation(int imgWidth, int pixelNum) const
{
  QPoint pt (pixelNum % imgWidth, pixelNum / imgWidth);
  return pt;
}

QPixmap DatLibrary::convertStpToPixmap(QByteArray &stpData, QVector<QRgb> palette, bool& status)
{
  status = true;
  uint16_t width = qFromLittleEndian<quint16>(stpData.data() + 0);
  uint16_t height = qFromLittleEndian<quint16>(stpData.data() + 2);

  QImage img(width, height, QImage::Format_Indexed8);
  img.setColorTable(palette);

  int pixelcount = width * height;
  int inputpos = 8; // STP image data begins at byte index 8
  int outputpos = 0;
  int endptr = 0;

  while ((inputpos < stpData.size()) && (outputpos < pixelcount))
  {
    uint8_t rlebyte = static_cast<uint8_t>(stpData.at(inputpos));
    inputpos++;

    if (rlebyte & 0x80)
    {
      // bit 7 is set, so this is moving the output pointer ahread,
      // leaving the default value in the skipped locations
      endptr = outputpos + (rlebyte & 0x7F);
      while ((outputpos < endptr) && (outputpos < pixelcount))
      {
        img.setPixel(getPixelLocation(width, outputpos), 0x00);
        outputpos++;
      }
    }
    else if (rlebyte & 0x40)
    {
      // Bit 7 is clear and bit 6 is set, so this is a repeating sequence of a single byte.
      // We only need to read one input byte for this RLE sequence, so verify that the input
      // pointer is still within the buffer range.
      if (inputpos < stpData.size())
      {
        endptr = outputpos + (rlebyte & 0x3F);

        unsigned int palindex = static_cast<unsigned int>(stpData.at(inputpos));
        while ((outputpos < endptr) && (outputpos < pixelcount))
        {
          img.setPixel(getPixelLocation(width, outputpos), palindex);
          outputpos++;
        }
      }

      // advance the input once more so that we read the next RLE byte at the top of the loop
      inputpos++;
    }
    else
    {
      // bits 6 and 7 are clear, so this is a byte sequence copy from the input
      endptr = outputpos + rlebyte;
      while ((outputpos < endptr) && (outputpos < pixelcount) && (inputpos < stpData.size()))
      {
        unsigned int palindex = static_cast<unsigned int>(stpData.at(inputpos));
        img.setPixel(getPixelLocation(width, outputpos), palindex);
        inputpos++;
        outputpos++;
      }
    }
  }

  // check if the input runs dry before all of the pixels are accounted for in the output
  if ((inputpos >= stpData.size()) && (outputpos < pixelcount))
  {
    status = false;
  }

  return QPixmap::fromImage(img);
}

bool DatLibrary::lzDecompress(QByteArray compressedfile, QByteArray &decompressedFile)
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

  while (inputPos < inputBufLen)
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
