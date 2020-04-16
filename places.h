#ifndef PLACES_H
#define PLACES_H

#include <QImage>
#include <QMap>
#include <stdint.h>
#include "enums.h"
#include "datlibrary.h"
#include "dattable.h"
#include "palette.h"
#include "placeclasses.h"

struct Place
{
  int id;
  QString name;
  bool isPlanet;
  int representativeId;
  int parentStarId;
  int classId;
  AlienRace race;
};

typedef struct __attribute__((packed)) PlaceTableEntry
{
  uint16_t nameOffset;
  uint16_t flags;
  uint8_t pclass;
  uint8_t isPlanet;
  uint8_t parentStarId;
  uint8_t unknown_a;
  uint8_t planetRepId;
  uint8_t race;
  uint8_t unknown_b[6];
} PlaceTableEntry;

/**
 * Reads data about places (stars and planets) from data files, and also provides images of the
 * surface textures used when planets are rendered in game.
 */
class Places : public DatTable<PlaceTableEntry>
{
public:
  Places(DatLibrary& lib, Palette& pal, PlaceClasses& pclasses);
  virtual ~Places();

  void clear();
  QMap<int,Place> getPlaceList();
  bool getPlace(int id, Place& p);
  QImage getPlaceSurfaceImage(int id, bool& status);
  QString getName(int id);

protected:
  bool populateList();

private:
  Palette* m_pal;
  PlaceClasses* m_placeClasses;
  QMap<int,Place> m_placeList;

  static const uint8_t s_planetTextureMapping[622];
};

#endif // PLACES_H
