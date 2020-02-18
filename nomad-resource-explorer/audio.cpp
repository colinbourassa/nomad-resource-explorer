#include <QtEndian>
#include <stdint.h>
#include "audio.h"

const int8_t Audio::s_deltaTable[240] =
{
  0, -7,    -6,    -5,    -4,    -3,    -2,    -1,    0, 1,    2,    3,    4,    5,    6,    7,
  0, -0x0A, -8,    -6,    -5,    -3,    -2,    -1,    0, 1,    2,    3,    5,    6,    8,    0x0A,
  0, -0x10, -0x0D, -0x0A, -8,    -6,    -4,    -2,    0, 2,    4,    6,    8,    0x0A, 0x0D, 0x10,
  0, -0x17, -0x13, -0x0F, -0x0B, -8,    -5,    -2,    0, 2,    5,    8,    0x0B, 0x0F, 0x13, 0x17,
  0, -0x1E, -0x18, -0x13, -0x0F, -0x0A, -7,    -3,    0, 3,    7,    0x0A, 0x0F, 0x13, 0x18, 0x1E,
  0, -0x26, -0x1F, -0x18, -0x12, -0x0D, -8,    -4,    0, 4,    8,    0x0D, 0x12, 0x18, 0x1F, 0x26,
  0, -0x2E, -0x25, -0x1D, -0x16, -0x10, -0x0A, -5,    0, 5,    0x0A, 0x10, 0x16, 0x1D, 0x25, 0x2E,
  0, -0x36, -0x2C, -0x23, -0x1A, -0x13, -0x0C, -6,    0, 6,    0x0C, 0x13, 0x1A, 0x23, 0x2C, 0x36,
  0, -0x3F, -0x33, -0x28, -0x1F, -0x16, -0x0E, -7,    0, 7,    0x0E, 0x16, 0x1F, 0x28, 0x33, 0x3F,
  0, -0x49, -0x3B, -0x2F, -0x23, -0x19, -0x10, -8,    0, 7,    0x10, 0x19, 0x23, 0x2E, 0x3A, 0x48,
  0, -0x53, -0x43, -0x35, -0x28, -0x1D, -0x12, -9,    0, 9,    0x12, 0x1C, 0x28, 0x35, 0x43, 0x52,
  0, -0x5D, -0x4B, -0x3C, -0x2D, -0x20, -0x14, -0x0A, 0, 0x0A, 0x14, 0x20, 0x2D, 0x3B, 0x4B, 0x5C,
  0, -0x68, -0x54, -0x43, -0x33, -0x24, -0x17, -0x0B, 0, 0x0B, 0x17, 0x24, 0x32, 0x42, 0x54, 0x67,
  0, -0x74, -0x5E, -0x4A, -0x38, -0x28, -0x19, -0x0C, 0, 0x0C, 0x19, 0x28, 0x38, 0x4A, 0x5D, 0x73,
  0, -0x80, -0x68, -0x52, -0x3E, -0x2C, -0x1C, -0x0D, 0, 0x0D, 0x1C, 0x2C, 0x3E, 0x51, 0x67, 0x7F
};

Audio::Audio(DatLibrary& lib) :
  m_lib(&lib)
{

}

bool Audio::readSound(DatFileType dat, QString nnvContainer, int soundId, QByteArray& pcmData)
{
  bool status = false;
  QByteArray nnvData;
  if (m_lib->getFileByName(dat, nnvContainer, nnvData) && !nnvData.isEmpty())
  {
    uint8_t* nnvptr = reinterpret_cast<uint8_t*>(nnvData.data());
    if ((soundId < nnvptr[0]) &&
        (nnvData.size() > (1 + (8 * (nnvptr[0])))))
    {
      const uint32_t startOffset =    qFromLittleEndian<quint32>(nnvptr[1 + (NNV_INDEX_SIZE * soundId)]);
      const uint32_t compressedSize = qFromLittleEndian<quint32>(nnvptr[5 + (NNV_INDEX_SIZE * soundId)]);

      if (nnvData.size() >= (startOffset + compressedSize))
      {
        decode(nnvptr + startOffset, compressedSize, pcmData);
        status = true;
      }
    }
  }

  return status;
}

void Audio::decode(uint8_t* encoded, unsigned int length, QByteArray& decoded)
{
  bool upper_nibble = true;
  bool cmdnibble = false;
  bool repeatcmd = false;
  unsigned int inpos = 0;
  unsigned int delta_table_offset = 0;
  uint8_t nibble;
  int8_t lastValue = -128;

  while (inpos < length)
  {
    if (upper_nibble)
    {
      nibble = encoded[inpos] >> 4;
      upper_nibble = false;
    }
    else
    {
      nibble = encoded[inpos] & 0x0F;
      upper_nibble = true;
      inpos++;
    }

    if (repeatcmd)
    {
      repeatcmd = false;
      for (int repeatidx = 0; repeatidx < nibble; repeatidx++)
      {
        decoded.append(lastValue);
      }
    }
    else if (cmdnibble)
    {
      cmdnibble = false;

      // if this nibble is less than 0xF, it is used as an index to the appropriate PCM delta subtable
      if (nibble < 0xF)
      {
        delta_table_offset = 0x10 * nibble;
      }
      else
      {
        // Otherwise, this is a command to repeat the last byte that was written to the output.
        // Number of times to repeat is the next byte? or nibble?
        // Not exactly sure of the byte-handling procedure here.
        repeatcmd = true;
      }
    }
    else
    {
      lastValue += s_deltaTable[delta_table_offset + nibble];
    }
  }
}

bool Audio::getSoundList(DatFileType dat, QString nnvContainer, QVector<unsigned int>& sounds)
{
  bool status = false;
  QByteArray nnvData;
  if (m_lib->getFileByName(dat, nnvContainer, nnvData) && !nnvData.isEmpty())
  {
    uint8_t* nnvptr = reinterpret_cast<uint8_t*>(nnvData.data());

    // ensure that the NNV is at least large enough to contain all of the index entries for the number
    // of sounds claimed to be contained in this NNV
    if (nnvData.size() > (1 + (nnvptr[0] * NNV_INDEX_SIZE)))
    {
      status = true;
      sounds.clear();
      int soundId = 0;

      while (status && soundId < nnvptr[0])
      {
        const uint32_t startOffset =    qFromLittleEndian<quint32>(nnvptr[1 + (NNV_INDEX_SIZE * soundId)]);
        const uint32_t compressedSize = qFromLittleEndian<quint32>(nnvptr[5 + (NNV_INDEX_SIZE * soundId)]);

        // if the NNV is large enough to contain this file
        if (nnvData.size() >= (startOffset + compressedSize))
        {
          sounds.append(compressedSize);
          soundId++;
        }
        else
        {
          status = false;
        }
      }
    }
  }

  return status;
}
