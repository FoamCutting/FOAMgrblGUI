#include "gcodetext.h"

GCodeText::GCodeText(QObject *parent) :
    QObject(parent)
{
    GCodeDocumentSet = false;
    err = new ErrorHandler;
}

GCodeText::~GCodeText()
{

}

void GCodeText::setErrorHandler(ErrorHandler *handler)
{
    err = handler;
}

void GCodeText::setFile(QFile *file)
{
    GCodeFile = file;
}

void GCodeText::setDocument(QTextDocument *document)
{
    GCodeDocument = document;
    GCodeDocumentSet = true;
    err->pushError(ErrorHandler::Okay);
}

QTextDocument* GCodeText::document()
{
    return GCodeDocument;
}

void GCodeText::scale(float xScale, float yScale)
{
    if(!GCodeDocumentSet)
    {
        err->pushError(ErrorHandler::GCodeFileNotOpen);
        err->assessErrorList();
        return;
    }
    QString docString = GCodeDocument->toPlainText();
    QTextStream docStream(&docString);
    docStream.seek(0);

    QString changeBuffer;
    QTextStream stream(&changeBuffer);

    while(!(docStream.atEnd()))
    {
        if(err->checkError())
        {
            err->assessErrorList();
            return;
        }
        bool ok = true;
        QString codeLine;
        QList<QString> codeLineList;
        QString command;
        codeLine = docStream.readLine();
        codeLineList = codeLine.toUpper().split(" ", QString::SkipEmptyParts);
        for(int i = 0; i<(codeLineList.size()); i++)
        {
            command = codeLineList[i].trimmed();

            if(command[0] == '(')
            {
                for(;i<(codeLineList.size()); i++)
                {
                    stream << codeLineList[i] << ' ';
                }
                break;
            }
            else if(command[0] == 'X')
            {
                float coordNumber = command.remove(0,1).toFloat(&ok);
                if(!ok)
                {
                    err->pushError(ErrorHandler::UnexpectedSyntax);
                    continue;
                }
                coordNumber *= xScale;
                stream << 'X' << coordNumber;
            }
            else if(command[0] == 'Y')
            {
                float coordNumber = command.remove(0,1).toFloat(&ok);
                if(!ok)
                {
                    err->pushError(ErrorHandler::UnexpectedSyntax);
                    continue;
                }
                coordNumber *= yScale;
                stream << 'Y' << coordNumber;
            }
            else if(command[0] == 'R')
            {
                float coordNumber = command.remove(0,1).toFloat(&ok);
                if(!ok)
                {
                    err->pushError(ErrorHandler::UnexpectedSyntax);
                    continue;
                }
                coordNumber *= xScale;
                stream << 'R' << coordNumber;
            }
            else if(command[0] == 'I')
            {
                if(xScale != yScale)
                {
                    err->pushError(ErrorHandler::UnevenArcScalingNotSupported);
                    continue;
                }
                float coordNumber = command.remove(0,1).toFloat(&ok);
                if(!ok)
                {
                    err->pushError(ErrorHandler::UnexpectedSyntax);
                    continue;
                };
                coordNumber *= xScale;
                stream << 'I' << coordNumber;
            }
            else if(command[0] == 'J')
            {
                if(xScale != yScale)
                {
                    err->pushError(ErrorHandler::UnevenArcScalingNotSupported);
                    continue;
                }
                float coordNumber = command.remove(0,1).toFloat(&ok);
                if(!ok)
                {
                    err->pushError(ErrorHandler::UnexpectedSyntax);
                    continue;
                }
                coordNumber *= yScale;
                stream << 'J' << coordNumber;
            }
            else
            {
                stream << command;
            }
            if(i != (codeLineList.size() - 1))
                stream << ' ';
        }
        stream << endl;
    }
    GCodeDocument->setPlainText(changeBuffer);
    err->pushError(ErrorHandler::Okay);
    err->assessErrorList();
}

