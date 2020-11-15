#include "glshipviewerwidget.h"

#define ROTATION_STEP 16
#define ROTATION_MAX 5760

static const char *vertexShaderSourceCore =
    "#version 140\n"
    "in vec4 vertex;\n"
    "in vec3 normal;\n"
    "in vec3 color;\n"
    "out vec3 vert;\n"
    "out vec3 vertNormal;\n"
    "out vec3 vertColor;\n"
    "uniform mat4 projMatrix;\n"
    "uniform mat4 mvMatrix;\n"
    "uniform mat3 normalMatrix;\n"
    "void main() {\n"
    "   vert = vertex.xyz;\n"
    "   vertNormal = normalMatrix * normal;\n"
    "   vertColor = color;\n"
    "   gl_Position = projMatrix * mvMatrix * vertex;\n"
    "}\n";

static const char *fragmentShaderSourceCore =
    "#version 140\n"
    "in highp vec3 vert;\n"
    "in highp vec3 vertNormal;\n"
    "in highp vec3 vertColor;\n"
    "out highp vec4 fragColor;\n"
    "uniform highp vec3 lightPos;\n"
    "void main() {\n"
    "   highp vec3 L = normalize(lightPos - vert);\n"
    "   highp float NL = max(dot(normalize(vertNormal), L), 0.0);\n"
    "   highp vec3 color = vertColor;\n"
    "   highp vec3 col = clamp(color * 0.2 + color * 0.8 * NL, 0.0, 1.0);\n"
    "   fragColor = vec4(col, 1.0);\n"
    "}\n";

GLShipViewerWidget::GLShipViewerWidget(QWidget* parent) : QOpenGLWidget(parent)
{

}

GLShipViewerWidget::~GLShipViewerWidget()
{
  cleanup();
}

QSize GLShipViewerWidget::minimumSizeHint() const
{
  return QSize(50, 50);
}

QSize GLShipViewerWidget::sizeHint() const
{
  return QSize(400, 400);
}

void GLShipViewerWidget::setXRotation(int angle)
{
  qNormalizeAngle(angle);
  if (angle != m_xRot)
  {
    m_xRot = angle;
    emit xRotationChanged(angle);
    update();
  }
}

void GLShipViewerWidget::setYRotation(int angle)
{
  qNormalizeAngle(angle);
  if (angle != m_yRot)
  {
    m_yRot = angle;
    emit yRotationChanged(angle);
    update();
  }
}

void GLShipViewerWidget::setZRotation(int angle)
{
  qNormalizeAngle(angle);
  if (angle != m_zRot)
  {
    m_zRot = angle;
    emit zRotationChanged(angle);
    update();
  }
}

void GLShipViewerWidget::incrementXRotation()
{
  m_xRot += ROTATION_STEP;
  if (m_xRot > ROTATION_MAX)
  {
    m_xRot = 0;
  }
  emit xRotationChanged(m_xRot);
  update();
}

void GLShipViewerWidget::incrementYRotation()
{
  m_yRot += ROTATION_STEP;
  if (m_yRot > ROTATION_MAX)
  {
    m_yRot = 0;
  }
  emit yRotationChanged(m_yRot);
  update();
}

void GLShipViewerWidget::incrementZRotation()
{
  m_zRot += ROTATION_STEP;
  if (m_zRot > ROTATION_MAX)
  {
    m_zRot = 0;
  }
  emit zRotationChanged(m_zRot);
  update();
}

void GLShipViewerWidget::resetView()
{
  // preset the X and Z rotations for a reasonable view
  m_xRot = (ROTATION_MAX / 3 * 2);
  m_yRot = 0;
  m_zRot = (ROTATION_MAX / 8);
  m_zoom = 1.0;
  update();
  emit xRotationChanged(m_xRot);
  emit yRotationChanged(m_yRot);
  emit zRotationChanged(m_zRot);
}

void GLShipViewerWidget::cleanup()
{
  if (m_program == nullptr)
  {
    return;
  }
  makeCurrent();
  m_logoVbo.destroy();
  delete m_program;
  m_program = nullptr;
  doneCurrent();
}

void GLShipViewerWidget::clear()
{
  m_model.clear();
  m_logoVbo.bind();
  m_logoVbo.allocate(m_model.constData(), m_model.count() * sizeof(GLfloat));
  repaint();
}

