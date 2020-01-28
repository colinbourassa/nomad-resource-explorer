#ifndef PLACES_H
#define PLACES_H

#include <QPixmap>
#include <QMap>
#include "datlibrary.h"

struct Place
{
  int id;
  QString name;
  int representativeId;
};

typedef struct __attribute__((packed)) PlaceTableEntry
{
  uint16_t nameOffset;
  uint16_t flags;
  uint8_t pclass;
  uint8_t travelDest;
  uint8_t parentStarId;
  uint8_t unknown_a;
  uint8_t planetRepId;
  uint8_t race;
  uint8_t unknown_b[6];
} PlaceTableEntry;

class Places
{
public:
  Places(DatLibrary& lib);

  void clear();
  QMap<int,Place> getPlaceList();
  QPixmap getPlaceSurfaceImage(int id);

private:
  DatLibrary* m_lib;
  QMap<int,Place> m_placeList;


  void populatePlaceList();
};

#endif // PLACES_H
