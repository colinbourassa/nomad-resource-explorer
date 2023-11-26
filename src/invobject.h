#pragma once
#include <QMap>
#include <QImage>
#include <stdint.h>
#include "dattable.h"
#include "gametext.h"
#include "enums.h"
#include "palette.h"

enum class InventoryObjType
{
  Normal = 0,
  NormalWithText = 1,
  Engine = 2,
  Scanner = 3,
  Jammer = 4,
  Shield = 5,
  Missile = 6,
  MissileLoader = 7,
  ShipBotbooster = 0xA,
  LaborBot = 0xB,
  LaborBotEnhancement = 0xC,
  Award = 0xD,
  Translator = 0xE,
  Invalid = 0xFF
};

static const QMap<InventoryObjType,QString> s_objTypeNames =
{
  {InventoryObjType::Normal, "Normal"},
  {InventoryObjType::NormalWithText, "Normal (with descriptive text)"},
  {InventoryObjType::Engine, "Engine"},
  {InventoryObjType::Scanner, "Scanner"},
  {InventoryObjType::Jammer, "Jammer"},
  {InventoryObjType::Shield, "Shield"},
  {InventoryObjType::Missile, "Missile"},
  {InventoryObjType::MissileLoader, "Missile loader"},
  {InventoryObjType::ShipBotbooster, "Ship botbooster"},
  {InventoryObjType::LaborBot, "Labor bot"},
  {InventoryObjType::LaborBotEnhancement, "Labor bot enhancement"},
  {InventoryObjType::Award, "Award/medal/recognition"},
  {InventoryObjType::Translator, "Translator device"},
  {InventoryObjType::Invalid, "(invalid)"},

};

struct InventoryObj
{
  int id;
  QString name;
  bool tradeable;
  bool unique;
  InventoryObjType type;
  int subtype;
  QString objText;
  bool knownByPlayer;
  int valueByRace[static_cast<int>(AlienRace::NumRaces)];
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
  uint8_t valueByRace[static_cast<int>(AlienRace::NumRaces)];
  uint8_t unknown_d;
} ObjectTableEntry;

static_assert(sizeof(ObjectTableEntry) == 24, "ObjectTableEntry packing does not match game data");

class InvObject : public DatTable<ObjectTableEntry>
{
public:
  InvObject(DatLibrary& lib, Palette& pal, GameText& gtext);
  bool getImage(int id, QImage& img);
  QMap<int,InventoryObj> getList();
  InventoryObjType getObjectType(int id);
  QString getObjectText(int id);
  QString getName(int id);
  bool isUnique(const int id);
  void clear();

protected:
  bool populateList();

private:
  Palette* m_pal;
  GameText* m_gtext;
  QMap<int,InventoryObj> m_objList;
};

