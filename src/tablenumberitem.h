#ifndef TABLENUMBERITEM_H
#define TABLENUMBERITEM_H

#include <QTableWidgetItem>

class TableNumberItem : public QTableWidgetItem
{
public:
  TableNumberItem(const QString txt = QString("0"));
  bool operator <(const QTableWidgetItem& other) const;
};

#endif // TABLENUMBERITEM_H
