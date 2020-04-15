#ifndef PLACECLASSES_H
#define PLACECLASSES_H

#include <QString>
#include <QVector>
#include <stdint.h>
#include "datlibrary.h"
#include "enums.h"

struct PlanetClass
{
  QString name;
  int temperature;
  QString temperatureRange;
  QMap<PlanetResourceType, QMap<int,int> > resources;
};

struct StarClass
{
  QString name;
};

// TODO: better multi-compiler support for struct packing?
typedef struct __attribute__((packed)) PClassTableEntry
{
  uint16_t nameOffset;
  uint8_t  unknown_a[4];
  int16_t  temperature;
  uint8_t  unknown_b;
  uint8_t  inhabited;
  uint8_t  classType;
  uint8_t  foods[3];
  uint8_t  foodsAgriculture[3];
  uint8_t  ores[3];
  uint8_t  oresConcentration[3];
  uint8_t  ancientArtifacts[3];
  uint8_t  ancientArtifactsConcentration[3];
  uint8_t  gasses[3];
  uint8_t  gassesConcentration[3];
  uint8_t  animals[3];
  uint8_t  animalsConcentration[3];
  uint8_t  intelligenceItems[3];
  uint8_t  intelligenceItemsConcentration[3];
  uint8_t  unknown_c;
} PClassTableEntry;

typedef struct __attribute__((packed)) StClassTableEntry
{
  uint16_t nameOffset;
  uint8_t unknown[4];
} StClassTableEntry;

/**
 * Reads and parses data table with place class information (for stars and planets).
 * This class stores information about planet names, resources, temperature, and
 * other factors.
 */
class PlaceClasses
{
public:
  PlaceClasses(DatLibrary& lib);
  bool pclassData(int id, PlanetClass& pclass);
  void clear();
  QString getStarClassName(int id);

private:
  DatLibrary* m_lib;
  QMap<int,PlanetClass> m_planetClassList;
  QMap<int,StarClass> m_starClassList;
  static const QMap<int,QString> s_tempRanges;

  void populatePlaceClassList();
};

#endif // PLACECLASSES_H
