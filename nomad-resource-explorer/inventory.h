#ifndef INVENTORY_H
#define INVENTORY_H

#include <QVector>
#include <QPixmap>
#include <stdint.h>
#include "datlibrary.h"
#include "enums.h"

enum InventoryObjType
{
  InventoryObjType_Normal = 0,
  InventoryObjType_NormalWithText = 1,
  InventoryObjType_Engine = 2,
  InventoryObjType_Scanner = 3,
  InventoryObjType_Jammer = 4,
  InventoryObjType_Shield = 5,
  InventoryObjType_Missile = 6,
  InventoryObjType_MissileLoader = 7,
  InventoryObjType_ShipBotbooster = 0xA,
  InventoryObjType_LaborBot = 0xB,
  InventoryObjType_LaborBotEnhancement = 0xC,
  InventoryObjType_Award = 0xD,
  InventoryObjType_Translator = 0xE
};

inline QString getInventoryObjTypeText(InventoryObjType type)
{
  switch (type)
  {
  case InventoryObjType_Normal:
    return QString("Normal");
  case InventoryObjType_NormalWithText:
    return QString("Normal (with descriptive text");
  case InventoryObjType_Engine:
    return QString("Engine");
  case InventoryObjType_Scanner:
    return QString("Scanner");
  case InventoryObjType_Jammer:
    return QString("Jammer");
  case InventoryObjType_Shield:
    return QString("Shield");
  case InventoryObjType_Missile:
    return QString("Missile");
  case InventoryObjType_MissileLoader:
    return QString("Missile loader");
  case InventoryObjType_ShipBotbooster:
    return QString("Ship botbooster");
  case InventoryObjType_LaborBot:
    return QString("Labor bot");
  case InventoryObjType_LaborBotEnhancement:
    return QString("Labor bot enhancement");
  case InventoryObjType_Award:
    return QString("Award/medal/recognition");
  case InventoryObjType_Translator:
    return QString("Translator device");
  }
}

struct InventoryObj
{
  unsigned int id;
  QString name;
  bool tradeable;
  bool unique;
  InventoryObjType type;
  unsigned int subtype;
  QString objText;
  bool knownByPlayer;
  unsigned int valueByRace[AlienRace_NumRaces];
};

typedef struct __attribute__((packed)) ObjectTableEntry
{
  uint16_t nameOffset;
  uint16_t unknown_a;
  uint16_t unknown_b;
  uint8_t isTradeable;
  uint8_t unknown_c;
  uint8_t type;
  uint8_t subtype;
  uint8_t flags;
  uint8_t valueByRace[AlienRace_NumRaces];
  uint8_t unusued;
} ObjectTableEntry;

class Inventory
{
public:
  Inventory(DatLibrary& lib);
  QPixmap getInventoryImage(unsigned int id);

  QVector<InventoryObj> getObjectList();

private:
  DatLibrary* m_lib;
};

#endif // INVENTORY_H

