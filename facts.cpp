#include "facts.h"

Facts::Facts(DatLibrary& lib) :
  DatTable<FactTableEntry> (lib)
{

}

Facts::~Facts()
{

}

/**
 * Clears locally cached data.
 */
void Facts::clear()
{
  m_factList.clear();
}

/**
 * Gets a container of data structures that each holds information about one of the
 * learnable facts from the game universe.
 */
QMap<int,Fact> Facts::getList()
{
  if (m_factList.isEmpty())
  {
    populateList();
  }

  return m_factList;
}

/**
 * Gets the data about the in-universe fact with the specified ID.
 * If no fact with the specified ID is found, the returned struct
 * will have an ID of 0 and empty string for its text.
 */
Fact Facts::getFact(int id) const
{
  if (m_factList.contains(id))
  {
    return m_factList[id];
  }

  Fact f;
  f.id = 0;
  f.text = QString();
  f.receptivity = QMap<AlienRace,int>();

  return f;
}

/**
 * Gets a map of each alien race's receptivity to the specified fact ID.
 * If no fact with the specified ID is found, an empty map will be returned.
 */
QMap<AlienRace,int> Facts::getReceptivity(int id)
{
  QMap<AlienRace,int> recep;

  if (m_factList.contains(id))
  {
    recep = m_factList[id].receptivity;
  }

  return recep;
}

/**
 * Reads and decodes the FACT.TAB data table.
 * @return True when FACT.TAB was successfully read, false otherwise.
 */
bool Facts::populateList()
{
  bool status = false;

  if (openFile(DatFileType_CONVERSE, "FACT.TAB"))
  {
    status = true;
    int index = 0;
    FactTableEntry* currentEntry = getEntry(index);

    while (currentEntry != nullptr)
    {
      if (currentEntry->textOffset != 0xFFFF)
      {
        Fact f;
        f.text = getGameText(currentEntry->textOffset);

        if (!f.text.isEmpty())
        {
          f.id = index;
          for (int raceId = 0; raceId < AlienRace_NumRaces; raceId++)
          {
            f.receptivity[static_cast<AlienRace>(raceId)] = currentEntry->receptivity[raceId] * 10;
          }

          m_factList.insert(index, f);
        }
      }
      index++;
      currentEntry = getEntry(index);
    }
  }

  return status;
}
