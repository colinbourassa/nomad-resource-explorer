#include "conversationtext.h"
#include "gametext.h"
#include <stdint.h>
#include <string.h>
#include <QtEndian>

ConversationText::ConversationText(DatLibrary& lib, Aliens& aliens, GameText& gtext) :
  m_lib(&lib),
  m_aliens(&aliens),
  m_gtext(&gtext)
{

}

/**
 * Gets a list of dialog lines, each one being a response from the specified alien about the
 * thing with the provided ID in the specified conversation topic category.
 */
QString ConversationText::getConversationText(int alienId, ConvTopicCategory topic, int thingId, QMap<GTxtCmd,int>& commands)
{
  QString dialogLine;
  QByteArray tlktData;
  ConvTableType tableType = ConvTableType_Individual;
  int alienOrRaceId = alienId;

  getTLKTData(tableType, alienOrRaceId, tlktData);

  int tlknIndex = getTLKNIndex(topic, thingId, tlktData);

  // if the individual-specific tables didn't have any entries for this topic,
  // fall back to the generic tables used by all members of an alien race
  if (tlknIndex < 0)
  {
    tableType = ConvTableType_Race;
    alienOrRaceId = m_aliens->getRace(alienId);
    getTLKTData(tableType, alienOrRaceId, tlktData);
    tlknIndex = getTLKNIndex(topic, thingId, tlktData);
  }

  if (tlknIndex >= 0)
  {
    QByteArray tlknData;
    getTLKNData(tableType, alienOrRaceId, tlknData);

    // get the list of indices in the table of pointers to dialog strings
    int tlkxIndex = getTLKXIndex(tlknIndex, tlknData);

    QByteArray tlkxIndexData;
    QByteArray tlkxStrData;
    getTLKXData(tableType, alienOrRaceId, tlkxIndexData, tlkxStrData);
    dialogLine = getTLKXString(tlkxIndex, tlkxIndexData, tlkxStrData, commands);
  }

  return dialogLine;
}

/**
 * Opens the conversation line files -- one for each of the indices and strings -- for either the
 * race or individual alien specified by "id". Returns true if both files were opened and the data
 * stored in the provided QByteArrays; false otherwise.
 */
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

/**
 * Builds and returns a list of strings, each one having been indexed by the provided list of indices.
 * The data used is provided in the QByteArrays containing the contents of the relevant TLKX files.
 */
QString ConversationText::getTLKXString(int tlkxIndex,
                                        const QByteArray& tlkxIndexData,
                                        const QByteArray& tlkxStrData,
                                        QMap<GTxtCmd,int>& commands)
{
  const uint8_t* idxData = reinterpret_cast<const uint8_t*>(tlkxIndexData.data());

  const int tlkxIndexOffset = (tlkxIndex * TLKX_RECORDSIZE) + TLKX_RECORDSIZE;
  uint32_t tlkxStrOffset = 0;
  memcpy(&tlkxStrOffset, &idxData[tlkxIndexOffset], TLKX_RECORDSIZE);
  tlkxStrOffset = qFromLittleEndian<quint32>(tlkxStrOffset);

  const QString line = m_gtext->readString(tlkxStrData.data() + tlkxStrOffset, commands);

  return line;
}

/**
 * Reads the specified TLKN file (for either race or individual, specified by parameter) and returns
 * the data in the provided QByteArray. Returns true if reading was successful, false otherwise.
 */
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

/**
 * Reads the specified TLKT file (for either race or individual, specified by parameter) and returns
 * the data in the provided QByteArray. Returns true if reading was successful, false otherwise.
 */
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

/**
 * Returns the list of indices into the TLKX file set, given the provided list of TLKN indices and the
 * TLKN file data.
 */
int ConversationText::getTLKXIndex(int tlknIndex, const QByteArray& tlknData)
{
  const uint8_t* data = reinterpret_cast<const uint8_t*>(tlknData.data());
  const int offset = tlknIndex * TLKN_RECORDSIZE;
  const int tlkxIndex = data[offset] + (0x100 * data[offset + 1]);

  return tlkxIndex;
}

