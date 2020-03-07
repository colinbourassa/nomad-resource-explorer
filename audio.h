#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>
#include <QByteArray>
#include <QString>
#include <QStringList>
#include "datlibrary.h"

#define NNV_INDEX_SIZE 8

class Audio
{
public:
  Audio(DatLibrary& lib);
  bool readSound(DatFileType dat, QString nnvContainer, int soundId, QByteArray& pcmData);
  int getNumberOfSoundsInNNV(DatFileType dat, QString nnvContainer);
  QMap<DatFileType, QStringList> getAllSoundList();

private:
  static const int8_t s_deltaTable[];
  DatLibrary* m_lib;

  static int getStartLocation(QByteArray& nnvData, int soundId);
  static int getSoundDataLength(QByteArray& nnvData, int soundId);
  static void decode(uint8_t* encoded, int length, QByteArray& decoded);
};

#endif // AUDIO_H
