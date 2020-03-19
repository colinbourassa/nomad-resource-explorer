#include "gametext.h"

GameText::GameText(DatLibrary& lib) :
  m_lib(&lib)
{

}

QString GameText::getMetaString(int metaTabIndex)
{
  QString metaStr;
  bool status = true;

  if (m_metaTab.isEmpty())
  {
    status = m_lib->getFileByName(DatFileType_CONVERSE, "META.TAB", m_metaTab);
  }

  if (status && m_metaTextTab.isEmpty())
  {
    status = m_lib->getFileByName(DatFileType_CONVERSE, "METATXT.TAB", m_metaTextTab);
  }

  if (status)
  {
    const uint8_t* metaData = reinterpret_cast<const uint8_t*>(m_metaTab.data());
    const int metaTabOffset   = metaTabIndex * METATAB_RECORDSIZE_BYTES;
    const int numberOfOptions = metaData[metaTabOffset + 0];
    const int type            = metaData[metaTabOffset + 1];
    const int metaTextIndex   = metaData[metaTabOffset + 2] + (0x100 * metaData[metaTabOffset + 3]);

    if ((type == METATAB_TYPE_SYNONYM) || (type == METATAB_TYPE_TRANSLATION))
    {
      const uint8_t* metaTextData = reinterpret_cast<const uint8_t*>(m_metaTextTab.data());
      const int metaTextOffset = metaTextIndex * METATEXT_RECORDSIZE_BYTES;
      const int gametextOffset = metaTextData[metaTextOffset] + (0x100 * metaTextData[metaTextOffset + 1]);
      metaStr = m_lib->getGameText(gametextOffset);
    }
    else if (type == METATAB_TYPE_LOSTENGATEWAY)
    {
      metaStr = QString("<Losten gateway code #%1>").arg(metaTextIndex + 1);
    }
  }

  return metaStr;
}

QString GameText::readString(const char* data, int maxlen)
{
  QString clean;

  int pos = 0;

  while ((data[pos] != 0) && (pos < maxlen))
  {
    // all normally printable characters are passed through to the output directly
    if (data[pos] >= 0x20)
    {
      clean.append(data[pos]);
      pos++;
    }
    else
    {
      const GTxtCmd cmd = static_cast<GTxtCmd>(data[pos]);
      pos++;

      if (cmd == GTxtCmd_InsertPlayerName)
      {
        clean.append("<Player's name>");
      }
      else if (cmd == GTxtCmd_InsertPlayerShip)
      {
        clean.append("<Player's ship>");
      }
      else if (cmd == GTxtCmd_GAMESTR)
      {
        clean.append("<GAMESTR>");
      }
      else if (cmd == GTxtCmd_METAMOVE)
      {
        clean.append("<METAMOVE>");
      }
      else if (cmd == GTxtCmd_MetaText)
      {
        uint16_t metaTabIdx = (static_cast<uint8_t>(data[pos]) + (0x100 * static_cast<uint8_t>(data[pos+1]))) - 1;
        const QString metaStr = getMetaString(metaTabIdx);
        clean.append(metaStr);
      }
      else if (cmd == GTxtCmd_InsertCurrentLocation)
      {
        clean.append("<current location>");
      }
      // because the number of places in the game can require a two-byte identifier,
      // we need to check whether an additional byte must be consumed as a parameter to this command
      else if (cmd == GTxtCmd_GrantKnowledgePlace)
      {
        const uint8_t firstParamByte = static_cast<uint8_t>(data[pos]);
        if (firstParamByte & 0x80)
        {
          pos++;
        }
      }

      pos += g_gameTextParamCount[cmd];
    }
  }

  return clean;
}
