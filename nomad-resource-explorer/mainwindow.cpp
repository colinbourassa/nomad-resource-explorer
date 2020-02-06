#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QVector>
#include <QTableWidgetItem>
#include <QImage>
#include <QGraphicsScene>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  m_inventory(m_lib, m_palette),
  m_palette(m_lib)
{
  ui->setupUi(this);
  ui->m_objectImageView->scale(3, 3);

  m_lib.openData("/home/cmb/opt/games/nomad");
  populatePlaceWidgets();
  populateObjectWidgets();
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::on_actionOpen_game_data_dir_triggered()
{
  m_gamedir = QFileDialog::getExistingDirectory(this, "Select directory containing Nomad .DAT files", "/home", QFileDialog::ShowDirsOnly);
  m_lib.openData(m_gamedir);
}

void MainWindow::on_actionExit_triggered()
{
  this->close();
}

void MainWindow::on_actionClose_data_files_triggered()
{
  m_lib.closeData();
}

void MainWindow::on_pushButton_clicked()
{
}

void MainWindow::populatePlaceWidgets()
{

}

void MainWindow::populateObjectWidgets()
{
  QMap<int,InventoryObj> objs = m_inventory.getObjectList();
  ui->m_objTable->clear();
  foreach (InventoryObj obj, objs.values())
  {
    const int rowcount = ui->m_objTable->rowCount();

    ui->m_objTable->insertRow(rowcount);
    ui->m_objTable->setItem(rowcount, 0, new QTableWidgetItem(QString("%1").arg(obj.id)));
    ui->m_objTable->setItem(rowcount, 1, new QTableWidgetItem(obj.name));
  }
  ui->m_objTable->resizeColumnsToContents();
}


void MainWindow::on_m_objTable_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
  Q_UNUSED(currentColumn)
  Q_UNUSED(previousRow)
  Q_UNUSED(previousColumn)

  int id = ui->m_objTable->item(currentRow, 0)->text().toInt();
  QPixmap pm = m_inventory.getInventoryImage(id);
  m_objScene.addPixmap(pm);
  ui->m_objectImageView->setScene(&m_objScene);
  ui->m_objectTypeLabel->setText("Type: " + getInventoryObjTypeText(m_inventory.getObjectType(id)));
}