void GCodeText::offset(float xOffset, float yOffset, bool units)
{
    float unitMultiplier = 1;
    if(!GCodeDocumentSet)
    {
        err->pushError(ErrorHandler::GCodeFileNotOpen);
        err->assessErrorList();
        return;
    }

    QString docString = GCodeDocument->toPlainText();
    QTextStream docStream(&docString);
    docStream.seek(0);

    QString changeBuffer;
    QTextStream stream(&changeBuffer);

    while(!(docStream.atEnd()))
    {
        if(err->checkError())
        {
            err->assessErrorList();
            return;
        }
        bool ok = true;
        QString codeLine;
        QList<QString> codeLineList;
        QString command;
        QString numberString;
        codeLine = docStream.readLine();
        codeLineList = codeLine.toUpper().split(" ", QString::SkipEmptyParts);
        for(int i = 0; i<(codeLineList.size()); i++)
        {
            command = codeLineList[i];
            numberString = command.remove(' ');
            numberString.remove(0,1);
            if(codeLine[0] == '(')
            {
                command.trimmed();
                stream << codeLine;
                break;
            }
            else if(command == "G91")
            {
                toAbsolute();
                offset(xOffset, yOffset, units);
                toIncremental();
                err->pushError(ErrorHandler::Okay);
                continue;
            }
            else if(command == "G20") {
                if(units != 0)
                    unitMultiplier = 1.0/25.4;
                else
                    unitMultiplier = 1;
                stream << command;
            }
            else if(command == "G21") {
                if(units != 1)
                    unitMultiplier = 25.4;
                else
                    unitMultiplier = 1;
                stream << command;
            }
            else if(command[0] == 'X')
            {
                float coordNumber = numberString.toFloat(&ok);
                if(!ok)
                {
                    err->pushError(ErrorHandler::UnexpectedSyntax);
                    continue;
                }
                coordNumber += xOffset*unitMultiplier;
                stream << 'X' << coordNumber;
            }
            else if(command[0] == 'Y')
            {
                float coordNumber = numberString.toFloat(&ok);
                if(!ok)
                {
                    err->pushError(ErrorHandler::UnexpectedSyntax);
                    continue;
                }
                coordNumber += yOffset*unitMultiplier;
                stream << 'Y' << coordNumber;
            }
            else
            {
                command.trimmed();
                stream << command;
            }
            if(i != (codeLineList.size() - 1))
                stream << ' ';
        }
        stream << endl;
    }
    GCodeDocument->setPlainText(changeBuffer);
    err->pushError(ErrorHandler::Okay);
    err->assessErrorList();
}

void GCodeText::toAbsolute()
{
    if(!GCodeDocumentSet)
    {
        err->pushError(ErrorHandler::GCodeFileNotOpen);
        err->assessErrorList();
        return;
    }

    QString docString = GCodeDocument->toPlainText();
    QTextStream docStream(&docString);
    docStream.seek(0);

    QString changeBuffer;
    QTextStream stream(&changeBuffer);

    float lastCoord[2];
    lastCoord[0] = 0;
    lastCoord[1] = 0;

    while(!(docStream.atEnd()))
    {
        if(err->checkError())
        {
            err->assessErrorList();
            return;
        }
        bool ok = true;
        QString codeLine;
        QList<QString> codeLineList;
        QString command;
        QString numberString;
        codeLine = docStream.readLine().trimmed();
        codeLineList = codeLine.toUpper().split(" ", QString::SkipEmptyParts);
        for(int i = 0; i<(codeLineList.size()); i++)
        {
            command = codeLineList[i];
            numberString = command.remove(' ');
            numberString.remove(0,1);

            if(codeLine[0] == '(')
            {
                command.trimmed();
                stream << codeLine;
                break;
            }
            else if(command == "G90")
            {
                err->pushError(ErrorHandler::FileIsAbsolute);
                continue;
            }
            else if(command == "G91")
            {
                stream << "G90";
            }
            else if(command[0] == 'X')
            {
                float coordNumber = numberString.toFloat(&ok);
                if(!ok)
                {
                    err->pushError(ErrorHandler::UnexpectedSyntax);
                    continue;
                }
                stream << 'X' << (coordNumber + lastCoord[0]);
                lastCoord[0] = coordNumber + lastCoord[0];
            }
            else if(command[0] == 'Y')
            {
                float coordNumber = numberString.toFloat(&ok);
                if(!ok)
                {
                    err->pushError(ErrorHandler::UnexpectedSyntax);
                    continue;
                }
                stream << 'Y' << (coordNumber + lastCoord[1]);
                lastCoord[1] = coordNumber + lastCoord[1];
            }
//            else if(command[0] == 'I')
//            {

//            }
//            else if(command[0] == 'J')
//            {

//            }
            else
            {
                command.trimmed();
                stream << command;
            }
            if(i != (codeLineList.size() - 1))
                stream << ' ';
        }
        stream << endl;
    }
    GCodeDocument->setPlainText(changeBuffer);
    err->pushError(ErrorHandler::Okay);
}

