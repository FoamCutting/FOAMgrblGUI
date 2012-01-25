#ifndef NEWMACHINEDIALOG_H
#define NEWMACHINEDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include "settings.h"

class NewMachineDialog : public QDialog
{
    Q_OBJECT
public:
    explicit NewMachineDialog(QWidget *parent = 0);
    QVBoxLayout *vLayout;
    QHBoxLayout *hLayout;
    QPushButton *ok_pButton;
    QPushButton *cancel_pButton;
    QLineEdit *newMachineName_lEdit;
    void SetSettings(Settings*);

private:
    Settings *settings;

signals:
    void machineChanged();

public slots:

private slots:
    void okay();
    void cancel();

};

#endif // NEWMACHINEDIALOG_H
