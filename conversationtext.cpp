#include "conversationtext.h"
#include "gametext.h"
#include <stdint.h>
#include <string.h>
#include <QtEndian>

ConversationText::ConversationText(DatLibrary& lib, Aliens& aliens) :
  m_lib(&lib),
  m_aliens(&aliens)
{

}

QStringList ConversationText::getConversationText(int alienId, ConvTopic topic, int thingId)
{
  QStringList dialogLines;
  QByteArray tlktData;
  ConvTableType tableType = ConvTableType_Individual;
  int alienOrRaceId = alienId;

  getTLKTData(tableType, alienOrRaceId, tlktData);

  QList<int> tlknIndexes = getTLKNIndexes(topic, thingId, tlktData);

  // if the individual-specific tables didn't have any entries for this topic,
  // fall back to the generic tables used by all members of an alien race
  if (tlknIndexes.size() == 0)
  {
    tableType = ConvTableType_Race;
    alienOrRaceId = m_aliens->getRace(alienId);
    getTLKTData(tableType, alienOrRaceId, tlktData);
    tlknIndexes = getTLKNIndexes(topic, thingId, tlktData);
  }

  if (tlknIndexes.size() > 0)
  {
    QByteArray tlknData;
    getTLKNData(tableType, alienOrRaceId, tlknData);

    QList<int> tlkxIndexes = getTLKXIndexes(tlknIndexes, tlknData);

    QByteArray tlkxIndexData;
    QByteArray tlkxStrData;
    getTLKXData(tableType, alienOrRaceId, tlkxIndexData, tlkxStrData);
    dialogLines = getTLKXStrings(tlkxIndexes, tlkxIndexData, tlkxStrData);
  }

  return dialogLines;
}

bool ConversationText::getTLKXData(ConvTableType tableType, int id, QByteArray& indexData, QByteArray& strData)
{
  bool status = false;
  QString idxFilename;
  QString strFilename;

  if (tableType == ConvTableType_Individual)
  {
    idxFilename = getTLKXCIndexFilename(id);
    strFilename = getTLKXCStringsFilename(id);
  }
  else if (tableType == ConvTableType_Race)
  {
    idxFilename = getTLKXRIndexFilename(id);
    strFilename = getTLKXRStringsFilename(id);
  }

  if (!idxFilename.isEmpty() && !strFilename.isEmpty())
  {
    status = (m_lib->getFileByName(DatFileType_CONVERSE, idxFilename, indexData) &&
              m_lib->getFileByName(DatFileType_CONVERSE, strFilename, strData));
  }

  return status;
}

QStringList ConversationText::getTLKXStrings(const QList<int>& tlkxIndexes,
                                             const QByteArray& tlkxIndexData,
                                             const QByteArray& tlkxStrData)
{
  QStringList strings;
  const uint8_t* idxData = reinterpret_cast<const uint8_t*>(tlkxIndexData.data());

  foreach (int idx, tlkxIndexes)
  {
    const int tlkxIndexOffset = (idx * TLKX_RECORDSIZE) + TLKX_RECORDSIZE;
    uint32_t tlkxStrOffset = 0;
    memcpy(&tlkxStrOffset, &idxData[tlkxIndexOffset], TLKX_RECORDSIZE);
    tlkxStrOffset = qFromLittleEndian<quint32>(tlkxStrOffset);

    const QString line = GameText::readString(tlkxStrData.data() + tlkxStrOffset);
    strings.append(line);
  }

  return strings;
}

bool ConversationText::getTLKNData(ConvTableType tableType, int id, QByteArray& data)
{
  bool status = false;

  if (tableType == ConvTableType_Individual)
  {
    status = m_lib->getFileByName(DatFileType_CONVERSE, getTLKNCFilename(id), data);
  }
  else if (tableType == ConvTableType_Race)
  {
    status = m_lib->getFileByName(DatFileType_CONVERSE, getTLKNRFilename(id), data);
  }

  return status;
}

bool ConversationText::getTLKTData(ConvTableType tableType, int id, QByteArray& data)
{
  bool status = false;

  if (tableType == ConvTableType_Individual)
  {
    status = m_lib->getFileByName(DatFileType_CONVERSE, getTLKTCFilename(id), data);
  }
  else if (tableType == ConvTableType_Race)
  {
    status = m_lib->getFileByName(DatFileType_CONVERSE, getTLKTRFilename(id), data);
  }

  return status;
}

const QString ConversationText::getTLKNCFilename(int id)
{
  return QString("TLKNC%1.TAB").arg(id, 3, 10, QChar('0'));
}

const QString ConversationText::getTLKNRFilename(int id)
{
  return QString("TLKNR%1.TAB").arg(id, 3, 10, QChar('0'));
}

const QString ConversationText::getTLKTCFilename(int id)
{
  return QString("TLKTC%1.TAB").arg(id, 3, 10, QChar('0'));
}

const QString ConversationText::getTLKTRFilename(int id)
{
  return QString("TLKTR%1.TAB").arg(id, 3, 10, QChar('0'));
}

const QString ConversationText::getTLKXCIndexFilename(int id)
{
  return QString("TLKXC%1.IDX").arg(id, 3, 10, QChar('0'));
}

const QString ConversationText::getTLKXCStringsFilename(int id)
{
  return QString("TLKXC%1.TXT").arg(id, 3, 10, QChar('0'));
}

const QString ConversationText::getTLKXRIndexFilename(int id)
{
  return QString("TLKXR%1.IDX").arg(id, 3, 10, QChar('0'));
}

const QString ConversationText::getTLKXRStringsFilename(int id)
{
  return QString("TLKXR%1.TXT").arg(id, 3, 10, QChar('0'));
}

QList<int> ConversationText::getTLKXIndexes(const QList<int>& tlknIndexes, const QByteArray& tlknData)
{
  QList<int> tlkxIndexes;

  const uint8_t* data = reinterpret_cast<const uint8_t*>(tlknData.data());

  foreach (int tlknIndex, tlknIndexes)
  {
    const int offset = tlknIndex * TLKN_RECORDSIZE;
    const int tlkxIndex = data[offset] + (0x100 * data[offset + 1]);
    tlkxIndexes.append(tlkxIndex);
  }

  return tlkxIndexes;
}

QList<int> ConversationText::getTLKNIndexes(ConvTopic topic, int requestedId, const QByteArray& tlktData)
{
  QList<int> indexes;

  const uint8_t* data = reinterpret_cast<const uint8_t*>(tlktData.data());
  int offset = 0;

  while (offset < tlktData.size())
  {
    const int firstByte = data[offset];
    const int alienId   = data[offset + 3];
    const int placeId   = data[offset + 4] + (0x100 * data[offset + 5]);
    const int objectId  = data[offset + 6];
    const int raceId    = data[offset + 7];
    const int tlknIndex = data[offset + 8] + (0x100 * data[offset + 9]);

    if ((topic == ConvTopic_Race) && (firstByte == 7) && (raceId == requestedId))
    {
      indexes.append(tlknIndex);
    }
    else if ((topic == ConvTopic_Person) && (alienId == requestedId))
    {
      indexes.append(tlknIndex);
    }
    else if ((topic == ConvTopic_Object) && (objectId == requestedId))
    {
      indexes.append(tlknIndex);
    }
    else if ((topic == ConvTopic_Location) && (placeId == requestedId))
    {
      indexes.append(tlknIndex);
    }

    offset += TLKT_RECORDSIZE;
  }

  return indexes;
}
