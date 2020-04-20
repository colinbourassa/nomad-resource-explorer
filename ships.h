#ifndef SHIPS_H
#define SHIPS_H

#include <QString>
#include <QMap>
#include "dattable.h"

struct Ship
{
  int id;
  QString name;
  int shipclass;
  QMap<int,int> inventory;
  int location;
  int pilot;
};

/**
 * SHIP.TAB record describing ship parameters. Note that many of these fields
 * are only used during gameplay (i.e. they are not relevant in the context of
 * initial game state), so they would only contain interesting data when reading
 * the copy of the ship table that is stored in saved game files.
 */
typedef struct __attribute__((packed)) ShipTableEntry
{
  uint16_t nameOffset;
  uint8_t pilot;
  uint8_t shipclass;
  uint8_t unknown_a[0x1E];
  uint16_t location;
  uint16_t engagedShipPtr;
  uint8_t weaponType;
  uint8_t weaponSystemDamage;
  uint8_t missileLoaderType;
  uint8_t unknown_b;
  uint8_t missileLoadDowncounter;
  uint8_t currentlyLoadedMissileType;
  uint8_t missileLockDowncounter;
  uint8_t unknown_c[5];
  uint8_t shieldRelatedA;
  uint8_t shieldSystemDamage;
  uint8_t shieldRelatedB;
  uint8_t unknown_d[6];
  uint8_t scannerSystemDamage;
  uint8_t scannerType;
  uint8_t unknown_e[5];
  uint8_t engineSystemDamage;
  uint8_t engineType;
  uint8_t jammerSystemDamage;
  uint8_t jammerType;
} ShipTableEntry;

/**
 * Reads and parses the game's ship table (SHIP.TAB).
 */
class Ships : public DatTable<ShipTableEntry>
{
public:
  Ships(DatLibrary& lib);
  virtual ~Ships();
  QMap<int,Ship> getList();
  QString getName(int id);

protected:
  bool populateList();

private:
  QMap<int,Ship> m_shipList;
};

#endif // SHIPS_H
