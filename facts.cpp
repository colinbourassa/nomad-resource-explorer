#include "facts.h"

Facts::Facts(DatLibrary& lib) :
  DatTable<FactTableEntry> (lib)
{

}

Facts::~Facts()
{

}

void Facts::clear()
{
  m_factList.clear();
}

QMap<int,Fact> Facts::getList()
{
  if (m_factList.isEmpty())
  {
    populateList();
  }

  return m_factList;
}

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

QMap<AlienRace,int> Facts::getReceptivity(int id)
{
  QMap<AlienRace,int> recep;

  if (m_factList.contains(id))
  {
    recep = m_factList[id].receptivity;
  }

  return recep;
}

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
