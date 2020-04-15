#include "tablenumberitem.h"

/**
 * Subclass of QTableWidgetItems with more intelligent sorting of strings that represent numbers.
 */
TableNumberItem::TableNumberItem(const QString txt) :
  QTableWidgetItem (txt)
{

}

bool TableNumberItem::operator<(const QTableWidgetItem &other) const
{
  if (text().isEmpty())
  {
    return text().toDouble() > other.text().toDouble();
  }
  return text().toDouble() < other.text().toDouble();
}