void GCodeText::toIncremental()
{
    if(!GCodeDocumentSet)
    {
        err->pushError(ErrorHandler::GCodeFileNotOpen);\
        err->assessErrorList();
        return;
    }

    QString docString = GCodeDocument->toPlainText();
    QTextStream docStream(&docString);
    docStream.seek(0);

    QString changeBuffer;
    QTextStream stream(&changeBuffer);

    float lastAbsCoord[2];
    lastAbsCoord[0] = 0;
    lastAbsCoord[1] = 0;

    while(!(docStream.atEnd()))
    {
        if(err->checkError())
        {
            err->assessErrorList();
            return;
        }
        bool ok = true;
        QString codeLine;
        QList<QString> codeLineList;
        QString command;
        QString numberString;
        codeLine = docStream.readLine().simplified();
        codeLineList = codeLine.toUpper().split(" ", QString::SkipEmptyParts);
        for(int i = 0; i<(codeLineList.size()); i++)
        {
            command = codeLineList[i];
            numberString = command.remove(' ').trimmed();
            numberString.remove(0,1);

            if(codeLine[0] == '(')
            {
                command.trimmed();
                stream << codeLine;
                break;
            }
            else if(command == "G91")
            {
                err->pushError(ErrorHandler::FileIsIncremental);
                continue;
            }
            else if(command == "G90")
            {
                stream << "G91";
            }
            else if(command[0] == 'X')
            {
                float coordNumber = numberString.toFloat(&ok);
                if(!ok)
                {
                    err->pushError(ErrorHandler::UnexpectedSyntax);
                    continue;
                }
                stream << 'X' << (coordNumber - lastAbsCoord[0]);
                lastAbsCoord[0] = coordNumber;
            }
            else if(command[0] == 'Y')
            {
                float coordNumber = numberString.toFloat(&ok);
                if(!ok)
                {
                    err->pushError(ErrorHandler::UnexpectedSyntax);
                    continue;
                }
                stream << 'Y' << (coordNumber - lastAbsCoord[1]);
                lastAbsCoord[1] = coordNumber;
            }
//            else if(command[0] == 'I')
//            {

//            }
//            else if(command[0] == 'J')
//            {

//            }
            else
            {
                command.trimmed();
                stream << command;
            }
            if(i != (codeLineList.size() - 1))
                stream << ' ';
        }
        stream << endl;
    }
    GCodeDocument->setPlainText(changeBuffer);
    err->pushError(ErrorHandler::Okay);
}

void GCodeText::addLineNumbers()
{
    if(!GCodeDocumentSet)
    {
        err->pushError(ErrorHandler::GCodeFileNotOpen);
        err->assessErrorList();
        return;
    }

    QString docString = GCodeDocument->toPlainText();
    QTextStream docStream(&docString);
    docStream.seek(0);

    QString changeBuffer;
    QTextStream stream(&changeBuffer);

    QString codeLine;
    int lineNumber = 0;
    while(!(docStream.atEnd()))
    {
        if(err->checkError())
        {
            err->assessErrorList();
            return;
        }
        codeLine = docStream.readLine().trimmed();
        if(codeLine[0] == '(')
        {
            stream << codeLine;
        }
        else if(codeLine[0] == 'N')
        {
            err->pushError(ErrorHandler::LineNumbersExist);
            continue;
        }
        else
        {
            stream << "N" << lineNumber << ' ' << codeLine;
            lineNumber++;
        }
        stream << endl;
    }
    GCodeDocument->setPlainText(changeBuffer);
    err->pushError(ErrorHandler::Okay);
}

