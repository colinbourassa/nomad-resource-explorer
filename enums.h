#ifndef ENUMS_H
#define ENUMS_H

#include <QMap>
#include <QString>

enum AlienRace
{
  AlienRace_Altec,
  AlienRace_Arden,
  AlienRace_Bellicosian,
  AlienRace_Chanticleer,
  AlienRace_Human,
  AlienRace_Kenelm,
  AlienRace_Korok,
  AlienRace_Musin,
  AlienRace_Pahrump,
  AlienRace_Phelonese,
  AlienRace_Shaasa,
  AlienRace_Ursor,
  AlienRace_NumRaces,
  AlienRace_Invalid
};

static const QMap<AlienRace,QString> s_raceNames =
{
  {AlienRace_Altec, "Altec Hocker"},
  {AlienRace_Arden, "Arden"},
  {AlienRace_Bellicosian, "Bellicosian"},
  {AlienRace_Chanticleer, "Chanticleer"},
  {AlienRace_Human, "Human"},
  {AlienRace_Kenelm, "Kenelm"},
  {AlienRace_Korok, "Korok"},
  {AlienRace_Musin, "Musin"},
  {AlienRace_Pahrump, "Pahrump"},
  {AlienRace_Phelonese, "Phelonese"},
  {AlienRace_Phelonese, "Phelonese"},
  {AlienRace_Shaasa, "Shaasa"},
  {AlienRace_Ursor, "Ursor"}
};

enum PlanetResourceType
{
  PlanetResourceType_Animal,
  PlanetResourceType_ArchaeologicalArtifact,
  PlanetResourceType_EspionageItem,
  PlanetResourceType_Food,
  PlanetResourceType_Gas,
  PlanetResourceType_Mineral,
  PlanetResourceType_NumTypes
};

#endif // ENUMS_H

