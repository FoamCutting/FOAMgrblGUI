#include "scaledialog.h"
#include "ui_scaledialog.h"

ScaleDialog::ScaleDialog(GCodeText* text, GCodePlot* plot, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScaleDialog)
{
    ui->setupUi(this);
    targetText = text;
    targetPlot = plot;
}

ScaleDialog::~ScaleDialog()
{
    delete ui;
}

void ScaleDialog::on_cancel_clicked()
{
    delete this;
}

void ScaleDialog::on_apply_clicked()
{
    targetText->scale(ui->xDoubleSpinBox->value(), ui->yDoubleSpinBox->value());
    emit GCodeDocumentAltered();
    delete this;
}
