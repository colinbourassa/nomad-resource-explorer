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
  ImageConverter();
  static bool stpToImage(QByteArray& stpData, QVector<QRgb> palette, QImage& image);
  static bool delToImage(const QByteArray& delData, QVector<QRgb> palette, QImage& image);
  static bool lbmToImage(const QByteArray& rawData, QVector<QRgb> palette, QImage& image);
  static QPixmap plnToPixmap(QByteArray& plnData, QVector<QRgb> palette, bool& status);

private:
  static QPoint getPixelLocation(int imgWidth, int pixelnum);
  static const int8_t s_deltas[];
};

#endif // IMAGECONVERTER_H
