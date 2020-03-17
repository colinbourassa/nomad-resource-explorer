#ifndef CONVERSATIONTEXT_H
#define CONVERSATIONTEXT_H

#include <QStringList>
#include <QByteArray>
#include <QList>
#include "datlibrary.h"
#include "aliens.h"

#define TLKT_RECORDSIZE 0x0A
#define TLKN_RECORDSIZE 0x06
#define TLKX_RECORDSIZE 0x04

enum ConvTopic
{
  ConvTopic_GreetingFirstMeeting,
  ConvTopic_GreetingPreviouslyMet,
  ConvTopic_Person,
  ConvTopic_Location,
  ConvTopic_Object,
  ConvTopic_Race
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
  ConversationText(DatLibrary& lib, Aliens& aliens);
  QStringList getConversationText(int alienId, ConvTopic topic, int thingId);

private:
  DatLibrary* m_lib;
  Aliens* m_aliens;

  bool getTLKTData(ConvTableType tableType, int id, QByteArray& data);
  bool getTLKNData(ConvTableType tableType, int id, QByteArray& data);
  bool getTLKXData(ConvTableType tableType, int id, QByteArray& indexData, QByteArray& strData);

  //! Searches the provided TLKTR or TLKTC file to get a list of TLKN indices that match the provided criteria
  QList<int> getTLKNIndexes(ConvTopic topic, int thingId, const QByteArray& tlktData);

  const QString getTLKNCFilename(int id);
  const QString getTLKNRFilename(int id);
  const QString getTLKTCFilename(int id);
  const QString getTLKTRFilename(int id);
  const QString getTLKXCIndexFilename(int id);
  const QString getTLKXRIndexFilename(int id);
  const QString getTLKXCStringsFilename(int id);
  const QString getTLKXRStringsFilename(int id);

  //! Reads data from the provided TLKNC or TLKNR data to provide an index into a TLKXC/TLKXR file
  QList<int> getTLKXIndexes(const QList<int>& tlknIndex, const QByteArray& tlknData);

  QStringList getTLKXStrings(const QList<int>& tlkxIndexes, const QByteArray& tlkxIndexData, const QByteArray& tlkxStrData);
};

#endif // CONVERSATIONTEXT_H
