#include "invobject.h"
#include "imageconverter.h"

InvObject::InvObject(DatLibrary& lib, Palette& pal) :
  m_lib(&lib),
  m_pal(&pal)
{

}

void InvObject::clear()
{
  m_objList.clear();
}

void InvObject::populateObjectList()
{
  QByteArray objdata;

  if (m_lib->getFileByName(DatFileType_CONVERSE, "OBJECT.TAB", objdata))
  {
    const uint8_t* rawdata = reinterpret_cast<const uint8_t*>(objdata.data());
    unsigned int offset = 0;
    int id = 0;

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
          m_objList.insert(id,obj);
        }
      }
      offset += sizeof(ObjectTableEntry);
      id++;
    }
  }
}

QMap<int,InventoryObj> InvObject::getObjectList()
{
  if (m_objList.isEmpty())
  {
    populateObjectList();
  }
  return m_objList;
}

QPixmap InvObject::getObjectImage(int id)
{
  bool status = false;
  QString invStpFilename = QString("inv%1.stp").arg(id, 4, 10, QChar('0'));
  QByteArray stpData;
  QPixmap objImage;

  if (m_lib->getFileByName(DatFileType_INVENT, invStpFilename, stpData))
  {
    QVector<QRgb> pal;
    if (m_pal->gamePalette(pal))
    {
      objImage = ImageConverter::stpToPixmap(stpData, pal, status);
    }
  }

  return objImage;
}

InventoryObjType InvObject::getObjectType(int id)
{
  InventoryObjType type = InventoryObjType_Invalid;

  if (m_objList.isEmpty())
  {
    populateObjectList();
  }

  if (m_objList.contains(id))
  {
    type = m_objList[id].type;
  }

  return type;
}

QString InvObject::getObjectName(int id)
{
  QString name("");

  if (m_objList.isEmpty())
  {
    populateObjectList();
  }

  if (m_objList.contains(id))
  {
    name = m_objList[id].name;
  }

  return name;
}
