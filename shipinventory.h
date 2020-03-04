#ifndef SHIPINVENTORY_H
#define SHIPINVENTORY_H

#include "dattable.h"
#include <QMap>

#define INVENT_TABLE_RECORD_SIZE_BYTES 4
#define MAX_SHIP_ID 255

class ShipInventory
{
public:
  ShipInventory(DatLibrary& lib);
  QMap<int,int> getInventory(int shipId);
  void clear();

private:
  DatLibrary* m_lib;
  QMap<int, QMap<int,int> > m_inventories;

  void populateInventoryData();
};

#endif // SHIPINVENTORY_H