bool GLShipViewerWidget::loadData(const QByteArray& bin, QString& modelInfo)
{
  bool status = m_model.loadData(bin, modelInfo);

  m_logoVbo.bind();
  m_logoVbo.allocate(m_model.constData(), m_model.count() * sizeof(GLfloat));
  repaint();

  return status;
}

void GLShipViewerWidget::initializeGL()
{
  connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &GLShipViewerWidget::cleanup);

  initializeOpenGLFunctions();
  glClearColor(0, 0, 0, 1);

  m_program = new QOpenGLShaderProgram;
  m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSourceCore);
  m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSourceCore);
  m_program->bindAttributeLocation("vertex", 0);
  m_program->bindAttributeLocation("normal", 1);
  m_program->bindAttributeLocation("color", 2);
  m_program->link();

  m_program->bind();
  m_projMatrixLoc = m_program->uniformLocation("projMatrix");
  m_mvMatrixLoc = m_program->uniformLocation("mvMatrix");
  m_normalMatrixLoc = m_program->uniformLocation("normalMatrix");
  m_lightPosLoc = m_program->uniformLocation("lightPos");

  // Create a vertex array object. In OpenGL ES 2.0 and OpenGL 2.x
  // implementations this is optional and support may not be present
  // at all. Nonetheless the below code works in all cases and makes
  // sure there is a VAO when one is needed.
  m_vao.create();
  QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

  // Setup our vertex buffer object.
  m_logoVbo.create();
  m_logoVbo.bind();
  m_logoVbo.allocate(m_model.constData(), m_model.count() * sizeof(GLfloat));

  // Store the vertex attribute bindings for the program.
  setupVertexAttribs();

  // fixed camera
  m_camera.setToIdentity();
  m_camera.translate(0, 0, -2);

  // fixed lighting
  m_program->setUniformValue(m_lightPosLoc, QVector3D(0, 0, 70));

  m_program->release();
}

void GLShipViewerWidget::setupVertexAttribs()
{
  m_logoVbo.bind();
  QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
  f->glEnableVertexAttribArray(0);
  f->glEnableVertexAttribArray(1);
  f->glEnableVertexAttribArray(2);
  f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), nullptr);
  f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), reinterpret_cast<void *>(3 * sizeof(GLfloat)));
  f->glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), reinterpret_cast<void *>(6 * sizeof(GLfloat)));
  m_logoVbo.release();
}

void GLShipViewerWidget::resizeGL(int w, int h)
{
  m_proj.setToIdentity();
  m_proj.perspective(45.0f, GLfloat(w) / h, 0.01f, 100.0f);
}

void GLShipViewerWidget::paintGL()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  m_world.setToIdentity();
  m_world.rotate(180.0f - (m_xRot / 16.0f), 1, 0, 0);
  m_world.rotate(m_yRot / 16.0f, 0, 1, 0);
  m_world.rotate(m_zRot / 16.0f, 0, 0, 1);
  m_world.scale(m_zoom);

  QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
  m_program->bind();
  m_program->setUniformValue(m_projMatrixLoc, m_proj);
  m_program->setUniformValue(m_mvMatrixLoc, m_camera * m_world);
  QMatrix3x3 normalMatrix = m_world.normalMatrix();
  m_program->setUniformValue(m_normalMatrixLoc, normalMatrix);

  glDrawArrays(GL_TRIANGLES, 0, m_model.vertexCount());

  m_program->release();
}

void GLShipViewerWidget::mouseMoveEvent(QMouseEvent* event)
{
  int dx = event->x() - m_lastPos.x();
  int dy = event->y() - m_lastPos.y();

  if (event->buttons() & Qt::LeftButton)
  {
    setXRotation(m_xRot + 8 * dy);
    setYRotation(m_yRot + 8 * dx);
  }
  else if (event->buttons() & Qt::RightButton)
  {
    setXRotation(m_xRot + 8 * dy);
    setZRotation(m_zRot + 8 * dx);
  }
  m_lastPos = event->pos();
}

void GLShipViewerWidget::mousePressEvent(QMouseEvent* event)
{
  m_lastPos = event->pos();
}

void GLShipViewerWidget::wheelEvent(QWheelEvent* event)
{
  const float deltaf = event->angleDelta().y() / 800.0f;
  m_zoom += deltaf;
  repaint();
}
