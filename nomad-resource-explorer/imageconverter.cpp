#include "imageconverter.h"
#include <stdint.h>
#include <QtEndian>
#include <QImage>

const int8_t ImageConverter::s_deltas[] = {0,1,2,3,4,5,6,7,-8,-7,-6,-5,-4,-3,-2,-1};

ImageConverter::ImageConverter()
{

}

QPoint ImageConverter::getPixelLocation(int imgWidth, int pixelNum)
{
  QPoint pt (pixelNum % imgWidth, pixelNum / imgWidth);
  return pt;
}

/**
 * Converts 8-bit raw image data (with a 4-byte header containing the image
 * width and height, respectively, in two 16-bit little-endian words) to
 * a QPixmap, using the provided palette.
 */
QPixmap ImageConverter::lbmToPixmap(QByteArray &lbmData, QVector<QRgb> palette, bool &status)
{
  status = true;
  QPixmap pmap;

  if (lbmData.size() >= 5) // 5 bytes is the minimum theoretical size of a raw image of this format
  {
    const uint16_t width = qFromLittleEndian<quint16>(lbmData.data() + 0);
    const uint16_t height = qFromLittleEndian<quint16>(lbmData.data() + 2);

    QImage img(width, height, QImage::Format_Indexed8);
    img.setColorTable(palette);

    int inputpos = 4; // pixel data starts at byte offset 4
    int outputpos = 0;

    while (inputpos < lbmData.size())
    {
      const unsigned int palIndex = static_cast<unsigned int>(lbmData.at(inputpos));
      inputpos++;
      img.setPixel(getPixelLocation(width, outputpos), palIndex);
      outputpos++;
    }

    pmap = QPixmap::fromImage(img);
  }

  return pmap;
}

QPixmap ImageConverter::plnToPixmap(QByteArray &plnData, QVector<QRgb> palette, bool &status)
{
  status = true;
  QPixmap pmap;

  if (plnData.size() >= 3) // 3 bytes is the minimum theoretical size of a .pln image
  {
    const uint16_t width = qFromLittleEndian<quint16>(plnData.data() + 0);
    const uint16_t height = static_cast<uint16_t>((plnData.size() - 2) / width);

    QImage img(width, height, QImage::Format_Indexed8);
    img.setColorTable(palette);

    int inputpos = 2;
    int outputpos = 0;

    while (inputpos < plnData.size())
    {
      const uint8_t palIndex = static_cast<uint8_t>(plnData.at(inputpos));
      inputpos++;
      img.setPixel(getPixelLocation(width, outputpos), palIndex);
      outputpos++;
    }

    pmap = QPixmap::fromImage(img);
  }

  return pmap;
}

QPixmap ImageConverter::stpToPixmap(QByteArray &stpData, QVector<QRgb> palette, bool &status)
{
  status = true;
  const uint16_t width = qFromLittleEndian<quint16>(stpData.data() + 0);
  const uint16_t height = qFromLittleEndian<quint16>(stpData.data() + 2);

  QImage img(width, height, QImage::Format_Indexed8);
  img.setColorTable(palette);

  const int pixelcount = width * height;
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

QPixmap ImageConverter::delToPixmap(QByteArray& delData, QVector<QRgb> palette, bool& status)
{
  status = true;
  const uint16_t width = qFromLittleEndian<quint16>(delData.data() + 0);
  const uint16_t height = qFromLittleEndian<quint16>(delData.data() + 2);
  int inputpos = 4;
  int outputpos = 0;

  QImage img(width, height, QImage::Format_Indexed8);
  img.setColorTable(palette);

  while (inputpos < delData.size())
  {
    uint8_t cmdbyte = static_cast<uint8_t>(delData.at(inputpos));
    uint8_t databyte = 0;
    inputpos++;

    if (cmdbyte & 0x01)
    {
      // we'll be writing the byte from the input stream to the output at least once,
      // so grab the input byte now
      databyte = static_cast<uint8_t>(delData.at(inputpos));
      inputpos++;

      if (cmdbyte & 0x02)
      {
        // single byte copy from input (low two bits of command are 11)
        img.setPixel(getPixelLocation(width, outputpos), databyte);
        outputpos++;
      }
      else
      {
        // repeat byte from input (low two bits of command are 01)
        for (int repeatidx = 0; repeatidx < (cmdbyte >> 2); repeatidx++)
        {
          img.setPixel(getPixelLocation(width, outputpos), databyte);
          outputpos++;
        }
      }
    }
    else
    {
      if (cmdbyte & 0x02)
      {
        // advance output ptr (low two bits are 10)

        int repeatcount = (cmdbyte >> 2);

        // the next byte from the input is used as the repeat count if and only if
        // the top six bits of the command byte are zeroed
        if (repeatcount == 0)
        {
          repeatcount = static_cast<uint8_t>(delData.at(inputpos));
          inputpos++;
        }

        for (int repeatidx = 0; repeatidx < repeatcount; repeatidx++)
        {
          img.setPixel(getPixelLocation(width, outputpos), 0);
          outputpos++;
        }
      }
      else
      {
        // delta encoding sequence (low two bits are 00)

        // length of the sequence, including the first literal byte
        const int length = (cmdbyte >> 2);

        if (length > 0)
        {
          // position in the input stream of the byte containing the last nibble for this sequence
          //const int sequenceEnd = inputpos + (length / 2);

          // first byte written to the output will be the next byte in the input stream
          databyte = static_cast<uint8_t>(delData.at(inputpos));
          inputpos++;

          img.setPixel(getPixelLocation(width, outputpos), databyte);
          outputpos++;

          int sequenceCount = 1;

          while (sequenceCount < length)
          {
            uint8_t nibble = static_cast<uint8_t>(delData.at(inputpos)) >> 4;

            databyte += s_deltas[nibble];
            img.setPixel(getPixelLocation(width, outputpos), databyte);
            outputpos++;
            sequenceCount++;

            // only process the second nibble if we haven't yet completed the sequence;
            // if the sequence does not require this nibble then it's simply wasted
            if (sequenceCount < length)
            {
              nibble = static_cast<uint8_t>(delData.at(inputpos)) & 0x0F;

              databyte += s_deltas[nibble];
              img.setPixel(getPixelLocation(width, outputpos), databyte);
              outputpos++;

              sequenceCount++;
            }

            inputpos++;
          }
        }
      }
    }
  }

  return QPixmap::fromImage(img);
}
