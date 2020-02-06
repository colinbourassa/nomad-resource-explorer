#ifndef ALIENS_H
#define ALIENS_H

#include <QVector>
#include <QString>

class Aliens
{
public:
  Aliens();

private:
  static const QVector<QString> s_animationMap;
};

#endif // ALIENS_H
