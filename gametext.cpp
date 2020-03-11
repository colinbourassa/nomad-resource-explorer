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

      // because the number of places in the game can require a two-byte identifier,
      // we need to check whether an additional byte must be consumed as a parameter to this command
      if (cmd == GTxtCmd_GrantKnowledgePlace)
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
