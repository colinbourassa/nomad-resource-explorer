#include "placeclasses.h"

/**
 * Human readable names for each planet temperature range. The key in this map
 * is the maximum temperature (in Fahrenheit degrees, apparently) that is
 * considered part of the range named by the value string.
 */
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

/**
 * Clears locally cached data.
 */
void PlaceClasses::clear()
{
  m_planetClassList.clear();
  m_starClassList.clear();
}

/**
 * Iterates through the entries in the PCLASS.TAB data table, parsing out the fields
 * and storing the data.
 */
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

        // there are three slots available for each of the planet's resources
        for (int rIdx = 0; rIdx< 3; rIdx++)
        {
          if (currentEntry->foods[rIdx])
          {
            pclass.resources[PlanetResourceType_Food].insert(currentEntry->foods[rIdx],
                                                             currentEntry->foodsAgriculture[rIdx]);
          }
          if (currentEntry->ores[rIdx])
          {
            pclass.resources[PlanetResourceType_Mineral].insert(currentEntry->ores[rIdx],
                                                                currentEntry->oresConcentration[rIdx]);
          }
          if (currentEntry->ancientArtifacts[rIdx])
          {
            pclass.resources[PlanetResourceType_ArchaeologicalArtifact].insert(currentEntry->ancientArtifacts[rIdx],
                                                                               currentEntry->ancientArtifactsConcentration[rIdx]);
          }
          if (currentEntry->gasses[rIdx])
          {
            pclass.resources[PlanetResourceType_Gas].insert(currentEntry->gasses[rIdx],
                                                            currentEntry->gassesConcentration[rIdx]);
          }
          if (currentEntry->animals[rIdx])
          {
            pclass.resources[PlanetResourceType_Animal].insert(currentEntry->animals[rIdx],
                                                               currentEntry->animalsConcentration[rIdx]);
          }
          if (currentEntry->intelligenceItems[rIdx])
          {
            pclass.resources[PlanetResourceType_EspionageItem].insert(currentEntry->intelligenceItems[rIdx],
                                                                      currentEntry->intelligenceItemsConcentration[rIdx]);
          }
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

/**
 * Retrieves the class data for a planet, and return it in the provided struct.
 * @return True when the data for the planet ID was found; false otherwise.
 */
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

/**
 * Gets the human readable name for the class of a star.
 * @return Star class name if the provided ID matches that of a star from the data table.
 * If a star with the provided ID is not found, empty string is returned.
 */
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
