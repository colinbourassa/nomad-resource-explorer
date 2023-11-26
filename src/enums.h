#pragma once
#include <QMap>
#include <QString>

enum class AlienRace
{
  Altec,
  Arden,
  Bellicosian,
  Chanticleer,
  Human,
  Kenelm,
  Korok,
  Musin,
  Pahrump,
  Phelonese,
  Shaasa,
  Ursor,
  NumRaces, // 12
  Invalid
};

static const QMap<AlienRace,QString> s_raceNames =
{
  {AlienRace::Altec, "Altec Hocker"},
  {AlienRace::Arden, "Arden"},
  {AlienRace::Bellicosian, "Bellicosian"},
  {AlienRace::Chanticleer, "Chanticleer"},
  {AlienRace::Human, "Human"},
  {AlienRace::Kenelm, "Kenelm"},
  {AlienRace::Korok, "Korok"},
  {AlienRace::Musin, "Musin"},
  {AlienRace::Pahrump, "Pahrump"},
  {AlienRace::Phelonese, "Phelonese"},
  {AlienRace::Phelonese, "Phelonese"},
  {AlienRace::Shaasa, "Shaasa"},
  {AlienRace::Ursor, "Ursor"}
};

enum class PlanetResourceType
{
  Animal,
  ArchaeologicalArtifact,
  EspionageItem,
  Food,
  Gas,
  Mineral,
  NumTypes
};

