#ifndef ALIENS_H
#define ALIENS_H

#include <QVector>
#include <QString>
#include <QPixmap>
#include <QMap>
#include "enums.h"
#include "palette.h"
#include "datlibrary.h"

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

class Aliens
{
public:
  Aliens(DatLibrary& lib, Palette& pal);
  void clear();
  QMap<int,Alien> getAlienList();
  bool getAlien(int id, Alien& alien);
  bool getPortrait(int id, QPixmap& status);

private:
  DatLibrary* m_lib;
  Palette* m_pal;
  static const QVector<QString> s_animationMap;
  QMap<int,Alien> m_alienList;

  bool populateAlienList();
};

#endif // ALIENS_H