/**
 * Gets the list of indices from the provided TLKT data that match the specified thing ID in the specified
 * conversation topic category. If none match, an empty list is returned.
 */
int ConversationText::getTLKNIndex(ConvTopicCategory topic, int requestedId, const QByteArray& tlktData)
{
  int index = -1;
  int highestPriority = 0;

  const uint8_t* data = reinterpret_cast<const uint8_t*>(tlktData.data());
  int offset = 0;

  while (offset < tlktData.size())
  {
    const int firstByte = data[offset];
    const int priority  = data[offset + 1];
    const int alienId   = data[offset + 3];
    const int placeId   = data[offset + 4] + (0x100 * data[offset + 5]);
    const int objectId  = data[offset + 6];
    const int miscId    = data[offset + 7];
    const int tlknIndex = data[offset + 8] + (0x100 * data[offset + 9]);

    if (priority >= highestPriority)
    {
      if ((topic == ConvTopicCategory_GreetingInitial) && (firstByte == TLKN_CMD_GREETFIRST))
      {
        index = tlknIndex;
      highestPriority = priority;
      }
      else if ((topic == ConvTopicCategory_GreetingSubsequent) && (firstByte == TLKN_CMD_GREETNEXT))
      {
        index = tlknIndex;
      highestPriority = priority;
      }
      else if ((topic == ConvTopicCategory_DisplayObject) && (firstByte == TLKN_CMD_DISPOBJECT) && (objectId == requestedId))
      {
        index = tlknIndex;
      highestPriority = priority;
      }
      else if ((topic == ConvTopicCategory_GiveObject) && (firstByte == TLKN_CMD_GIVEOBJECT) && (objectId == requestedId))
      {
        index = tlknIndex;
      highestPriority = priority;
      }
      else if ((topic == ConvTopicCategory_SeesObject) && (firstByte == TLKN_CMD_SEESOBJ) && (objectId == requestedId))
      {
        index = tlknIndex;
      highestPriority = priority;
      }
      else if ((topic == ConvTopicCategory_AskAboutRace) && (firstByte == TLKN_CMD_ASKABOUTRACE) && (miscId == requestedId))
      {
        index = tlknIndex;
      highestPriority = priority;
      }
      else if ((topic == ConvTopicCategory_AskAboutPerson) && (firstByte == TLKN_CMD_ASKABOUT) && (alienId == requestedId))
      {
        index = tlknIndex;
      highestPriority = priority;
      }
      else if ((topic == ConvTopicCategory_AskAboutObject) && (firstByte == TLKN_CMD_ASKABOUT) && (objectId == requestedId))
      {
        index = tlknIndex;
      highestPriority = priority;
      }
      else if ((topic == ConvTopicCategory_AskAboutLocation) && (firstByte == TLKN_CMD_ASKABOUT) && (placeId == requestedId))
      {
        index = tlknIndex;
      highestPriority = priority;
      }
      else if ((topic == ConvTopicCategory_GiveFact) && (firstByte == TLKN_CMD_ASKABOUT) && (miscId == requestedId))
      {
        index = tlknIndex;
      highestPriority = priority;
      }

    }

    offset += TLKT_RECORDSIZE;
  }

  return index;
}

/**
 * Returns true if there are lines of dialogue associated specifically with the given alien ID,
 * about the provided topic ID in the provided category. Return false if the game will select
 * a generic fallback dialogue line for the provided combination.
 */
bool ConversationText::doesInterestingDialogExist(int alienId, ConvTopicCategory category, int thingId)
{
  bool status = false;

  QByteArray tlktData;
  ConvTableType tableType = ConvTableType_Individual;
  int alienOrRaceId = alienId;

  getTLKTData(tableType, alienOrRaceId, tlktData);

  int tlknIndex = getTLKNIndex(category, thingId, tlktData);

  if (tlknIndex >= 0)
  {
    status = true;
  }
  else
  {
    tableType = ConvTableType_Race;
    alienOrRaceId = m_aliens->getRace(alienId);
    getTLKTData(tableType, alienOrRaceId, tlktData);
    tlknIndex = getTLKNIndex(category, thingId, tlktData);

    status = (tlknIndex >= 0);
  }

  return status;
}
