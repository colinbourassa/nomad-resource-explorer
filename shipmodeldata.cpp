#include "shipmodeldata.h"

ShipModelData::ShipModelData()
{
  m_data.resize(250 * 9);

  const QVector3D v0( .200f,  .050f,  .000f);
  const QVector3D v1( .100f,  .100f,  .000f);
  const QVector3D v2( .100f,  .050f, -.050f);
  const QVector3D v3(-.040f,  .050f,  .000f);
  const QVector3D v4( .100f,  .050f,  .050f);
  const QVector3D v5( .100f, -.050f,  .050f);
  const QVector3D v6(-.040f, -.050f,  .000f);
  const QVector3D v7( .200f, -.050f,  .000f);
  const QVector3D v8( .100f, -.050f, -.050f);
  const QVector3D v9( .100f, -.100f,  .000f);

  tri (v0, v1, v2,     Qt::gray);
  quad(v3, v4, v5, v6, Qt::red);
  quad(v7, v0, v2, v8, Qt::red);
  tri (v6, v5, v9,     Qt::gray);
  tri (v9, v7, v8,     Qt::gray);
  tri (v4, v3, v1,     Qt::gray);
  tri (v0, v4, v1,     Qt::gray);
  tri (v6, v9, v8,     Qt::gray);
  tri (v5, v7, v9,     Qt::gray);
  quad(v3, v2, v8, v6, Qt::red);
  quad(v5, v4, v0, v7, Qt::red);
  tri (v1, v3, v2,     Qt::gray);
}

void ShipModelData::add(const QVector3D& v, const QVector3D& n, const QColor color)
{
    GLfloat* p = m_data.data() + m_count;
    *p++ = (v.x() * 2.0f);
    *p++ = (v.y() * 2.0f);
    *p++ = (v.z() * 2.0f);
    *p++ = n.x();
    *p++ = n.y();
    *p++ = n.z();
    *p++ = color.redF();
    *p++ = color.greenF();
    *p++ = color.blueF();
    m_count += 9;
}

void ShipModelData::quad(QVector3D a, QVector3D b, QVector3D c, QVector3D d, QColor color)
{
  //const QVector3D colorV(color.redF(), color.greenF(), color.blueF());
  QVector3D n = QVector3D::normal(QVector3D(d.x() - a.x(), d.y() - a.y(), d.z() - a.z()),
                                  QVector3D(b.x() - a.x(), b.y() - a.y(), b.z() - a.z()));

  add(QVector3D(a.x(), a.y(), a.z()), n, color);
  add(QVector3D(d.x(), d.y(), d.z()), n, color);
  add(QVector3D(b.x(), b.y(), b.z()), n, color);

  add(QVector3D(c.x(), c.y(), c.z()), n, color);
  add(QVector3D(b.x(), b.y(), b.z()), n, color);
  add(QVector3D(d.x(), d.y(), d.z()), n, color);

  // is this second set here to provide a back face? is that what's going on?

  n = QVector3D::normal(QVector3D(a.x() - d.x(), a.y() - d.y(), a.z() - d.z()),
                        QVector3D(b.x() - d.x(), b.y() - d.y(), b.z() - d.z()));

  add(QVector3D(d.x(), d.y(), d.z()), n, color);
  add(QVector3D(a.x(), a.y(), a.z()), n, color);
  add(QVector3D(b.x(), b.y(), b.z()), n, color);

  add(QVector3D(b.x(), b.y(), b.z()), n, color);
  add(QVector3D(c.x(), c.y(), c.z()), n, color);
  add(QVector3D(d.x(), d.y(), d.z()), n, color);
}


void ShipModelData::tri(QVector3D a, QVector3D b, QVector3D c, QColor color)
{
  //const QVector3D colorV(color.redF(), color.greenF(), color.blueF());
  QVector3D n = QVector3D::normal(QVector3D(c.x() - a.x(), c.y() - a.y(), c.z() - a.z()),
                                  QVector3D(b.x() - a.x(), b.y() - a.y(), b.z() - a.z()));

  add(QVector3D(a.x(), a.y(), a.z()), n, color);
  add(QVector3D(c.x(), c.y(), c.z()), n, color);
  add(QVector3D(b.x(), b.y(), b.z()), n, color);

  n = QVector3D::normal(QVector3D(a.x() - c.x(), a.y() - c.y(), a.z() - c.z()),
                        QVector3D(b.x() - c.x(), b.y() - c.y(), b.z() - c.z()));

  add(QVector3D(c.x(), c.y(), c.z()), n, color);
  add(QVector3D(a.x(), a.y(), a.z()), n, color);
  add(QVector3D(b.x(), b.y(), b.z()), n, color);
}
