#ifndef STAMPIMAGES_H
#define STAMPIMAGES_H

#include <stdint.h>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QRgb>
#include <QByteArray>
#include "datlibrary.h"
#include "palette.h"

#define STAMP_EXTENSION ".stp"
#define ROLL_EXTENSION  ".rol"

/**
 * Reads and decodes stamp (.stp) and stamp roll (.rol) image files (other
 * than those for the inventory objects, which are separately handled by
 * the InvObject class).
 */
class StampImages
{
public:
  StampImages(DatLibrary& lib, Palette& pal);
  QMap<DatFileType,QStringList> getAllStampsList();
  bool getStamp(DatFileType dat, QString filename, QList<QImage>& images);

private:
  DatLibrary* m_lib;
  Palette* m_pal;
  static const QList<DatFileType> s_datsToSearch;
  static const QMap<QString,QString> s_stpToPal;

  int getNumOfStampsInRoll(const QByteArray& roll);
  QList<int> getStpStartOffsetListFromRoll(const QByteArray& roll);
  QList<QByteArray> getStpDataFromRoll(const QByteArray& roll);
};

#endif // STAMPIMAGES_H
