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
//			  else if(command[0] == 'I')
//			  {

//			  }
//			  else if(command[0] == 'J')
//			  {

//			  }
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
//			  else if(command[0] == 'I')
//			  {

//			  }
//			  else if(command[0] == 'J')
//			  {

//			  }
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

QString GCodeText::Preprocess()
{
	qDebug() << "PreProcess";
	QString gcodeString = GCodeDocument->toPlainText();
	if(!settings->PreprocessSettings().enabled)
	return gcodeString;
	gcodeString.append('\0');
	QString processedString("");
	enum{Seek, Cut, CWArc, CCWArc};
	enum{Absolute, Incremental};
	enum{Center, Radius};
	int arcMode = 9;
	int arcCenterMode = 9;
	int moveMode = 0;
	bool moveModeLast = 0;
	int coordinateMode = 9;
	int coordinateModeLast = 9;
	int unitMode = 9;
	int unitModeLast = 9;
	//look into QVector; might be more appropriate
	QList<QPolygon> shape;
	QList<QPointF> point;
	QString startCoord[3] = {0};
	QString endCoord[3] = {0};
	QString arcCenter[3] = {0};
	QString arcCenterLast[3] = {0};
	float radius = 0;
	int position = 0;

   while(gcodeString[position] != '\0')
   {
	   //G 0 1 2 3 4 17 18 19 20 21 28 30 53 80 90 91 92 93 94
	   //M 0 1 2 3 4 5 30 60
	   char ch = gcodeString[position].toAscii();
	   switch(ch)
	   {
	   case '%':
	   //ignore
	   position++;
	   break;
	   case ' ':
	   //ignore
	   position++;
	   break;
	   case 'F':case 'f':
	   position++;
	   while(gcodeString[position] >= '0' && gcodeString[position] <= '9')
		   position++;
	   break;
	   case '(':case '[':
	   while(gcodeString[position] != ')' && gcodeString[position] != ']')
		   position++;
	   break;
	   case '/':
	   while(gcodeString[position] != '\n')
		   position++;
	   break;
	   case 'G':case 'g':
	   switch(GetNumString(gcodeString, &position).toInt())
	   {
	   case 0:
		   moveMode = Seek;
		   break;
	   case 1:
		   moveMode = Cut;
		   break;
	   case 2:
		   moveMode = CWArc;
		   break;
	   case 3:
		   moveMode = CCWArc;
		   break;
	   case 4:
		   //dwell
		   break;
//	   case 17:	//irrelevant for foam cutting machine
//		   break;
//	   case 18:
//		   break;
//	   case 19:
//		   break;
	   case 20:
		   unitMode = 0;
		   break;
	   case 21:
		   unitMode = 1;
		   break;
	   case 28:
		   //go home
		   break;
	   case 30:
		   //go home
		   break;
	   case 53:
		   //absolute coordinate override
		   break;
	   case 80:
		   //cancel canned cycle
		   break;
	   case 90:
		   coordinateMode = Absolute;
		   break;
	   case 91:
		   coordinateMode = Incremental;
		   break;
	   case 92:
		   //set coordinate offset
		   break;
	   case 93:
		   //inverse feed rate on
		   break;
	   case 94:
		   //inverse feed rate off
		   break;
	   default:
		   //unsupported code
		   break;
	   }
	   break;
	   case 'M':case 'm':
	   {
	   switch(GetNumString(gcodeString, &position).toInt())
	   {
	   case 0:
		   //compulsory stop
		   break;
	   case 1:
		   //optional stop
		   break;
//	   case 2:		//irrelvant for foam cutting machine
//		   //end of program
//		   break;
//	   case 3:
//		   //spindle on CW
//		   break;
//	   case 4:
//		   //spindle on CCW
//		   break;
	   case 5:
		   //spindle off
		   break;
	   case 30:
		   //end of program (return to top)
		   break;
	   case 60:
		   //automatic pallet change?
		   break;
	   default:
		   //unsupported code
		   break;
	   }
	   }
	   break;
	   case 'X':case 'x':
	   {
	   endCoord[0] = GetNumString(gcodeString, &position);
	   }
	   break;
	   case 'Y':case 'y':
	   {
	   endCoord[1] = GetNumString(gcodeString, &position);
	   }
	   break;
	   case 'Z':case 'z':
	   {
	   endCoord[2] = GetNumString(gcodeString, &position);
	   }
	   break;
	   case 'R':case 'r':
	   {
	   arcMode = Radius;
	   radius = GetNumString(gcodeString, &position).toFloat();
	   }
	   break;
	   case 'I':case 'i':
	   {
	   arcMode = Center;
	   arcCenter[0] = GetNumString(gcodeString, &position);
	   }
	   break;
	   case 'J':case 'j':
	   {
	   arcMode = Center;
	   arcCenter[1] = GetNumString(gcodeString, &position);
	   }
	   case 'K':case'k':
	   {
//	   invalid for foam cutting machine
//	   arcMode = Center;
//	   arcCenter[2] = GetNumString(gcodeString, &position);
	   }
	   break;
	   case '\n': case '\r':
	   {
	   int changed = 0;
//				 qDebug() << "end of line";
	   while(gcodeString[position] <= ' ')
	   {
		   if(gcodeString[position] == '\0')
		   break;
		   position++;
	   }
	   if(unitMode != unitModeLast) {
		   processedString.append("G2").append(QString::number(unitMode));
		   changed = 1;
	   }
	   if(coordinateMode != coordinateModeLast) {
		   processedString.append("G9").append(QString::number(coordinateMode));
		   changed = 1;
	   }
	   if(moveMode != moveModeLast) {
		   processedString.append("G0").append(QString::number(moveMode));
		   changed = 1;
	   }
	   if(startCoord[0] != endCoord[0]) {
		   processedString.append('X').append(endCoord[0]);
		   changed = 1;
	   }
	   if(startCoord[1] != endCoord[1]) {
		   processedString.append('Y').append(endCoord[1]);
		   changed = 1;
	   }
	   if(startCoord[2] != endCoord[2]) {
		   if(!(settings->PreprocessSettings().uniformZ)) {
		   processedString.append('Z').append(endCoord[2]);
		   }
		   else {
	//	   make z changes uniform for foam cutting
		   float num = settings->PreprocessSettings().zMagnitude;
		   if(unitMode == 1)
			   num *= 25.4;
		   if(endCoord[2] > startCoord[2])
			   processedString.append('Z').append(QString::number(num));
		   else if(endCoord[2] < startCoord[2])
			   processedString.append("Z-").append(QString::number(num));
		   }
		   changed =1;
	   }
	   if(arcCenter[0] != arcCenterLast[0]) {
		   processedString.append('I').append(arcCenter[0]);
		   changed = 1;
	   }
	   if(arcCenter[1] != arcCenterLast[1]) {
		   processedString.append('J').append(arcCenter[1]);
		   changed = 1;
	   }
	   if(arcCenter[2] != arcCenterLast[2]) {
		   processedString.append('K').append(arcCenter[2]);
		   changed = 1;
	   }
	   if(changed == 1) {
	   coordinateModeLast = coordinateMode;
	   unitModeLast = unitMode;
	   moveModeLast = moveMode;
	   startCoord[0] = endCoord[0];
	   startCoord[1] = endCoord[1];
	   startCoord[2] = endCoord[2];
	   arcCenter[0] = arcCenterLast[0];
	   arcCenter[1] = arcCenterLast[1];
	   arcCenter[2] = arcCenterLast[2];
	   processedString.append('\n');
	   }
	   break;
	   }
	   default:
	   position++;
	   }

	qDebug() << gcodeString[position];
	   }
   qDebug() << "--------Processed String---------\n" << processedString << "------------------End-----------------\n";
   return processedString;
}

void GCodeText::SetSettings(Settings *s)
{
	settings = s;
}



