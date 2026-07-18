#ifndef CONVERSATIONTEXT_H
#define CONVERSATIONTEXT_H

#include <QStringList>
#include <QByteArray>
#include <QList>
#include <QMetaType>
#include <QHash>
#include <QPair>
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

//! Identifies the conversation line (alien-or-race, topic, thing) that produced a given
//! embedded game-text command; used by MainWindow to build a reverse index (e.g. "what
//! conversations grant this fact?").
struct ConversationRef
{
  bool isRace;
  int alienOrRaceId;
  ConvTopicCategory topic;
  int thingId;

  bool operator==(const ConversationRef& other) const
  {
    return (isRace == other.isRace) && (alienOrRaceId == other.alienOrRaceId) &&
           (topic == other.topic) && (thingId == other.thingId);
  }
};

class ConversationText
{
public:
  ConversationText(DatLibrary& lib, Aliens& aliens, GameText& gtext);

  //! Clears the TLK* file-content caches; must be called when a new game data directory is
  //! opened, or stale data from the previous directory would be returned.
  void clear();

  //! If commandsOnly is true, skips building the display string -- used by callers (e.g. the
  //! conversation reverse index) that only need the resulting (command, parameter) list.
  QString getConversationText(int alienId, ConvTopicCategory topic, int thingId, QVector<QPair<GTxtCmd,int> >& commands,
                              ConvTableType* resolvedTableType = nullptr, int* resolvedId = nullptr,
                              bool commandsOnly = false);
  bool doesInterestingDialogExist(int alienId, ConvTopicCategory category, int thingId);

private:
  DatLibrary* m_lib;
  Aliens* m_aliens;
  GameText* m_gtext;

  // Cache of TLK* file contents by (tableType, id), keyed on the cheap int pair rather than the
  // formatted filename string -- avoids reformatting/rehashing the same filename on every one of
  // the many topic/thing combinations queried for a given alien or race during index construction.
  QHash<QPair<int,int>, QByteArray> m_tlktCache;
  QHash<QPair<int,int>, QByteArray> m_tlknCache;
  QHash<QPair<int,int>, QPair<QByteArray,QByteArray> > m_tlkxCache;

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

  QString getTLKXString(int tlkxIndex, const QByteArray& tlkxIndexData, const QByteArray& tlkxStrData, QVector<QPair<GTxtCmd,int> >& commands,
                        bool commandsOnly = false);
};

Q_DECLARE_METATYPE(ConversationRef)

#endif // CONVERSATIONTEXT_H
