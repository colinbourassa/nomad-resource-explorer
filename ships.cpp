#include <stdint.h>
#include "ships.h"
#include <QByteArray>

Ships::Ships(DatLibrary& lib) :
  m_lib(&lib)
{

}

QMap<int,Ship> Ships::getShipList()
{
  if (m_shipList.isEmpty())
  {
    populateShipList();
  }

  return m_shipList;
}

bool Ships::populateShipList()
{
  bool status = false;
  QByteArray shipdata;

  if (m_lib->getFileByName(DatFileType_CONVERSE, "SHIP.TAB", shipdata))
  {
    status = true;
    const uint8_t* rawdata = reinterpret_cast<const uint8_t*>(shipdata.data());
    unsigned int offset = 0;
    int id = 0;

    while (offset <= (shipdata.size() - sizeof(ShipTableEntry)))
    {
      const ShipTableEntry* currentEntry = reinterpret_cast<const ShipTableEntry*>(rawdata + offset);

      // the object is only valid if the name offset into GAMETEXT.TXT is not 0xFFFF
      if (currentEntry->nameOffset != 0xFFFF)
      {
        Ship s;
        s.name = m_lib->getGameText(currentEntry->nameOffset);

        if (!s.name.isEmpty())
        {
          s.id = id;
          s.location = currentEntry->location;
          s.pilot = currentEntry->pilot;

          m_shipList.insert(id, s);
        }
      }
      offset += sizeof(ShipTableEntry);
      id++;
    }
  }

  return status;
}
