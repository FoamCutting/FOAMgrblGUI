#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <QDialog>

namespace Ui {
    class ErrorHandler;
}

class ErrorHandler : public QDialog
{
    Q_OBJECT
private:
    int listPosition;
    void inline display(QString errorText);
    QList<int> errorList;



public:
    void assessErrorList();
    explicit ErrorHandler(QWidget *parent = 0);
    ~ErrorHandler();
    void pushError(int errorCode);
    bool checkError();
    enum{Okay,
         UnexpectedSyntax,
         FileIsAbsolute,
         FileIsIncremental,
         GCodeFileNotOpen,
         UnevenArcScalingNotSupported,
         SaveFailed,
         LineNumbersExist,
         NoArduinoConnected};

private slots:
    void on_closeButton_clicked();

private:
    Ui::ErrorHandler *ui;
};

#endif // ERRORHANDLER_H
