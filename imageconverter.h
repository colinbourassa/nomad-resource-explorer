#ifndef IMAGECONVERTER_H
#define IMAGECONVERTER_H

#include <QPixmap>
#include <QImage>
#include <QByteArray>
#include <QVector>
#include <QRgb>
#include <QPoint>
#include <stdint.h>

class ImageConverter
{
public:
  static bool stpToImage(const QByteArray& stpData, QVector<QRgb> palette, QImage& image);
  static bool delToImage(const QByteArray& delData, QVector<QRgb> palette, QImage& image);
  static bool lbmToImage(const QByteArray& rawData, QVector<QRgb> palette, QImage& image);
  static bool plnToPixmap(const QByteArray& plnData, QVector<QRgb> palette, QImage& image);

private:
  ImageConverter();
  static QPoint getPixelLocation(int imgWidth, int pixelnum);
  static const int8_t s_deltas[];
};

#endif // IMAGECONVERTER_H
