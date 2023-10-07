#include "missions.h"
#include <QtEndian>

Missions::Missions(DatLibrary& lib, GameText& gametext) :
  DatTable<MissionTableEntry>(lib),
  m_gtext (&gametext)
{

}

/**
 * Gets the list of missions read from the data file, populating it first if necessary.
 */
QMap<int,Mission> Missions::getList()
{
  if (m_missions.isEmpty())
  {
    populateList();
  }

  return m_missions;
}

/**
 * Parses the MISSION.TAB data file to read information about Alliance mission postings.
 */
bool Missions::populateList()
{
  bool status = false;

  if (openFile(DatFileType::CONVERSE, "MISSION.TAB"))
  {
    status = true;
    int index = 0;
    MissionTableEntry* currentEntry = getEntry(index);

    while (currentEntry != nullptr)
    {
      // placeholder: not sure of the best way to determine whether a MISSION.TAB entry is
      // valid, but I've noticed that the fourth byte is always 0x01 for every valid entry
      if (currentEntry->unknown_a[1] == 0x01)
      {
        Mission m;
        if (currentEntry->actionRequired == 0)
        {
          m.action = MissionActionType::None;
        }
        else if (currentEntry->actionRequired == 2)
        {
          m.action = MissionActionType::DestroyShip;
        }
        else if (currentEntry->actionRequired == 3)
        {
          m.action = MissionActionType::DeliverItem;
        }
        else
        {
          m.action = MissionActionType::Unknown;
        }
        m.missionActionRawVal = currentEntry->actionRequired;
        m.startText    = getMissionText(qFromLittleEndian<quint16>(currentEntry->startTextIndex), m.startTextCommands);
        m.completeText = getMissionText(qFromLittleEndian<quint16>(currentEntry->completeTextIndex), m.completeTextCommands);
        m.objectiveId  = currentEntry->objectiveId;
        m.objectiveLocation = currentEntry->placeId;

        m_missions.insert(index, m);
      }
      index++;
      currentEntry = getEntry(index);
    }
  }

  return status;
}

/**
 * Returns the text associated with a missions at the provided index in the MISTEXT.IDX index file.
 * Populates the provided QVector with the list of game commands embedded in that text.
 */
QString Missions::getMissionText(uint16_t idxFileIndex, QVector<QPair<GTxtCmd,int> >& commands)
{
  QByteArray misTextIdxData;
  QByteArray misTextStrData;
  QString txt;

  if (m_lib->getFileByName(DatFileType::CONVERSE, "MISTEXT.IDX", misTextIdxData) &&
      m_lib->getFileByName(DatFileType::CONVERSE, "MISTEXT.TXT", misTextStrData))
  {
    const int idxOffset = (idxFileIndex + 1) * 4;
    int32_t txtOffset = 0;

    memcpy(&txtOffset, misTextIdxData.data() + idxOffset, 4);
    txtOffset = qFromLittleEndian<qint32>(txtOffset);

    if (txtOffset < misTextStrData.size())
    {
      const char* rawdata = misTextStrData.data();
      txt = m_gtext->readString(rawdata + txtOffset, commands);
    }
  }

  return txt;
}
