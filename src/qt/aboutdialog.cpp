#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include "clientmodel.h"
#include "clientversion.h"

// Copyright year (2009-this)
// Todo: update this when changing our copyright comments in the source
const int ABOUTDIALOG_COPYRIGHT_YEAR = 2014;

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    // Set current copyright year
    ui->label_2->setText(
        tr("Copyright") 
      + QString(" &copy; 2009-2013 ") + tr("The Bitcoin developers")
      + QString("<br>") + tr("Copyright")
      + QString(" &copy; 2011-2013 ") + tr("The Litecoin developers")
      + QString("<br>") + tr("Copyright")
      + QString(" &copy; 2013-2014 ") + tr("The Vertcoin Developers")
      + QString("<br>") + tr("Copyright")
      + QString(" &copy; 2013-2014 ") + tr("The SiliconValleycoin Developers")
      + QString("<br>") + tr("Copyright")
      + QString(" &copy; 2013-2014 ") + tr("Cohors LLC")      
    );}

void AboutDialog::setModel(ClientModel *model)
{
    if(model)
    {
        //->versionLabel->setText(model->formatFullVersion());
    }
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::on_buttonBox_accepted()
{
    close();
}
