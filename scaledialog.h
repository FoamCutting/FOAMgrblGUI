#ifndef SCALEDIALOG_H
#define SCALEDIALOG_H

#include <QDialog>
#include "gcodetext.h"
#include "gcodeplot.h"

namespace Ui {
	class ScaleDialog;
}

class ScaleDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ScaleDialog(GCodeText* text, GCodePlot* plot, QWidget *parent = 0);
	~ScaleDialog();

private slots:
	void on_cancel_clicked();

	void on_apply_clicked();

private:
	Ui::ScaleDialog *ui;
	GCodeText* targetText;
	GCodePlot* targetPlot;

signals:
	void GCodeDocumentAltered();
};

#endif // SCALEDIALOG_H
