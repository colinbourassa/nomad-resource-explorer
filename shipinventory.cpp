#include "shipinventory.h"
#include <QByteArray>

ShipInventory::ShipInventory(DatLibrary& lib) :
  m_lib(&lib)
{

}

void ShipInventory::clear()
{
  m_inventories.clear();
}

QMap<int,int> ShipInventory::getInventory(int shipId)
{
  QMap<int,int> invent;

  if (m_inventories.keys().isEmpty())
  {
    populateInventoryData();
  }

  if (m_inventories.contains(shipId))
  {
    invent = m_inventories[shipId];
  }

  return invent;
}

void ShipInventory::populateInventoryData()
{
  QByteArray inventdata;

  if (m_lib->getFileByName(DatFileType_CONVERSE, "INVENT.TAB", inventdata))
  {
    const uint8_t* rawdata = reinterpret_cast<const uint8_t*>(inventdata.data());
    int shipRecordOffset = 0;
    int shipid = 0;

    while ((shipid < MAX_SHIP_ID) &&
           (shipRecordOffset <= (inventdata.size() - INVENT_TABLE_RECORD_SIZE_BYTES)))
    {
      const uint8_t numberOfItemRecords = rawdata[shipRecordOffset + 1];
      uint16_t currentItemRecordIndex = (rawdata[shipRecordOffset + 3] * 256) + rawdata[shipRecordOffset + 2];
      int currentItemRecordOffset = currentItemRecordIndex * INVENT_TABLE_RECORD_SIZE_BYTES;
      int currentItemRecordNum = 0;

      while ((currentItemRecordNum < numberOfItemRecords) &&
             (currentItemRecordOffset <= (inventdata.size() - INVENT_TABLE_RECORD_SIZE_BYTES)))
      {
        const uint8_t objectId = rawdata[currentItemRecordOffset];
        const uint8_t objectCount = rawdata[currentItemRecordOffset + 1];
        m_inventories[shipid].insert(objectId, objectCount);
        currentItemRecordNum++;
        currentItemRecordIndex = (rawdata[currentItemRecordOffset + 3] * 256) + rawdata[currentItemRecordOffset + 2];
        currentItemRecordOffset = currentItemRecordIndex * INVENT_TABLE_RECORD_SIZE_BYTES;
      }

      shipRecordOffset += INVENT_TABLE_RECORD_SIZE_BYTES;
      shipid++;
    }
  }
}
