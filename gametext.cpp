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
 */
QString GameText::readString(const char* data, QVector<QPair<GTxtCmd,int> >& commands, bool showEmbeddedCommands, int maxlen)
{
  QString clean;
  int pos = 0;
  commands.clear();

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
      else if ((cmd == GTxtCmd_AddItem) ||
               ((cmd >= GTxtCmd_ChangeAlienTemperament) && (cmd <= GTxtCmd_ModifyMissionTable)))
      {
        // the command is one of several that modify game state, so let's capture it along
        // with its parameters to pass back to the caller in a list

        int param = 0;
        if (showEmbeddedCommands && g_gameTextParamCount.contains(cmd))
        {
          // the GrantKnowledgePlace command is special because it can use either one or two
          // parameter bytes, determined during parsing by the content of the first byte
          if (cmd == GTxtCmd_GrantKnowledgePlace)
          {
            param = data[pos];

            // if bit 7 is set in the first parameter byte to GrantKnowledgePlace, we need
            // to consume one additional byte and combine them to form a 16-bit place ID
            if (param & 0x80)
            {
              // advance the input pointer once more since we just determined that an
              // additional parameter byte is required
              pos++;
              if (pos < maxlen)
              {
                param &= 0x7F;
                param |= (static_cast<uint16_t>(data[pos]) << 7);
              }
            }
          }
          else if (g_gameTextParamCount[cmd] == 1)
          {
            param = static_cast<uint8_t>(data[pos]);
          }
          else if (g_gameTextParamCount[cmd] == 2)
          {
            // interpret bytes as little-endian 16-bit word
            param = (static_cast<uint8_t>(data[pos]) + (0x100 * static_cast<uint8_t>(data[pos+1])));
          }

          commands.append(QPair<GTxtCmd,int>(cmd, param));
          clean += QString("<font color=\"red\">[%1]</font>").arg(commands.count());
        }
      }
      else
      {
        // all other command values (bytes < 0x20) are unused, and cause the game's text engine to
        // insert the string "<HUH?>"
        clean.append(QString("<HUH?>").toHtmlEscaped());
      }

      // advance the pointer by the number of parameter bytes used by this command
      pos += g_gameTextParamCount[cmd];
    }
  }

  return clean;
}
