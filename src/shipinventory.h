#ifndef SHIPINVENTORY_H
#define SHIPINVENTORY_H

#include "datlibrary.h"
#include <QMap>

#define INVENT_TABLE_RECORD_SIZE_BYTES 4
#define MAX_SHIP_ID 255

/**
 * Reads and parses the data table that describes the starting inventory for
 * each ship in the game. This class does not use the DatTable template because
 * it structure is not a regular sequential list of records, but rather a
 * linked-list style series.
 */
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
