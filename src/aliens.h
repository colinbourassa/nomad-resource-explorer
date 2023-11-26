#pragma once
#include <QByteArray>
#include <QVector>
#include <QString>
#include <QImage>
#include <QMap>
#include "enums.h"
#include "palette.h"
#include "dattable.h"

#define ANM_RECORD_SIZE_BYTES 16
#define ANM_FIRST_RECORD_OFFSET 0x1A

struct Alien
{
  int id;
  QString name;
  AlienRace race;
};

typedef struct __attribute__((packed)) AlienTableEntry
{
  uint16_t nameOffset;
  uint8_t race;
  uint8_t unknown[5];
} AlienTableEntry;

static_assert(sizeof(AlienTableEntry) == 8, "AlienTableEntry packing does not match game data");

class Aliens : DatTable<AlienTableEntry>
{
public:
  Aliens(DatLibrary& lib, Palette& pal);
  void clear();
  QMap<int,Alien> getList();
  QString getName(int id);
  AlienRace getRace(int id);
  bool getAlien(int id, Alien& alien);
  bool getAnimationFrames(int id, QMap<int,QImage>& frames, QString& paletteFile);

protected:
  bool populateList();

private:
  Palette* m_pal;
  static const QVector<QString> s_animationMap;
  QMap<int,Alien> m_alienList;

  QMap< int, QVector<int> > getListOfFrames(const QByteArray& anmData) const;
  bool buildFrame(QVector<int> delIdList, QString delFilenamePrefix, const QVector<QRgb> pal, QImage& frame) const;
};

