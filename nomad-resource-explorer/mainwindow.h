#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QGraphicsScene>
#include "datlibrary.h"
#include "inventory.h"
#include "palette.h"
#include "places.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
  void on_actionOpen_game_data_dir_triggered();
  void on_actionExit_triggered();
  void on_actionClose_data_files_triggered();
  void on_m_objTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
  void on_m_placeTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

private:
  Ui::MainWindow *ui;

  QString m_gamedir;

  DatLibrary m_lib;
  Inventory m_inventory;
  Places m_places;
  Palette m_palette;

  QGraphicsScene m_objScene;
  QGraphicsScene m_planetSurfaceScene;

  void populatePlaceWidgets();
  void populateObjectWidgets();
};

#endif // MAINWINDOW_H
