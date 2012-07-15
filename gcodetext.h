#ifndef GCODETEXT_H
#define GCODETEXT_H



#include <QDebug>

#include <QObject>
#include <QTextDocument>
#include <QTextStream>
#include <QFile>
#include "errorhandler.h"
#include "Toolbox.h"
#include "settings.h"

class GCodeText : public QObject
{
	Q_OBJECT

private:
	bool GCodeDocumentSet;
	QTextDocument* GCodeDocument;
	QFile* GCodeFile;
	ErrorHandler* err;
	Settings* settings;

public:
	~GCodeText();
	explicit GCodeText(QObject *parent = 0);
	void setDocument(QTextDocument *document);
	void setErrorHandler(ErrorHandler *handler);
	void setFile(QFile*);
	QTextDocument* document();
	void scale(float xScale, float yScale);
	void offset(float xOffset, float yOffset, bool units);
	void toAbsolute();
	void toIncremental();
	void addLineNumbers();
	void removeLineNumbers();
	int countLines();
	int getUnits();
	bool check();
	QString Preprocess();
	void SetSettings(Settings*);

signals:

public slots:

};

#endif // GCODETEXT_H
