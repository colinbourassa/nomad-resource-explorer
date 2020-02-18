#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>
#include <QByteArray>
#include <QString>
#include "datlibrary.h"

#define NNV_INDEX_SIZE 8

class Audio
{
public:
  Audio(DatLibrary& lib);
  bool readSound(DatFileType dat, QString nnvContainer, int soundId, QByteArray& pcmData);
  bool getSoundList(DatFileType dat, QString nnvContainer, QVector<unsigned int>& sounds);

private:
  static const int8_t s_deltaTable[];
  DatLibrary* m_lib;

  void decode(uint8_t* encoded, unsigned int length, QByteArray& decoded);
};

#endif // AUDIO_H
