#include "offsetdialog.h"
#include "ui_offsetdialog.h"

OffsetDialog::OffsetDialog(GCodeText* text, GCodePlot* plot, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::OffsetDialog)
{
	ui->setupUi(this);
	targetText = text;
	targetPlot = plot;
}

OffsetDialog::~OffsetDialog()
{
	delete ui;
}

void OffsetDialog::on_cancel_clicked()
{
	delete this;
}

void OffsetDialog::on_apply_clicked()
{
	targetText->offset(ui->xDoubleSpinBox->value(), ui->yDoubleSpinBox->value(), ui->units_combo->currentIndex());
	emit GCodeDocumentAltered();
	delete this;
}
