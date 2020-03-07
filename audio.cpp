#include <QtEndian>
#include <QString>
#include <stdint.h>
#include <string.h>
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

int Audio::getStartLocation(QByteArray& nnvData, int soundId)
{
  int loc = -1;

  // if the NNV file is at least big enough to contain the header fields for the requested sound ID
  if (nnvData.size() >= 1 + (NNV_INDEX_SIZE * (soundId + 1)))
  {
    int32_t locationField;
    memcpy (&locationField, nnvData.data() + 1 + (NNV_INDEX_SIZE * soundId), sizeof(int32_t));
    locationField = qFromLittleEndian<qint32>(locationField);
    loc = locationField;
  }

  return loc;
}

int Audio::getSoundDataLength(QByteArray& nnvData, int soundId)
{
  int len = -1;

  // if the NNV file is at least big enough to contain the header fields for the requested sound ID
  if (nnvData.size() >= 1 + (NNV_INDEX_SIZE * (soundId + 1)))
  {
    int32_t lengthField;
    memcpy (&lengthField, nnvData.data() + 1 + (NNV_INDEX_SIZE * soundId) + 4, sizeof(int32_t));
    lengthField = qFromLittleEndian<qint32>(lengthField);
    len = lengthField;
  }

  return len;
}

bool Audio::readSound(DatFileType dat, QString nnvContainer, int soundId, QByteArray& pcmData)
{
  bool status = false;
  QByteArray nnvData;
  if (m_lib->getFileByName(dat, nnvContainer, nnvData) && !nnvData.isEmpty())
  {
    uint8_t* nnvptr = reinterpret_cast<uint8_t*>(nnvData.data());
    if (soundId < nnvptr[0])
    {
      const int32_t startOffset = getStartLocation(nnvData, soundId);
      const int32_t compressedSize = getSoundDataLength(nnvData, soundId);

      if ((compressedSize > 0) && (nnvData.size() >= (startOffset + compressedSize)))
      {
        decode(nnvptr + startOffset, compressedSize, pcmData);
        status = true;
      }
    }
  }

  return status;
}

void Audio::decode(uint8_t* encoded, int length, QByteArray& decoded)
{
  bool upper_nibble = true;
  bool cmd_nibble = false;
  bool repeat_cmd = false;
  int repeat_idx = 0;
  int repeat_count = 0;
  bool repeat_count_first_nibble = false;
  int inpos = 0;
  unsigned int delta_table_offset = 0;
  uint8_t nibble;
  uint8_t last_value = 0x80;

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

    // if we're in the middle of a repeat command
    if (repeat_cmd)
    {
      // if we haven't yet read the first of two nibbles of the repeat count
      if (!repeat_count_first_nibble)
      {
        repeat_count = (nibble << 4);
        repeat_count_first_nibble = true;
      }
      else
      {
        // we already got the first nibble of the repeat count
        // on a previous iteration, so get the second nibble and
        // write the output now
        repeat_count |= nibble;

        // the repeat count value of 0 is reserved to indicate 0x100
        if (repeat_count == 0)
        {
          repeat_count = 0x100;
        }

        for (repeat_idx = 0; repeat_idx < repeat_count; repeat_idx++)
        {
          decoded.append(static_cast<int8_t>(last_value));
        }
        repeat_cmd = false;
        repeat_count_first_nibble = false;
      }
    }
    // otherwise, if we saw a nibble of 0, this indicates the start of a command sequence
    else if (cmd_nibble)
    {
      cmd_nibble = false;

      // if this nibble is less than 0xF, it is used as an index to the appropriate PCM delta subtable
      if (nibble < 0xF)
      {
        delta_table_offset = 0x10 * nibble;
      }
      else
      {
        // this is the start of a byte-repeat command, so we need to collect
        // the following two nibbles and combine them into the repeat count
        repeat_cmd = true;
        repeat_count_first_nibble = false;
      }
    }
    else if (nibble == 0)
    {
      cmd_nibble = true;
    }
    else
    {
      last_value += s_deltaTable[delta_table_offset + nibble];
      decoded.append(static_cast<int8_t>(last_value));
    }
  }
}

int Audio::getNumberOfSoundsInNNV(DatFileType dat, QString nnvContainer)
{
  QByteArray nnvData;
  int soundCount = 0;

  if (m_lib->getFileByName(dat, nnvContainer, nnvData) && !nnvData.isEmpty())
  {
    uint8_t* nnvptr = reinterpret_cast<uint8_t*>(nnvData.data());
    soundCount = nnvptr[0];
  }

  return soundCount;
}

QMap<DatFileType,QStringList> Audio::getAllSoundList()
{
  QMap<DatFileType,QStringList> nnvContainerList;

  for (int datIdx = 0; datIdx < DatFileType_NUM_DAT_FILES; datIdx++)
  {
    const DatFileType datType = static_cast<DatFileType>(datIdx);
    QStringList nnvsInDat = m_lib->getFilenamesByExtension(datType, ".NNV");
    nnvContainerList[datType] = nnvsInDat;
  }

  return nnvContainerList;
}
