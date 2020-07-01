#ifndef SHIPMODELDATA_H
#define SHIPMODELDATA_H

#include <qopengl.h>
#include <QVector>
#include <QMap>
#include <QVector3D>
#include <QColor>
#include <QString>

#define FLOATS_PER_VERTEX 9

class ShipModelData
{
public:
  ShipModelData();

  const GLfloat* constData() const
  {
    return m_data.constData();
  }

  int count() const
  {
    return m_count;
  }

  int vertexCount() const
  {
    return m_count / FLOATS_PER_VERTEX;
  }

  bool loadData(const QByteArray& bin, QString& modelInfo);
  void clear();

private:
  static int getTotalVertexCount(const QMap<int,QVector<int> >& polygons);

  void triangulatePolygon(const QVector<QVector3D>& vertices, const QColor);
  void addTriangle(QVector3D a, QVector3D b, QVector3D c, QColor color);
  void add(const QVector3D& v, const QVector3D& n, const QColor c);

  QVector<GLfloat> m_data;
  int m_count = 0;
  static QMap<int,QColor> s_modelColors;
};

#endif // SHIPMODELDATA_H
