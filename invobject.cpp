#include <QtEndian>
#include "invobject.h"
#include "imageconverter.h"
#include "gametext.h"

InvObject::InvObject(DatLibrary& lib, Palette& pal, GameText& gtext) :
  DatTable<ObjectTableEntry> (lib),
  m_pal(&pal),
  m_gtext(&gtext)
{
}

/**
 * Clears locally cached data.
 */
void InvObject::clear()
{
  m_objList.clear();
}

/**
 * Iterates through all the entries in the object table, saving data about each
 * object in a local cache.
 * @return True when the object table was read successfully; false otherwise.
 */
bool InvObject::populateList()
{
  bool status = false;

  if (openFile(DatFileType::CONVERSE, "OBJECT.TAB"))
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
          obj.subtype = currentEntry->subtype;
          obj.knownByPlayer = (currentEntry->flags & 0x04);
          for (int raceid = 0; raceid < static_cast<int>(AlienRace::NumRaces); ++raceid)
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

/**
 * Gets a map of object IDs to structs containing data about those objects.
 */
QMap<int,InventoryObj> InvObject::getList()
{
  if (m_objList.isEmpty())
  {
    populateList();
  }
  return m_objList;
}

/**
 * Gets the informational text associated with a particular object ID.
 * @return Text string associated with the object ID, or an empty string if
 * no object with the specified ID could be found.
 */
QString InvObject::getObjectText(int id)
{
  QByteArray objTextIdxData;
  QByteArray objTextStrData;
  QString txt;

  if (m_objList.contains(id) && (m_objList[id].type == InventoryObjType::NormalWithText))
  {
    if (m_lib->getFileByName(DatFileType::CONVERSE, "OBJTEXT.IDX", objTextIdxData) &&
        m_lib->getFileByName(DatFileType::CONVERSE, "OBJTEXT.TXT", objTextStrData))
    {
      const int idxOffset = m_objList[id].subtype * 4;
      int32_t txtOffset = 0;

      memcpy(&txtOffset, objTextIdxData.data() + idxOffset, 4);
      txtOffset = qFromLittleEndian<qint32>(txtOffset);

      if (txtOffset < objTextStrData.size())
      {
        QVector<QPair<GTxtCmd,int> > commands;
        const char* rawdata = objTextStrData.data();
        txt = m_gtext->readString(rawdata + txtOffset, commands);
      }
    }
  }

  return txt;
}

/**
 * Returns an image of the object with the specified ID by reference.
 * @return True if the object ID was valid and its image was successfully
 * decoded; false otherwise.
 */
bool InvObject::getImage(int id, QImage& img)
{
  bool status = false;
  const QString invStpFilename = QString("inv%1.stp").arg(id, 4, 10, QChar('0'));
  QByteArray stpData;

  if (m_lib->getFileByName(DatFileType::INVENT, invStpFilename, stpData))
  {
    QVector<QRgb> pal;
    if (m_pal->gamePalette(pal))
    {
      status = ImageConverter::stpToImage(stpData, pal, img);
    }
  }

  return status;
}

/**
 * Gets the type of the object with the specified ID.
 * @return Object type. If no object with the specified ID can be found,
 * InventoryObjType_Invalid is returned.
 */
InventoryObjType InvObject::getObjectType(int id)
{
  InventoryObjType type = InventoryObjType::Invalid;

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

/**
 * Gets the name of the inventory object with the specified ID.
 * @return Name of object. If no object with the specified ID can be found,
 * an empty string is returned.
 */
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

/**
 * Returns the flag indicating whether the specified object
 * @return True if the "unique" flag is set, false otherwise.
 * A default value of false is returned if no object with the provided ID is found.
 */
bool InvObject::isUnique(const int id)
{
  if (m_objList.contains(id))
  {
    return m_objList[id].unique;
  }

  return false;
}

