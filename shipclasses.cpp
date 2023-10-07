#include "shipclasses.h"
#include <QtEndian>

ShipClasses::ShipClasses(DatLibrary& lib) :
  DatTable<ShipClassTableEntry> (lib)
{
}

/**
 * Gets a map of all ship classes, keyed by class ID.
 */
QMap<int,ShipClass> ShipClasses::getList()
{
  if (m_shipClasses.isEmpty())
  {
    populateList();
  }

  return m_shipClasses;
}

/**
 * Reads the ship class data table (SCLASS.TAB), parses, and stores the data.
 * @return
 */
bool ShipClasses::populateList()
{
  bool status = false;

  if (openFile(DatFileType::CONVERSE, "SCLASS.TAB"))
  {
    status = true;
    int index = 0;

    ShipClassTableEntry* currentEntry = getEntry(index);
    while (currentEntry != nullptr)
    {
      if (currentEntry->nameOffset != 0xFFFF)
      {
        ShipClass sc;
        sc.name = getGameText(qFromLittleEndian<quint16>(currentEntry->nameOffset));

        if (!sc.name.isEmpty())
        {
          m_shipClasses.insert(index, sc);
        }
      }
      index++;
      currentEntry = getEntry(index);
    }
  }

  return status;
}

/**
 * Gets the name of the ship class with the provided ID.
 * @return Ship class name, or empty string if no class with the provided ID was found.
 */
QString ShipClasses::getName(int id)
{
  QString name;

  if (m_shipClasses.isEmpty())
  {
    populateList();
  }

  if (m_shipClasses.contains(id))
  {
    name = m_shipClasses[id].name;
  }

  return name;
}

