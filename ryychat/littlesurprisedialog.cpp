#include "littlesurprisedialog.h"
#include "ui_littlesurprisedialog.h"

LittleSurpriseDialog::LittleSurpriseDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LittleSurpriseDialog)
{
    ui->setupUi(this);
}

LittleSurpriseDialog::~LittleSurpriseDialog()
{
    delete ui;
}

void LittleSurpriseDialog::on_return_btn_clicked()
{
    emit sigSwitchLogin();
}
