#ifndef PALETTE_H
#define PALETTE_H

#include <QVector>
#include <QRgb>
#include <stdint.h>
#include "datlibrary.h"

class Palette
{
public:
  Palette(DatLibrary& lib);
  void clear();

  void defaultVgaPalette(QVector<QRgb>& palette);
  bool gamePalette(QVector<QRgb>& palette);
  bool paletteByName(DatFileType datContainer, QString palFilename, QVector<QRgb>& palette, bool prefillWithDefaultVga = true) const;
  QMap<DatFileType,QStringList> getAllPaletteList();

private:
  static const QVector<QRgb> s_defaultVgaPalette;
  static const QString s_gamePalFilename;
  DatLibrary* m_lib;
  QVector<QRgb> m_gamePal;

  bool loadPalData(DatFileType datContainer, QString palFileName, QVector<QRgb>& palette, bool prefillWithDefaultVga = true) const;
};

#endif
