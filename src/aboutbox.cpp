#include "aboutbox.h"
#include "ui_aboutbox.h"

AboutBox::AboutBox(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::AboutBox)
{
  ui->setupUi(this);
  ui->m_labelTitle->setText(QString("Nomad Resource Explorer v%1.%2.%3").arg(NRE_VER_MAJOR).arg(NRE_VER_MINOR).arg(NRE_VER_PATCH));
}

AboutBox::~AboutBox()
{
  delete ui;
}

void AboutBox::on_m_buttonClose_clicked()
{
  this->close();
}
