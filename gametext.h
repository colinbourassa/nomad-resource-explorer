#ifndef GAMETEXT_H
#define GAMETEXT_H

#include <QString>
#include <QMap>
#include <QByteArray>
#include "datlibrary.h"

#define METATAB_RECORDSIZE_BYTES   4
#define METATAB_TYPE_SYNONYM       0
#define METATAB_TYPE_TRANSLATION   1
#define METATAB_TYPE_LOSTENGATEWAY 2
#define METATEXT_RECORDSIZE_BYTES  2

/**
 * Enumeration of all the embedded text command bytes supported by the game.
 * This enumeration (and the prefix on its values) begins with "GTxt" as an
 * abbreviation for "game text", but the game's originally source code may
 * have referred to it as "rtext".
 */
enum GTxtCmd
{
  GTxtCmd_InsertPlayerName         = 0x01,
  GTxtCmd_InsertPlayerShip         = 0x02,
  GTxtCmd_InsertCurrentLocation    = 0x03,
  GTxtCmd_GAMESTR                  = 0x04,
  GTxtCmd_MetaText                 = 0x05,
  GTxtCmd_AddItem                  = 0x06,
  GTxtCmd_METAMOVE                 = 0x07,
  GTxtCmd_ChangeAlienTemperament   = 0x08,
  GTxtCmd_GrantKnowledgeFact       = 0x09,
  GTxtCmd_GrantKnowledgePlace      = 0x0A,
  GTxtCmd_AStateTableModifyA       = 0x0B,
  GTxtCmd_GrantKnowledgeObject     = 0x0C,
  GTxtCmd_AStateTableModifyB       = 0x0D,
  GTxtCmd_CopyEncountRelateTable   = 0x0E,
  GTxtCmd_GrantKnowledgeRace       = 0x0F,
  GTxtCmd_ModifyEncountRelate      = 0x10,
  GTxtCmd_StoreShipIDInSetupTab    = 0x11,
  GTxtCmd_ModifySetupTable         = 0x12,
  GTxtCmd_SetAlienConvBehaviorFlag = 0x13,
  GTxtCmd_SetAlienAttrMax          = 0x14,
  GTxtCmd_SetAlienAttrMin          = 0x15,
  GTxtCmd_ModifyMissionTable       = 0x16
};

/**
 * Map associating game text commands with the number of bytes that each command
 * takes as parameters. All of these byte counts are fixed values with the exception
 * of the command GrantKnowledgePlace: this command can take either one or two bytes,
 * depending on whether the place ID requires the extra width. (The place ID is unlike
 * other IDs in the game in that the maximum ID is larger than a full 8-bit count.)
 */
static const QMap<GTxtCmd,int> g_gameTextParamCount =
{
  { GTxtCmd_InsertPlayerName,         0 },
  { GTxtCmd_InsertPlayerShip,         0 },
  { GTxtCmd_InsertCurrentLocation,    0 },
  { GTxtCmd_GAMESTR,                  0 },
  { GTxtCmd_MetaText,                 2 },
  { GTxtCmd_AddItem,                  1 },
  { GTxtCmd_METAMOVE,                 0 },
  { GTxtCmd_ChangeAlienTemperament,   1 },
  { GTxtCmd_GrantKnowledgeFact,       1 },
  { GTxtCmd_GrantKnowledgePlace,      1 }, // can possibly require a second argument byte as well
  { GTxtCmd_AStateTableModifyA,       1 },
  { GTxtCmd_GrantKnowledgeObject,     1 },
  { GTxtCmd_AStateTableModifyB,       0 },
  { GTxtCmd_CopyEncountRelateTable,   0 },
  { GTxtCmd_GrantKnowledgeRace,       1 },
  { GTxtCmd_ModifyEncountRelate,      2 },
  { GTxtCmd_StoreShipIDInSetupTab,    1 },
  { GTxtCmd_ModifySetupTable,         2 },
  { GTxtCmd_SetAlienConvBehaviorFlag, 0 },
  { GTxtCmd_SetAlienAttrMax,          0 },
  { GTxtCmd_SetAlienAttrMin,          0 },
  { GTxtCmd_ModifyMissionTable,       1 }
};

/**
 * Reads mission, conversation, and object text from a data file. This text
 * may have embedded command sequences that control the game environment, and
 * this class will process them appropriately.
 */
class GameText
{
public:
  GameText(DatLibrary& lib);
  void clear();

  //! Produces a regular string from string data with embedded commands
  QString readString(const char* data, int maxlen = 0x1000);

private:
  DatLibrary* m_lib;
  QByteArray m_metaTab;
  QByteArray m_metaTextTab;

  QString getMetaString(int metaTabIndex);
};

#endif // GAMETEXT_H
