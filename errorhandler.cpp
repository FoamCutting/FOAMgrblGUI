#include "errorhandler.h"
#include "ui_errorhandler.h"

ErrorHandler::ErrorHandler(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ErrorHandler)
{
    ui->setupUi(this);
    Qt::WindowFlags flags = ((this->windowFlags()) | Qt::WindowStaysOnTopHint);
    this->setWindowFlags(flags);
}

ErrorHandler::~ErrorHandler()
{
    delete ui;
}

void ErrorHandler::on_closeButton_clicked()
{
    this->hide();
}

void inline ErrorHandler::display(QString errorText)
{
    ui->errorDisplay->setText(errorText);
    this->show();
}

void ErrorHandler::pushError(int errorCode)
{
    errorList << errorCode;
    listPosition++;
}

bool ErrorHandler::checkError()
{
    for(int i = 0; i < errorList.size(); i++)
    {
        if(errorList[i])
            return true;
    }
    return false;
}

void ErrorHandler::assessErrorList()
{

    while(!errorList.isEmpty())
    {
        switch(errorList.takeFirst())
        {
        case Okay:
            break;
        case UnexpectedSyntax:
            display("Unexpected Syntax\n");
            break;
        case FileIsAbsolute:
            display("File is already Absolute\n");
            break;
        case FileIsIncremental:
            display("File is already Incremental\n");
            break;
        case GCodeFileNotOpen:
            display("There is no open file\n");
            break;
        case UnevenArcScalingNotSupported:
            display("This program does not support unequal x and y scaling for g code files that use arc interpolation\n");
            break;
        case UnsavedChanges:
            display("There are unsaved changes to the current file. You must save before streaming.\n");
        }
    }
    errorList.clear();
}
