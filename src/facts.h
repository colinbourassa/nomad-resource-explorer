#pragma once
#include <QString>
#include <QMap>
#include "enums.h"
#include "dattable.h"
#include "datlibrary.h"

struct Fact
{
  int id;
  QString text;
  QMap<AlienRace,int> receptivity;
};

typedef struct __attribute__((packed)) FactTableEntry
{
  uint16_t textOffset;
  uint8_t receptivity[static_cast<int>(AlienRace::NumRaces)];
  uint8_t bitfield;
  uint8_t unused;
} FactTableEntry;

static_assert(sizeof(FactTableEntry) == 16, "FactTableEntry packing does not match game data");

class Facts : public DatTable<FactTableEntry>
{
public:
  Facts(DatLibrary& lib);
  QMap<int,Fact> getList();
  Fact getFact(int id) const;
  QMap<AlienRace,int> getReceptivity(int id);
  void clear();

protected:
  bool populateList();

private:
  QMap<int,Fact> m_factList;
};