void GCodeText::removeLineNumbers()
{
    if(!GCodeDocumentSet)
    {
        err->pushError(ErrorHandler::GCodeFileNotOpen);
        err->assessErrorList();
        return;
    }

    QString docString = GCodeDocument->toPlainText();
    QTextStream docStream(&docString);
    docStream.seek(0);

    QString changeBuffer;
    QTextStream stream(&changeBuffer);

    while(!(docStream.atEnd()))
    {
        if(err->checkError())
        {
            err->assessErrorList();
            return;
        }
        QString codeLine;
        QList<QString> codeLineList;
        QString command;
        codeLine = docStream.readLine().trimmed();
        codeLineList = codeLine.toUpper().split(" ", QString::SkipEmptyParts);
        for(int i = 0; i<(codeLineList.size()); i++)
        {
            command = codeLineList[i].trimmed();
            if(codeLine[0] == '(')
            {
                stream << codeLine;
                break;
            }
            else if(command[0] == 'N')
            {
                //do not stream
                continue;
            }
            else
            {
                stream << command;
            }
            if(i != (codeLineList.size() - 1))
                stream << ' ';
        }
        stream << endl;
    }
    GCodeDocument->setPlainText(changeBuffer);
    err->pushError(ErrorHandler::Okay);
}

int GCodeText::getUnits()
{
    QString docString = GCodeDocument->toPlainText();
    QTextStream docStream(&docString);
    while(!docStream.atEnd())
    {
        QList<QString> codeLineList = docStream.readLine().trimmed().toUpper().split(" ", QString::SkipEmptyParts);
        for(int i = 0; i<(codeLineList.size()); i++)
        {
            if(codeLineList[i] == "G90")
                return 1;
            else if(codeLineList[i] == "G91")
                return 2;
        }
    }
    return 0;
}

int GCodeText::countLines()
{
    int count = 0;
    GCodeFile->seek(0);
    while(!GCodeFile->atEnd())
    {
        GCodeFile->readLine(100);
        count++;
    }
    return count;
}

bool GCodeText::check()
{
//    if(!GCodeDocumentSet)
//    {
//        err->pushError(ErrorHandler::GCodeFileNotOpen);
//        err->assessErrorList();
//        return;
//    }

//    QString docString = GCodeDocument->toPlainText();
//    QTextStream docStream(&docString);
//    docStream.seek(0);

//    while(!(docStream.atEnd()))
//    {
////      Supported Codes
////      G 0 1 2 3 4 17 18 19 20 21 28 53 80 90 91 93 94
////      M 0 2 3 4 5
////      T
//        if(err->checkError())
//        {
//            err->assessErrorList();
//            return;
//        }
//        bool ok = true;
//        QString codeLine;
//        QList<QString> codeLineList;
//        QString command;
//        QString numberString;
//        codeLine = docStream.readLine();
//        codeLineList = codeLine.toUpper().split(" ", QString::SkipEmptyParts);
//        for(int i = 0; i<(codeLineList.size()); i++)
//        {
//            command = codeLineList[i];
//            numberString = command.remove(' ');
//            numberString.remove(0,1);
//            if(codeLine[0] == '(')
//            {
//                command.trimmed();
//                stream << codeLine;
//                break;
//            }
//            else if(command == "G91")
//            {
//                toAbsolute();
//                offset(xOffset, yOffset, units);
//                toIncremental();
//                err->pushError(ErrorHandler::Okay);
//                continue;
//            }
//            else if(command == "G20") {
//                if(units != 0)
//                    unitMultiplier = 1.0/25.4;
//                else
//                    unitMultiplier = 1;
//                stream << command;
//            }
//            else if(command == "G21") {
//                if(units != 1)
//                    unitMultiplier = 25.4;
//                else
//                    unitMultiplier = 1;
//                stream << command;
//            }
//            else if(command[0] == 'X')
//            {
//                float coordNumber = numberString.toFloat(&ok);
//                if(!ok)
//                {
//                    err->pushError(ErrorHandler::UnexpectedSyntax);
//                    continue;
//                }
//                coordNumber += xOffset*unitMultiplier;
//                stream << 'X' << coordNumber;
//            }
//            else if(command[0] == 'Y')
//            {
//                float coordNumber = numberString.toFloat(&ok);
//                if(!ok)
//                {
//                    err->pushError(ErrorHandler::UnexpectedSyntax);
//                    continue;
//                }
//                coordNumber += yOffset*unitMultiplier;
//                stream << 'Y' << coordNumber;
//            }
//            else
//            {
//                command.trimmed();
//                stream << command;
//            }
//            if(i != (codeLineList.size() - 1))
//                stream << ' ';
//        }
//        stream << endl;
//    }
//    GCodeDocument->setPlainText(changeBuffer);
//    err->pushError(ErrorHandler::Okay);
//    err->assessErrorList();
}

bool GCodeText::fix()
{
}

