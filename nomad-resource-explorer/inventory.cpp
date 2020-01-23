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
