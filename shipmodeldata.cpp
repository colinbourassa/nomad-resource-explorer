#include "shipmodeldata.h"
#include <QMap>
#include <QVector>
#include <QtEndian>

/**
 * Number of bytes that the smallest possible polygon record will occupy
 * in a .BIN file. This is for a 3-vertex polygon; the record size will
 * increase by 2 bytes (a single 16-bit word) for each additional vertex.
 */
#define MIN_POLYGON_RECORD_SIZE 18

QMap<int,QColor> ShipModelData::s_modelColors =
{
  {0, QColor(32, 32, 32)   },
  {1, QColor(24, 24, 182)  },
  {2, QColor(32, 186, 32)  },
  {3, QColor(65, 178, 178) },
  {4, QColor(207, 24, 24)  },
  {5, QColor(170, 56, 170) },
  {6, QColor(195, 199, 36) },
  {7, QColor(170, 170, 170)}
};

ShipModelData::ShipModelData()
{
  /* TEMP */
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

/**
 * Parses the data read from a .BIN 3D model file and uses its data to populate
 * the internal OpenGL buffers required for rendering.
 */
bool ShipModelData::loadData(const QByteArray& bin, QString& modelInfo)
{
  bool status = false;
  modelInfo.clear();
  int offset = 0;

  // each polygon is composed of a list of vertex IDs and a color
  QMap<int,QVector<int> > polygonPoints;
  QMap<int,QColor> polygonColors;

  // each vertex ID maps to a point in 3D space
  QMap<int,QVector3D> vertices;

  // start by ensuring that we have at least enough data to
  // cover for the 16-bit polygon count
  if (bin.size() >= sizeof(uint16_t))
  {
    const uint16_t numPolys = qFromLittleEndian<quint16>(bin.data() + 0);
    modelInfo += QString("Number of faces: %1\n").arg(numPolys);
    offset += sizeof(uint16_t);

    // iterate over all the polygons defined in this model
    int polyId = 0;
    while ((polyId < numPolys) && (bin.size() >= (offset + MIN_POLYGON_RECORD_SIZE)))
    {
      // ten bytes into the record is a byte containing a 7-bit vertex count
      offset += 10;

      const uint8_t polyVertexCount = *(bin.data() + offset) & 0x7F;
      offset++;

      const uint8_t surfaceColorIndex = *(bin.data() + offset);
      offset++;

      if (s_modelColors.contains(surfaceColorIndex))
      {
        polygonColors[polyId] = s_modelColors[surfaceColorIndex];
        modelInfo += QString("Face %1, color %2:").arg(polyId).arg(s_modelColors[surfaceColorIndex].name());
      }
      else
      {
        polygonColors[polyId] = s_modelColors[7];
        modelInfo += QString("Face %1 (invalid color index %2):").arg(polyId).arg(surfaceColorIndex);
      }

      // interate over all the vertices defined for this polygon
      int vertIndex = 0;
      while ((vertIndex < polyVertexCount) && (bin.size() >= (offset + sizeof(uint16_t))))
      {
        const uint16_t vertId = qFromLittleEndian<quint16>(bin.data() + offset);
        polygonPoints[polyId].append(vertId);
        modelInfo += QString(" %1").arg(vertId);
        offset += sizeof(uint16_t);
        vertIndex++;
      }
      modelInfo += QString("\n");

      if (vertIndex < polyVertexCount)
      {
        status = false;
        modelInfo += QString("\nError: Ran out of data before end of polygon vertex list.\n");
      }

      polyId++;
    }

    // we're done reading the polygon definitions; ensure that we got to
    // the end of that section without running out of data before we
    // proceed to the global vertex list
    if (polyId == numPolys)
    {
      if (bin.size() >= (offset + sizeof(uint16_t)))
      {
        const uint16_t vertCount = qFromLittleEndian<quint16>(bin.data() + offset);
        offset += sizeof(uint16_t);

        int globalVertexIndex = 0;
        while ((globalVertexIndex < vertCount) && (bin.size() >= offset + (3 * sizeof(int16_t))))
        {
          const float vertX = qFromLittleEndian<qint16>(bin.data() + offset);
          offset += sizeof(int16_t);
          const float vertY = qFromLittleEndian<qint16>(bin.data() + offset);
          offset += sizeof(int16_t);
          const float vertZ = qFromLittleEndian<qint16>(bin.data() + offset);
          offset += sizeof(int16_t);

          vertices[globalVertexIndex++] = QVector3D(vertX / 500.0f, vertY / 500.0f, vertZ / 500.0f);
        }

        if (globalVertexIndex == vertCount)
        {
          status = true;

          foreach (int polygonId, polygonPoints.keys())
          {
            QVector<int>* polyVertexList = &polygonPoints[polygonId];

            if (polyVertexList->size() == 3)
            {
              tri(vertices[polyVertexList->at(0)],
                  vertices[polyVertexList->at(1)],
                  vertices[polyVertexList->at(2)],
                  polygonColors[polygonId]);
            }
            else if (polyVertexList->size() == 4)
            {
              quad(vertices[polyVertexList->at(0)],
                   vertices[polyVertexList->at(1)],
                   vertices[polyVertexList->at(2)],
                   vertices[polyVertexList->at(3)],
                   polygonColors[polygonId]);
            }
            else
            {
              modelInfo += QString("\nWarning: Polygon %1 has %2 faces; skipping...\n").arg(polygonId).arg(polyVertexList->size());
            }
          }
        }
        else
        {
          modelInfo += QString("\nError: Ran out of data before the end of the global vertex list.\n");
        }
      }
      else
      {
        modelInfo += QString("\nError: No data left for vertex definitions.\n");
      }
    }
    else
    {
      modelInfo += QString("\nError: Ran out of data before end of polygon definitions.\n");
    }
  }

  return status;
}

/**
 * Clears out the 3D model data to leave an empty scene.
 */
void ShipModelData::clear()
{
  m_count = 0;
  m_data.clear();
}

/**
 * Adds a vertex, its normal, and its color to the OpenGL buffer that will be used later for rendering.
 */
void ShipModelData::add(const QVector3D& v, const QVector3D& n, const QColor color)
{
  GLfloat* p = m_data.data() + m_count;
  *p++ = v.x();
  *p++ = v.y();
  *p++ = v.z();
  *p++ = n.x();
  *p++ = n.y();
  *p++ = n.z();
  *p++ = color.redF();
  *p++ = color.greenF();
  *p++ = color.blueF();
  m_count += FLOATS_PER_VERTEX;
}

/**
 * Triangulates a 4-gon into triangles, computes the normals for the vertices,
 * and calls a routine to add those vertices and their associated attributes to
 * the OpenGL buffer.
 */
void ShipModelData::quad(QVector3D a, QVector3D b, QVector3D c, QVector3D d, QColor color)
{
  QVector3D n = QVector3D::normal(QVector3D(d.x() - a.x(), d.y() - a.y(), d.z() - a.z()),
                                  QVector3D(b.x() - a.x(), b.y() - a.y(), b.z() - a.z()));

  add(QVector3D(a.x(), a.y(), a.z()), n, color);
  add(QVector3D(d.x(), d.y(), d.z()), n, color);
  add(QVector3D(b.x(), b.y(), b.z()), n, color);

  add(QVector3D(c.x(), c.y(), c.z()), n, color);
  add(QVector3D(b.x(), b.y(), b.z()), n, color);
  add(QVector3D(d.x(), d.y(), d.z()), n, color);

  // is this second section here to provide a back face? is that what's going on?

  n = QVector3D::normal(QVector3D(a.x() - d.x(), a.y() - d.y(), a.z() - d.z()),
                        QVector3D(b.x() - d.x(), b.y() - d.y(), b.z() - d.z()));

  add(QVector3D(d.x(), d.y(), d.z()), n, color);
  add(QVector3D(a.x(), a.y(), a.z()), n, color);
  add(QVector3D(b.x(), b.y(), b.z()), n, color);

  add(QVector3D(b.x(), b.y(), b.z()), n, color);
  add(QVector3D(c.x(), c.y(), c.z()), n, color);
  add(QVector3D(d.x(), d.y(), d.z()), n, color);
}

/**
 * Computes the normals for the vertices provided that define a triangle, and
 * calls a routine that adds those vertices, their normals, and their colors
 * to an OpenGL buffer.
 */
void ShipModelData::tri(QVector3D a, QVector3D b, QVector3D c, QColor color)
{
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

