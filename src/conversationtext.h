#ifndef CONVERSATIONTEXT_H
#define CONVERSATIONTEXT_H

#include <QStringList>
#include <QByteArray>
#include <QList>
#include "datlibrary.h"
#include "gametext.h"
#include "aliens.h"

#define TLKT_RECORDSIZE 0x0A
#define TLKN_RECORDSIZE 0x06
#define TLKX_RECORDSIZE 0x04

#define TLKN_CMD_ASKABOUT     0x00
#define TLKN_CMD_SEESOBJ      0x02
#define TLKN_CMD_TRADEFOROBJ  0x03
#define TLKN_CMD_DISPOBJECT   0x04
#define TLKN_CMD_GREETFIRST   0x05
#define TLKN_CMD_GREETNEXT    0x06
#define TLKN_CMD_ASKABOUTRACE 0x07
#define TLKN_CMD_GIVEOBJECT   0x08

enum ConvTopicCategory
{
  ConvTopicCategory_GreetingInitial,
  ConvTopicCategory_GreetingSubsequent,
  ConvTopicCategory_AskAboutPerson,
  ConvTopicCategory_AskAboutLocation,
  ConvTopicCategory_AskAboutObject,
  ConvTopicCategory_AskAboutRace,
  ConvTopicCategory_DisplayObject,
  ConvTopicCategory_GiveObject,
  ConvTopicCategory_GiveFact,
  ConvTopicCategory_SeesObject,
};

enum ConvTableType
{
  ConvTableType_Individual,
  ConvTableType_Race,
  ConvTableType_Invalid
};

class ConversationText
{
public:
  ConversationText(DatLibrary& lib, Aliens& aliens, GameText& gtext);
  QString getConversationText(int alienId, ConvTopicCategory topic, int thingId, QVector<QPair<GTxtCmd,int> >& commands);
  bool doesInterestingDialogExist(int alienId, ConvTopicCategory category, int thingId);

private:
  DatLibrary* m_lib;
  Aliens* m_aliens;
  GameText* m_gtext;

  bool getTLKTData(ConvTableType tableType, int id, QByteArray& data);
  bool getTLKNData(ConvTableType tableType, int id, QByteArray& data);
  bool getTLKXData(ConvTableType tableType, int id, QByteArray& indexData, QByteArray& strData);

  //! Searches the provided TLKTR or TLKTC file to get a list of TLKN indices that match the provided criteria
  int getTLKNIndex(ConvTopicCategory topic, int thingId, const QByteArray& tlktData);

  const QString getTLKNCFilename(int id);
  const QString getTLKNRFilename(int id);
  const QString getTLKTCFilename(int id);
  const QString getTLKTRFilename(int id);
  const QString getTLKXCIndexFilename(int id);
  const QString getTLKXRIndexFilename(int id);
  const QString getTLKXCStringsFilename(int id);
  const QString getTLKXRStringsFilename(int id);

  //! Reads data from the provided TLKNC or TLKNR data to provide an index into a TLKXC/TLKXR file
  int getTLKXIndex(int tlknIndex, const QByteArray& tlknData);

  QString getTLKXString(int tlkxIndex, const QByteArray& tlkxIndexData, const QByteArray& tlkxStrData, QVector<QPair<GTxtCmd,int> >& commands);
};

#endif // CONVERSATIONTEXT_H
