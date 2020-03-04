#ifndef PLACECLASSES_H
#define PLACECLASSES_H

#include <QString>
#include <QVector>
#include <stdint.h>
#include "datlibrary.h"

struct PlanetClass
{
  QString name;
  int temperature;
  QString temperatureRange;
  QMap<int,int> foods;
  QMap<int,int> ores;
  QMap<int,int> ancientArtifacts;
  QMap<int,int> gasses;
  QMap<int,int> animals;
  QMap<int,int> intelligenceArtifacts;
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

class PlaceClasses
{
public:
  PlaceClasses(DatLibrary& lib);
  const QMap<int,PlanetClass>* planetClassDataList();
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
