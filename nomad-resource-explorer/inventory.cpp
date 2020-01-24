#include "inventory.h"

Inventory::Inventory(DatLibrary& lib) :
  m_lib(&lib)
{

}

QVector<InventoryObj> Inventory::getObjectList()
{
  QVector<InventoryObj> list;
  QByteArray objdata;

  if (m_lib->getFileByName(DatFileType_CONVERSE, "OBJECT.TAB", objdata))
  {
    const uint8_t* rawdata = reinterpret_cast<const uint8_t*>(objdata.data());
    unsigned int offset = 0;
    unsigned int id = 0;

    while (offset <= (objdata.size() - sizeof(ObjectTableEntry)))
    {
      const ObjectTableEntry* currentEntry = reinterpret_cast<const ObjectTableEntry*>(rawdata + offset);

      // the object is only valid if the name offset into GAMETEXT.TXT is not 0xFFFF
      if (currentEntry->nameOffset != 0xFFFF)
      {
        InventoryObj obj;
        obj.id = id;
        obj.name = m_lib->getGameText(currentEntry->nameOffset);
        obj.tradeable = currentEntry->isTradeable;
        obj.unique = (currentEntry->type & 0x80);
        obj.type = static_cast<InventoryObjType>(currentEntry->type & 0x7F);
        obj.knownByPlayer = (currentEntry->flags & 0x04);
        for (int raceid = 0; raceid < AlienRace_NumRaces; ++raceid)
        {
          obj.valueByRace[raceid] = currentEntry->valueByRace[raceid];
        }

        if (!obj.name.isEmpty())
        {
          list.append(obj);
        }
      }
      offset += sizeof(ObjectTableEntry);
      id++;
    }
  }

  return list;
}

QPixmap Inventory::getInventoryImage(unsigned int id)
{
  bool status = false;
  QString invStpFilename = QString("inv%1.stp").arg(id, 4, 10, QChar('0'));
  QByteArray stpData;
  QPixmap objImage;

  if (m_lib->getFileByName(DatFileType_INVENT, invStpFilename, stpData))
  {
    objImage = m_lib->convertStpToPixmap(stpData, m_lib->getGamePalette(), status);
  }

  return objImage;
}
