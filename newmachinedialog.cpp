#include "newmachinedialog.h"

NewMachineDialog::NewMachineDialog(QWidget *parent) :
    QDialog(parent)
{
    this->setFixedSize(300,100);
    vLayout = new QVBoxLayout(this);
    hLayout = new QHBoxLayout();
    ok_pButton = new QPushButton("ok", this);
    cancel_pButton = new QPushButton("cancel", this);
    newMachineName_lEdit = new QLineEdit();
    vLayout->addWidget(newMachineName_lEdit);
    vLayout->addStretch();
    vLayout->addLayout(hLayout);
    hLayout->addWidget(cancel_pButton);
    hLayout->addStretch();
    hLayout->addWidget(ok_pButton);
    QDir::setCurrent("MachineSettings");
    QStringList fileList = QDir::current().entryList();

    connect(ok_pButton, SIGNAL(clicked()), this, SLOT(okay()));
    connect(cancel_pButton, SIGNAL(clicked()), this, SLOT(cancel()));
}

void NewMachineDialog::okay()
{
    settings->setMachineName(newMachineName_lEdit->text());
    this->done(0);
}

void NewMachineDialog::cancel()
{
    QDir::setCurrent(settings->ApplicationDirectory().absolutePath());
    this->done(0);
}

void NewMachineDialog::SetSettings(Settings *s)
{
    QDir::setCurrent(settings->ApplicationDirectory().absolutePath());
    settings = s;
}
