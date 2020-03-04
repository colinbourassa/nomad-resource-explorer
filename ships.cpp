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

QMap<int,Ship> Ships::getList()
{
  if (m_shipList.isEmpty())
  {
    populateList();
  }

  return m_shipList;
}

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
