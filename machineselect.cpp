#include "machineselect.h"

MachineSelect::MachineSelect(QString pref, QWidget *parent) :
    QDialog(parent)
{
    this->setFixedSize(300,100);
    vLayout = new QVBoxLayout(this);
    hLayout = new QHBoxLayout();
    ok_pButton = new QPushButton("ok", this);
    cancel_pButton = new QPushButton("cancel", this);
    machineList_combo = new QComboBox();
    vLayout->addWidget(machineList_combo);
    vLayout->addStretch();
    vLayout->addLayout(hLayout);
    hLayout->addWidget(cancel_pButton);
    hLayout->addStretch();
    hLayout->addWidget(ok_pButton);
    QDir::setCurrent("MachineSettings");
    QStringList fileList = QDir::current().entryList();
    for(int i = 2; i < fileList.size(); i++) {
	machineList_combo->addItem(fileList.at(i));
	if(fileList.at(i) == pref)
	    machineList_combo->setCurrentIndex(i);
    }
    if(fileList.contains(pref))
	machineList_combo->setCurrentIndex(fileList.indexOf(QRegExp(pref))-2);
    connect(ok_pButton, SIGNAL(clicked()), this, SLOT(okay()));
    connect(cancel_pButton, SIGNAL(clicked()), this, SLOT(cancel()));
}

void MachineSelect::okay()
{
    settings->setMachineName(machineList_combo->currentText());
    emit machineChanged();
    this->done(0);
}

void MachineSelect::cancel()
{
    QDir::setCurrent(settings->ApplicationDirectory().absolutePath());
    this->done(0);
}

void MachineSelect::SetSettings(Settings *s)
{
    QDir::setCurrent(settings->ApplicationDirectory().absolutePath());
    settings = s;
}

void MachineSelect::SetPreference(QString pref)
{
    preferred = &pref;
}
