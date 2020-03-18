#include "gametext.h"

GameText::GameText()
{

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
        uint16_t metaTabIdx = static_cast<uint8_t>(data[pos]) + (0x100 * static_cast<uint8_t>(data[pos+1]));
        clean.append(QString("<metatext %1>").arg(static_cast<uint8_t>(metaTabIdx)));
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
