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
  m_pclassList.clear();
}

void PlaceClasses::populatePlaceClassList()
{
  QByteArray pclassdata;

  if (m_lib->getFileByName(DatFileType_CONVERSE, "PCLASS.TAB", pclassdata))
  {
    const uint8_t* rawdata = reinterpret_cast<const uint8_t*>(pclassdata.data());
    unsigned int offset = 0;
    int id = 0;

    while (offset <= (pclassdata.size() - sizeof(PClassTableEntry)))
    {
      const PClassTableEntry* currentEntry = reinterpret_cast<const PClassTableEntry*>(rawdata + offset);

      // the place class is only valid if the name offset into GAMETEXT.TXT is not 0xFFFF
      if (currentEntry->nameOffset != 0xFFFF)
      {
        PlaceClass pclass;
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
          m_pclassList.insert(id, pclass);
        }
      }
      offset += sizeof(PClassTableEntry);
      id++;
    }
  }
}

QMap<int,PlaceClass>* PlaceClasses::pclassDataList()
{
  if (m_pclassList.isEmpty())
  {
    populatePlaceClassList();
  }

  return &m_pclassList;
}

bool PlaceClasses::pclassData(int id, PlaceClass& pclass)
{
  bool status = false;

  if (m_pclassList.isEmpty())
  {
    populatePlaceClassList();
  }

  if (m_pclassList.contains(id))
  {
    status = true;
    pclass = m_pclassList[id];
  }

  return status;
}
