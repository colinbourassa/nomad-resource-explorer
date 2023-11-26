#pragma once
#include <stdint.h>
#include <QVector>
#include <QPair>
#include <QMap>
#include "dattable.h"
#include "datlibrary.h"
#include "gametext.h"

enum class MissionActionType
{
  None,
  DestroyShip,
  DeliverItem,
  Unknown
};

struct Mission
{
  MissionActionType action;
  int missionActionRawVal;
  int objectiveId;
  int objectiveLocation;
  QString startText;
  QString completeText;
  QVector<QPair<GTxtCmd,int> > startTextCommands;
  QVector<QPair<GTxtCmd,int> > completeTextCommands;
};

typedef struct __attribute__((packed)) MissionTableEntry
{
  uint16_t placeId;
  uint8_t  unknown_a[13];
  uint8_t  prereqMissionId;
  uint8_t  actionRequired;
  uint8_t  objectiveId;
  uint8_t  unknown_b[2];
  uint16_t startTextIndex;
  uint16_t completeTextIndex;

} MissionTableEntry;

static_assert(sizeof(MissionTableEntry) == 24, "MissionTableEntry packing does not match game data");

class Missions : public DatTable<MissionTableEntry>
{
public:
  Missions(DatLibrary& lib, GameText& gametext);
  QMap<int,Mission> getList();

protected:
  bool populateList();

private:
  GameText* m_gtext;
  QMap<int,Mission> m_missions;

  QString getMissionText(uint16_t idxFileIndex, QVector<QPair<GTxtCmd,int> >& commands);
};

