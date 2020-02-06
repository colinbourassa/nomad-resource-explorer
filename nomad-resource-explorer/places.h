#ifndef PLACES_H
#define PLACES_H

#include <QPixmap>
#include <QMap>
#include <stdint.h>
#include "datlibrary.h"
#include "palette.h"

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
  Places(DatLibrary& lib, Palette& pal);

  void clear();
  QMap<int,Place> getPlaceList();
  QPixmap getPlaceSurfaceImage(int id, bool& status);

private:
  DatLibrary* m_lib;
  Palette* m_pal;
  QMap<int,Place> m_placeList;

  static const uint8_t s_planetTextureMapping[622];

  void populatePlaceList();
};

#endif // PLACES_H
