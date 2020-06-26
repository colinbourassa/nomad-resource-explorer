#ifndef SHIPMODELDATA_H
#define SHIPMODELDATA_H

#include <qopengl.h>
#include <QVector>
#include <QVector3D>
#include <QColor>

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

private:
  void quad(QVector3D a, QVector3D b, QVector3D c, QVector3D d, QColor color);
  void tri(QVector3D a, QVector3D b, QVector3D c, QColor color);
  void add(const QVector3D& v, const QVector3D& n, const QColor c);

  QVector<GLfloat> m_data;
  int m_count = 0;
};

#endif // SHIPMODELDATA_H
