#include "places.h"

Places::Places(DatLibrary& lib) : m_lib(&lib)
{

}

void Places::clear()
{
  m_placeList.clear();
}

void Places::populatePlaceList()
{
  QByteArray placedata;

  if (m_lib->getFileByName(DatFileType_CONVERSE, "PLACE.TAB", placedata))
  {
    const uint8_t* rawdata = reinterpret_cast<const uint8_t*>(placedata.data());
    unsigned int offset = 0;
    int id = 0;

    while (offset <= (placedata.size() - sizeof(PlaceTableEntry)))
    {
      const PlaceTableEntry* currentEntry = reinterpret_cast<const PlaceTableEntry*>(rawdata + offset);

      // the object is only valid if the name offset into GAMETEXT.TXT is not 0xFFFF
      if (currentEntry->nameOffset != 0xFFFF)
      {
        Place p;
        p.id = id;
        p.name = m_lib->getGameText(currentEntry->nameOffset);
        p.representativeId = currentEntry->planetRepId;
        //p.knownByPlayer = (currentEntry->flags & 0x04);

        if (!p.name.isEmpty())
        {
          m_placeList.insert(id,p);
        }
      }
      offset += sizeof(PlaceTableEntry);
      id++;
    }
  }
}

QMap<int,Place> Places::getPlaceList()
{
  return m_placeList;
}

QPixmap Places::getPlaceSurfaceImage(int id)
{

}
