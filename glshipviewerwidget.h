#ifndef GLSHIPVIEWERWIDGET_H
#define GLSHIPVIEWERWIDGET_H

#include <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QPoint>
#include <QMouseEvent>
#include "shipmodeldata.h"

static void qNormalizeAngle(int& angle)
{
  while (angle < 0)
  {
    angle += 360 * 16;
  }
  while (angle > 360 * 16)
  {
    angle -= 360 * 16;
  }
}

class GLShipViewerWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
  Q_OBJECT

public:
  GLShipViewerWidget(QWidget* parent = nullptr);
  ~GLShipViewerWidget();

  QSize minimumSizeHint() const override;
  QSize sizeHint() const override;

public slots:
  void setXRotation(int angle);
  void setYRotation(int angle);
  void setZRotation(int angle);
  void cleanup();

signals:
  void xRotationChanged(int angle);
  void yRotationChanged(int angle);
  void zRotationChanged(int angle);

protected:
  void initializeGL() override;
  void paintGL() override;
  void resizeGL(int w, int h) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;

private:
  void setupVertexAttribs();

  int m_xRot = 0;
  int m_yRot = 0;
  int m_zRot = 0;
  QPoint m_lastPos;
  ShipModelData m_model;
  QOpenGLVertexArrayObject m_vao;
  QOpenGLBuffer m_logoVbo;
  QOpenGLShaderProgram* m_program = nullptr;
  int m_projMatrixLoc = 0;
  int m_mvMatrixLoc = 0;
  int m_normalMatrixLoc = 0;
  int m_lightPosLoc = 0;
  QMatrix4x4 m_proj;
  QMatrix4x4 m_camera;
  QMatrix4x4 m_world;
};

#endif // GLSHIPVIEWERWIDGET_H
