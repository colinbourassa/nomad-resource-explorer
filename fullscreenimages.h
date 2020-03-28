#ifndef FULLSCREENIMAGES_H
#define FULLSCREENIMAGES_H

#include <QMap>
#include <QStringList>
#include <QString>
#include <QImage>
#include "datlibrary.h"
#include "palette.h"

class FullscreenImages
{
public:
  FullscreenImages(DatLibrary& lib, Palette& pal);
  QMap<DatFileType, QStringList> getAllLbmList();
  bool getImage(DatFileType dat, QString lbmFilename, QImage& img);

private:
  DatLibrary* m_lib;
  Palette* m_pal;
  static const QMap<QString,QString> s_lbmToPal;
  static const QString s_lbmExtension;
  static const QString s_palExtension;
};

#endif // FULLSCREENIMAGES_H
