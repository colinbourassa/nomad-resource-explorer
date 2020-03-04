#include "placeclasses.h"

const QMap<int,QString> PlaceClasses::s_tempRanges =
{
  {-120, "Arctic"},
  {39, "Frozen"},
  {99, "Temperate"},
  {149, "Tropic"},
  {899, "Searing"},
  {0xffff, "Molten"}
};

PlaceClasses::PlaceClasses(DatLibrary& lib) :
  m_lib(&lib)
{

}

void PlaceClasses::clear()
{
  m_planetClassList.clear();
  m_starClassList.clear();
}

void PlaceClasses::populatePlaceClassList()
{
  QByteArray classdata;
  clear();

  if (m_lib->getFileByName(DatFileType_CONVERSE, "PCLASS.TAB", classdata))
  {
    const uint8_t* rawdata = reinterpret_cast<const uint8_t*>(classdata.data());
    unsigned int offset = 0;
    int id = 0;

    while (offset <= (classdata.size() - sizeof(PClassTableEntry)))
    {
      const PClassTableEntry* currentEntry = reinterpret_cast<const PClassTableEntry*>(rawdata + offset);

      // the place class is only valid if the name offset into GAMETEXT.TXT is not 0xFFFF
      if (currentEntry->nameOffset != 0xFFFF)
      {
        PlanetClass pclass;
        pclass.name = m_lib->getGameText(currentEntry->nameOffset);
        pclass.temperature = currentEntry->temperature;

        for (int rIdx = 0; rIdx< 3; rIdx++)
        {
          pclass.foods.insert(currentEntry->foods[rIdx],
                              currentEntry->foodsAgriculture[rIdx]);
          pclass.ores.insert(currentEntry->ores[rIdx],
                             currentEntry->oresConcentration[rIdx]);
          pclass.ancientArtifacts.insert(currentEntry->ancientArtifacts[rIdx],
                                         currentEntry->ancientArtifactsConcentration[rIdx]);
          pclass.gasses.insert(currentEntry->gasses[rIdx],
                               currentEntry->gassesConcentration[rIdx]);
          pclass.animals.insert(currentEntry->animals[rIdx],
                                currentEntry->animalsConcentration[rIdx]);
          pclass.intelligenceArtifacts.insert(currentEntry->intelligenceItems[rIdx],
                                              currentEntry->intelligenceItemsConcentration[rIdx]);

        }

        int tempRangeIdx = 0;
        while ( (tempRangeIdx < s_tempRanges.count()) &&
                (pclass.temperature > s_tempRanges.keys().at(tempRangeIdx)) )
        {
          tempRangeIdx++;
        }
        const int key = s_tempRanges.keys().at(tempRangeIdx);
        pclass.temperatureRange = s_tempRanges[key];

        if (!pclass.name.isEmpty())
        {
          m_planetClassList.insert(id, pclass);
        }
      }
      offset += sizeof(PClassTableEntry);
      id++;
    }
  }

  classdata.clear();

  if (m_lib->getFileByName(DatFileType_CONVERSE, "STCLASS.TAB", classdata))
  {
    const uint8_t* rawdata = reinterpret_cast<const uint8_t*>(classdata.data());
    unsigned int offset = 0;
    int id = 0;

    while (offset <= (classdata.size() - sizeof(StClassTableEntry)))
    {
      const StClassTableEntry* currentEntry = reinterpret_cast<const StClassTableEntry*>(rawdata + offset);

      // the place class is only valid if the name offset into GAMETEXT.TXT is not 0xFFFF
      if (currentEntry->nameOffset != 0xFFFF)
      {
        StarClass stclass;
        stclass.name = m_lib->getGameText(currentEntry->nameOffset);

        if (!stclass.name.isEmpty())
        {
          m_starClassList.insert(id, stclass);
        }
      }
      offset += sizeof(StClassTableEntry);
      id++;
    }
  }
}

const QMap<int,PlanetClass>* PlaceClasses::planetClassDataList()
{
  if (m_planetClassList.isEmpty())
  {
    populatePlaceClassList();
  }

  return &m_planetClassList;
}

bool PlaceClasses::pclassData(int id, PlanetClass& pclass)
{
  bool status = false;

  if (m_planetClassList.isEmpty())
  {
    populatePlaceClassList();
  }

  if (m_planetClassList.contains(id))
  {
    status = true;
    pclass = m_planetClassList[id];
  }

  return status;
}

QString PlaceClasses::getStarClassName(int id)
{
  QString name;

  if (m_starClassList.isEmpty())
  {
    populatePlaceClassList();
  }

  if (m_starClassList.contains(id))
  {
    name = m_starClassList[id].name;
  }

  return name;
}
