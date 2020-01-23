#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QVector>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow),
  m_inventory(m_lib)
{
  ui->setupUi(this);
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
  QVector<InventoryObj> objs = m_inventory.getObjectList();
  foreach (InventoryObj obj, objs)
  {
    ui->listWidget->addItem(obj.name);
  }
}

