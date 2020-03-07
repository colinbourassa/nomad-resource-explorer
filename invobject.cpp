#include "invobject.h"
#include "imageconverter.h"

InvObject::InvObject(DatLibrary& lib, Palette& pal) :
  DatTable<ObjectTableEntry> (lib),
  m_pal(&pal)
{

}

InvObject::~InvObject()
{

}

void InvObject::clear()
{
  m_objList.clear();
}

bool InvObject::populateList()
{
  bool status = false;

  if (openFile(DatFileType_CONVERSE, "OBJECT.TAB"))
  {
    status = true;
    int index = 0;
    ObjectTableEntry* currentEntry = getEntry(index);

    while (currentEntry != nullptr)
    {
      if (currentEntry->nameOffset != 0xFFFF)
      {
        InventoryObj obj;
        obj.name = getGameText(currentEntry->nameOffset);

        if (!obj.name.isEmpty())
        {
          obj.id = index;
          obj.tradeable = currentEntry->isTradeable;
          obj.unique = (currentEntry->type & 0x80);
          obj.type = static_cast<InventoryObjType>(currentEntry->type & 0x7F);
          obj.knownByPlayer = (currentEntry->flags & 0x04);
          for (int raceid = 0; raceid < AlienRace_NumRaces; ++raceid)
          {
            obj.valueByRace[raceid] = currentEntry->valueByRace[raceid];
          }

          m_objList.insert(index, obj);
        }
      }
      index++;
      currentEntry = getEntry(index);
    }
  }

  return status;
}

QMap<int,InventoryObj> InvObject::getList()
{
  if (m_objList.isEmpty())
  {
    populateList();
  }
  return m_objList;
}

QPixmap InvObject::getImage(int id, bool& status)
{
  status = false;
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
    populateList();
  }

  if (m_objList.contains(id))
  {
    type = m_objList[id].type;
  }

  return type;
}

QString InvObject::getName(int id)
{
  QString name("");

  if (m_objList.isEmpty())
  {
    populateList();
  }

  if (m_objList.contains(id))
  {
    name = m_objList[id].name;
  }

  return name;
}
