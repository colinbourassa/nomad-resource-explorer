#include <stdint.h>
#include "ships.h"
#include <QByteArray>

Ships::Ships(DatLibrary& lib) :
  DatTable<ShipTableEntry> (lib)
{

}

Ships::~Ships()
{

}

/**
 * Gets a map of ship IDs to ship data structs, representing
 * all ships in the game.
 */
QMap<int,Ship> Ships::getList()
{
  if (m_shipList.isEmpty())
  {
    populateList();
  }

  return m_shipList;
}

QString Ships::getName(int id)
{
  QString name;

  if (m_shipList.isEmpty())
  {
    populateList();
  }

  if (m_shipList.contains(id))
  {
    name = m_shipList[id].name;
  }

  return name;
}

/**
 * Reads and parses records in the game's ship table (SHIP.TAB).
 */
bool Ships::populateList()
{
  bool status = false;

  if (openFile(DatFileType_CONVERSE, "SHIP.TAB"))
  {
    status = true;
    int index = 0;
    ShipTableEntry* currentEntry = getEntry(index);

    while (currentEntry != nullptr)
    {
      if (currentEntry->nameOffset != 0xFFFF)
      {
        Ship s;
        s.name = getGameText(currentEntry->nameOffset);

        if (!s.name.isEmpty())
        {
          s.id = index;
          s.location = currentEntry->location;
          s.pilot = currentEntry->pilot;
          s.shipclass = currentEntry->shipclass;

          m_shipList.insert(index, s);
        }
      }
      index++;
      currentEntry = getEntry(index);
    }
  }

  return status;
}
