#ifndef MACHINESELECT_H
#define MACHINESELECT_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include "settings.h"

class MachineSelect : public QDialog
{
    Q_OBJECT
public:
    explicit MachineSelect(QString, QWidget *parent = 0);
    QVBoxLayout *vLayout;
    QHBoxLayout *hLayout;
    QPushButton *ok_pButton;
    QPushButton *cancel_pButton;
    QComboBox *machineList_combo;
    void SetSettings(Settings*);
    void SetPreference(QString);

private:
    Settings *settings;
    QString *preferred;

signals:
    void machineChanged();

public slots:

private slots:
	void okay();
	void cancel();
};

#endif // MACHINESELECT_H
