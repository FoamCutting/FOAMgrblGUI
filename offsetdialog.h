#ifndef OFFSETDIALOG_H
#define OFFSETDIALOG_H

#include <QDialog>
#include "gcodetext.h"
#include "gcodeplot.h"

namespace Ui {
	class OffsetDialog;
}

class OffsetDialog : public QDialog
{
	Q_OBJECT

public:
	explicit OffsetDialog(GCodeText* targetText, GCodePlot* targetPlot, QWidget *parent = 0);
	~OffsetDialog();

private slots:
	void on_cancel_clicked();

	void on_apply_clicked();

private:
	Ui::OffsetDialog *ui;
	GCodeText* targetText;
	GCodePlot* targetPlot;

signals:
	void GCodeDocumentAltered();
};

#endif // OFFSETDIALOG_H
