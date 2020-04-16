#include "gametext.h"
#include <QString>
#include <QUrl>

/**
 * Processes the mission and conversation text in the game by removing the special
 * nonprintable command bytes and inserting the proper substitution text.
 */
GameText::GameText(DatLibrary& lib) :
  m_lib(&lib)
{

}

/**
 * Returns a string that may be substituted for a metatext command. The metatext commands
 * are one of three types:
 * - Conversational synonym. Used to add variety to the alien dialog.
 * - Translated phrases. Used to (partially) translate the Shaasa alien language.
 * - Losten gateway code. Used to create (or recall) one of the randomly generated codes
 *   for the Losten planetary gateway.
 */
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
      QStringList synonyms;
      const uint8_t* metaTextData = reinterpret_cast<const uint8_t*>(m_metaTextTab.data());

      // build a string list of all the alternatives/options for this this metatext
      for (int optionIndex = 0; optionIndex < numberOfOptions; optionIndex++)
      {
        const int metaTextOffset = (metaTextIndex + optionIndex) * METATEXT_RECORDSIZE_BYTES;
        const int gametextOffset = metaTextData[metaTextOffset] + (0x100 * metaTextData[metaTextOffset + 1]);
        synonyms.append(m_lib->getGameText(gametextOffset));
      }

      if (synonyms.size() > 0)
      {
        // build a URL for the anchor that is just a pipe-separated list of the synonyms
        metaStr = "<a href=\"";
        foreach (QString synonym, synonyms)
        {
          metaStr += synonym + "|";
        }
        metaStr += "\">" + synonyms[0] + "</a>";
      }
    }
    else if (type == METATAB_TYPE_LOSTENGATEWAY)
    {
      metaStr = QString("<Losten gateway code #%1>").arg(metaTextIndex + 1).toHtmlEscaped();
    }
  }

  return metaStr;
}

/**
 * Walks through the provided ASCII character string one byte at a time, consuming the special command
 * bytes and their parameters, and producing the text that gets generated in their place (if any).
 *
 * TODO: It would be nice if this function took some sort of container reference as a parameter,
 * and used it to return a list of the encountered command bytes that took some invisible actions
 * (granting knowledge of people/places/things/facts, modifying the setup table, changing the alien's
 * temperament, etc.)
 */
QString GameText::readString(const char* data, int maxlen)
{
  QString clean;
  int pos = 0;

  while ((data[pos] != 0) && (pos < maxlen))
  {
    // all normally printable characters are passed through to the output directly
    if (data[pos] >= 0x20)
    {
      clean += QString(data[pos]).toHtmlEscaped();
      pos++;
    }
    else
    {
      const GTxtCmd cmd = static_cast<GTxtCmd>(data[pos]);
      pos++;

      if (cmd == GTxtCmd_InsertPlayerName)
      {
        clean.append(QString("<Player's name>").toHtmlEscaped());
      }
      else if (cmd == GTxtCmd_InsertPlayerShip)
      {
        clean.append(QString("<Player's ship>").toHtmlEscaped());
      }
      else if (cmd == GTxtCmd_GAMESTR)
      {
        clean.append(QString("<GAMESTR>").toHtmlEscaped());
      }
      else if (cmd == GTxtCmd_METAMOVE)
      {
        clean.append(QString("<METAMOVE>").toHtmlEscaped());
      }
      else if (cmd == GTxtCmd_MetaText)
      {
        uint16_t metaTabIdx = (static_cast<uint8_t>(data[pos]) + (0x100 * static_cast<uint8_t>(data[pos+1]))) - 1;
        const QString metaStr = getMetaString(metaTabIdx);
        clean.append(metaStr);
      }
      else if (cmd == GTxtCmd_InsertCurrentLocation)
      {
        clean.append(QString("<current location>").toHtmlEscaped());
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
