#ifndef LITTLESURPRISEDIALOG_H
#define LITTLESURPRISEDIALOG_H

#include <QDialog>

namespace Ui {
class LittleSurpriseDialog;
}

class LittleSurpriseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LittleSurpriseDialog(QWidget *parent = nullptr);
    ~LittleSurpriseDialog();

private:
    Ui::LittleSurpriseDialog *ui;

private slots:
    void on_return_btn_clicked();
signals:
    void sigSwitchLogin();
};

#endif // LITTLESURPRISEDIALOG_H
