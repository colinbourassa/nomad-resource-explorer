#ifndef DATTABLE_H
#define DATTABLE_H

#include <QByteArray>
#include <QMap>
#include <QString>
#include "datlibrary.h"

/**
 * Template for classes that handle the game's data table files. The base functionality
 * in this template provides the ability to sequence through the entries in such a
 * data table.
 */
template <typename StructType>
class DatTable
{
public:
  void clear()
  {
    m_rawdata.clear();
  }

protected:
  DatLibrary* m_lib;

  DatTable(DatLibrary& lib)
  {
    m_lib = &lib;
  }

  ~DatTable()
  {
  }

  StructType* getEntry(int index)
  {
    StructType* ptr = nullptr;

    if (((index * s_entrySize) + s_entrySize) <= m_rawdata.size())
    {
      ptr = reinterpret_cast<StructType*>(&m_rawdata.data()[index * s_entrySize]);
    }

    return ptr;
  }

  bool openFile(DatFileType dat, QString filename)
  {
    return m_lib->getFileByName(dat, filename, m_rawdata);
  }

  QString getGameText(int offset) const
  {
    return m_lib->getGameText(offset);
  }

  bool dataAvailable() const
  {
    return (!m_rawdata.isEmpty());
  }

  virtual bool populateList() = 0;

private:
  QByteArray m_rawdata;
  static const int s_entrySize = sizeof(StructType);
};

#endif // DATTABLE_H
