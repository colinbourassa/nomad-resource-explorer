#ifndef FACTS_H
#define FACTS_H

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
  uint8_t receptivity[AlienRace_NumRaces];
  uint8_t bitfield;
  uint8_t unused;
} FactTableEntry;

class Facts : public DatTable<FactTableEntry>
{
public:
  Facts(DatLibrary& lib);
  virtual ~Facts();
  QMap<int,Fact> getList();
  Fact getFact(int id) const;
  QMap<AlienRace,int> getReceptivity(int id);
  void clear();

protected:
  bool populateList();

private:
  QMap<int,Fact> m_factList;
};

#endif // FACTS_H
