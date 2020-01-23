#ifndef INVENTORY_H
#define INVENTORY_H

#include <QVector>
#include <QImage>
#include <stdint.h>
#include "datlibrary.h"
#include "enums.h"

struct InventoryObj
{
  unsigned int id;
  QString name;
  unsigned int valueByRace[AlienRace_NumRaces];
};

struct ObjectTableEntry
{
  uint16_t nameOffset;
  uint8_t remainingFields[22];
};

class Inventory
{
public:
  Inventory(DatLibrary& lib);
  bool getInventoryImage (unsigned int id, QImage* img);
  QVector<InventoryObj> getObjectList();

private:
  DatLibrary* m_lib;
};

#endif // INVENTORY_H

