#ifndef IMAGECONVERTER_H
#define IMAGECONVERTER_H

#include <QPixmap>
#include <QByteArray>
#include <QVector>
#include <QRgb>
#include <QPoint>
#include <stdint.h>

class ImageConverter
{
public:
  ImageConverter();
  static QPixmap stpToPixmap(QByteArray& stpData, QVector<QRgb> palette, bool& status);
  static QPixmap delToPixmap(QByteArray& delData, QVector<QRgb> palette, bool& status);
  static QPixmap lbmToPixmap(QByteArray& rawData, QVector<QRgb> palette, bool& status);
  static QPixmap plnToPixmap(QByteArray& plnData, QVector<QRgb> palette, bool& status);

private:
  static QPoint getPixelLocation(int imgWidth, int pixelnum);
  static const int8_t s_deltas[];
};

#endif // IMAGECONVERTER_H
