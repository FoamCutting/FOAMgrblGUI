 #include "arduinoio.h"

ArduinoIO::ArduinoIO(QObject *parent) :
    QObject(parent)
{
//    arduinoPort = new AbstractSerial();
    arduinoPort = new SerialPort();
    deviceState = DISCONNECTED;
    GrblSettings.version = -1;
    connect(this, SIGNAL(deviceStateChanged(int)), this, SLOT(SetDeviceState(int)));
}

ArduinoIO::~ArduinoIO()
{
    if(deviceState > DISCONNECTED)
	ClosePort();
}

void ArduinoIO::SetErrorHandler(ErrorHandler *handler)
{
    err = handler;
}

QStringList ArduinoIO::GetPorts()
{
    ports.clear();
    SerialPortInfo::availablePorts();

    foreach (const SerialPortInfo &info, SerialPortInfo::availablePorts()) {
        QString s(QObject::tr("Port: %1\n"
                              "Location: %2\n"
                              "Description: %3\n"
                              "Manufacturer: %4\n"
                              "Vendor Identifier: %5\n"
                              "Product Identifier: %6\n"
                              "Busy: %7\n"));

        s = s.arg(info.portName()).arg(info.systemLocation())
                .arg(info.description()).arg(info.manufacturer())
                .arg(info.vendorIdentifier()).arg(info.productIdentifier())
                .arg(info.isBusy() ? QObject::tr("Yes") : QObject::tr("No"));
        qDebug() << s;

        #ifdef Q_OS_LINUX
            if(info.portName().contains("ttyUSB") || info.portName().contains("ttyACM") ) {
                ports.append(info.systemLocation());
                }
        #endif

        #ifdef Q_OS_WIN
            //have a list of previously verified COM ports
        #endif

        #ifdef Q_OS_MAC
            //find out how Macs handle USB port naming
        #endif
            qDebug() << "Available Ports: \n" << ports;
            return ports;
    }
    qDebug() << ports;
}

bool ArduinoIO::OpenPort(int index)
{
    if(deviceState > 0)
	ClosePort();
    GetPorts();
    if(ports.isEmpty())
	return 0;
    arduinoPortName = ports.at(index);
    //arduinoPort->setDeviceName(arduinoPortName);
    arduinoPort->setPort(arduinoPortName);
    arduinoPort->setFlowControl(SerialPort::NoFlowControl);
    arduinoPort->setParity(SerialPort::NoParity);
    arduinoPort->setDataBits(SerialPort::Data8);
    arduinoPort->setStopBits(SerialPort::OneStop);
    arduinoPort->setRate(SerialPort::Rate9600);

    disconnect(arduinoPort, SIGNAL(exception()), this, SLOT(SerialException()));
    connect(arduinoPort, SIGNAL(exception()), this, SLOT(SerialException()));
    arduinoPort->flush();
    arduinoPort->reset();
    deviceState = arduinoPort->open(QIODevice::ReadWrite);
    if(deviceState) {
	arduinoPort->setDtr(true);
	arduinoPort->setRts(true);
//	arduinoPort->setDtr(false);
//	arduinoPort->setRts(false);
	qDebug() << "Arduino Port Opened!";
	emit deviceStateChanged(CONNECTED);
       // flush();
//	QTimer::singleShot(2000, this, SLOT(GetDeviceGrblSettings()));
	connect(arduinoPort, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
	QTimer::singleShot(2000, this, SLOT(GetDeviceGrblSettings()));
    }
    qDebug() << "OpenPort finished";
    return deviceState;
}

void ArduinoIO::ClosePort()
{
    disconnect(arduinoPort, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    Flush();
    ClearGrblSettings();
    arduinoPort->close();
    emit deviceStateChanged(DISCONNECTED);
    qDebug() << "Port Closed";
}

void ArduinoIO::GetDeviceGrblSettings()
{
    qDebug() << "GetDeviceGrblSettings1";

//    if(GrblSettings.version == -1) {
//	qDebug() << "Settings get aborted";
//	QTimer::singleShot(500, this, SLOT(GetDeviceGrblSettings()));
//	return;
//    }
    if(DeviceState() == READY || DeviceState() == CHECKING || DeviceState() == CONNECTED) {
	emit deviceStateChanged(BUSY);
	Flush();
//        this << QString("\r\n\r\n");     //this was causing issues in 0.7d; is it really necessary?
//	this << QString("$\n");
	arduinoPort->write(QByteArray("$\n"));
	disconnect(this, SIGNAL(deviceStateChanged(int)), this, SLOT(GetDeviceGrblSettings()));
	connect(this, SIGNAL(ok()), this, SLOT(GetDeviceGrblSettings2()));
	return;
     }
     else if(DeviceState() > ArduinoIO::DISCONNECTED) {
	 connect(this, SIGNAL(deviceStateChanged(int)), this, SLOT(GetDeviceGrblSettings()));
	 return;
     }
     else {
	 err->pushError(ErrorHandler::NoArduinoConnected);
	 return;
     }
}

void ArduinoIO::GetDeviceGrblSettings2()
{
    qDebug() << "GetDeviceGrblSettings2";
    bool shortSettings = 0;
    QString line = getLine();
    int escape = 0;
    disconnect(this, SIGNAL(ok()), this, SLOT(GetDeviceGrblSettings2()));

    while(line != "ok")
    {
	qDebug() << line << " -- from GDGS2";
	if(line.startsWith('$'))
	{
	    if(line.length() > 1) {
		switch(line.at(1).digitValue())
		{
		case 0:
    //                qDebug() << line;
		    GrblSettings.stepsX = line.remove(line.indexOf('('), 50).remove(0,5).toFloat();
		    break;
		case 1:
    //                qDebug() << line;
		    GrblSettings.stepsY = line.remove(line.indexOf('('), 50).remove(0,5).toFloat();
		    break;
		case 2:
    //                qDebug() << line;
		    GrblSettings.stepsZ = line.remove(line.indexOf('('), 50).remove(0,5).toFloat();
		    break;
		case 3:
    //                qDebug() << line;
		    GrblSettings.stepPulse = line.remove(line.indexOf('('), 50).remove(0,5).toFloat();
		    break;
		case 4:
    //                qDebug() << line;
		    GrblSettings.feedRate = line.remove(line.indexOf('('), 50).remove(0,5).toFloat();
		    break;
		case 5:
    //                qDebug() << line;
		    GrblSettings.seekRate = line.remove(line.indexOf('('), 50).remove(0,5).toFloat();
		    break;
		case 6:
    //                qDebug() << line;
		    GrblSettings.arcSegment = line.remove(line.indexOf('('), 50).remove(0,5).toFloat();
		    break;
		case 7:
    //                qDebug() << line;
		    GrblSettings.stepInvert = line.remove(line.indexOf('('), 50).remove(0,5).toFloat();
		    if(shortSettings) {
			GrblSettings.version = 0;
			qDebug() << "Grbl version 0.51";
			emit deviceStateChanged(READY);
			emit getDeviceGrblSettingsFinished();
			qDebug() << "GetDeviceGrblSettings2End";
			return;
		    }
		    break;
		case 8:
    //                qDebug() << line;
		    GrblSettings.acceleration = line.remove(line.indexOf('('), 50).remove(0,5).toFloat();
		    break;
		case 9:
    //                qDebug() << line;
		    if(line.contains("(max instant cornering speed change in delta mm/min)")) {
			GrblSettings.version = 2;
			qDebug() << "Grbl version 0.6b";
		    }
		    else
			qDebug() <<"Grbl version 0.7d or 0.8a -- " << GrblSettings.version; //need to figure out how to differentiate between these versions
		    GrblSettings.cornering = line.remove(line.indexOf('('), 75).remove(0,5).toFloat();
		    emit deviceStateChanged(READY);
		    emit getDeviceGrblSettingsFinished();
		    qDebug() << "GetDeviceGrblSettings2End";
		    return;
		}
	    }
	    else
		shortSettings = 1;
	}
	line = getLine();
	if(escape > 100) {
	    qDebug() << "Failed to retrieve settings from " << arduinoPortName;
	    break;
	}

	escape++;
//        qDebug() << escape;
    }
//    qDebug() << deviceGrblSettings.stepsX;
//    qDebug() << deviceGrblSettings.stepsY;
//    qDebug() << deviceGrblSettings.stepsZ;
//    qDebug() << deviceGrblSettings.stepPulse;
//    qDebug() << deviceGrblSettings.feedRate;
//    qDebug() << deviceGrblSettings.seekRate;
//    qDebug() << deviceGrblSettings.arcSegment;
//    qDebug() << deviceGrblSettings.stepInvert;
//    qDebug() << deviceGrblSettings.acceleration;
//    qDebug() << deviceGrblSettings.cornering;
}

int ArduinoIO::NumberConnected()
{
    return ports.count();
}

int ArduinoIO::DeviceState()
{
    return deviceState;
}

void ArduinoIO::SetDeviceState(int state)
{
    deviceState = state;
    qDebug() << "Arduino state is:" << state;
}

bool ArduinoIO::SetBaudRate(int baud)
{
    switch(baud)
    {
    case 9600:
    arduinoPort->setRate(SerialPort::Rate9600);
        return 1;
    default:
        return 0;
    }
}

QString ArduinoIO::portName()
{
    return arduinoPortName;
}

QString ArduinoIO::getLine()
{
    if(!(dataBuffer.isEmpty()))
        return dataBuffer.takeFirst();
    else return QString("No data or Arduino disconnected");
}

void ArduinoIO::Flush()
{
    arduinoPort->flush();
    dataBuffer.clear();
//    this << QString("\n");
}

void ArduinoIO::onReadyRead()
{
    static int count = 0;
//    emit deviceStateChanged(BUSY);
    if(!arduinoPort->canReadLine())  {
//	qDebug() << "PEEK: " << arduinoPort->peek(64);
	return;
    }
    QString data = arduinoPort->readLine().trimmed();
    qDebug() << ">>" << data;
    if(data == "") {	    //this isn't quite right: will disconnect after 100, regardless of time interval
	count++;
	if(count == 100) {
	    qDebug() << "Arduino Disconnected: Closing Port";
	    ClosePort();
	}
	return;
    }
    else if(data == "Stored new setting") {
	emit settingChangeSucceeded();
	count = 0;
	return;
    }
    else if(data == "ok")
    {
        emit ok();
    }
    else if(QString(data).remove(6,100) == "error:")
    {
        emit error();
        //pass to errorhandler
    }
    else if(data[0] == 'G' && data[1] == 'r' && data[2] == 'b' && data[3] == 'l' && data[4] == ' ') {
        QString version = data.remove(0,5);
	if(version == "0.8a")
	    GrblSettings.version = 3;
        if(version == "0.7d")
	    GrblSettings.version = 2;
        else if(version == "0.6b")
	    GrblSettings.version = 1;
        else if(version == "0.51")
	    GrblSettings.version = 0;
    	count = 0;
        return;
    }
    else if(data == "'$' to dump current settings") {
	emit deviceStateChanged(CHECKING);
	GetDeviceGrblSettings();
	count = 0;
	return;
    }
    count = 0;
    dataBuffer << data;
    emit newData();
}

void ArduinoIO::SeekRelative(double dX, double dY, double dZ)
{
    QString command;
    command.append("G20").append(" G00").append(" X").append(QString::number(dX)).append(" Y").append(QString::number(dY)).append(" Z").append(QString::number(dZ)).append("\r\n");
    qDebug() << command;
    this << QString("G91");
    this << command;
}

void ArduinoIO::SeekAbsolute(double dX, double dY, double dZ)
{
    QString command;
    command.append("G00").append(" X").append(QString::number(dX)).append(" Y").append(QString::number(dY)).append(" Z").append(QString::number(dZ)).append("\r\n");
    this << QString("G90");
    this << command;
}

ArduinoIO* &operator <<(ArduinoIO *output, const QString string) //could probably put this in the class.Too tired to think about it.
{
    output->arduinoPort->write(string.toAscii());
    qDebug() << "<< " << string.trimmed();
    return output;
}


void ArduinoIO::ClearGrblSettings()
{
    GrblSettings.version = -1;
    GrblSettings.stepsX = -1;
    GrblSettings.stepsY = -1;
    GrblSettings.stepsZ = -1;
    GrblSettings.stepPulse = 0;
    GrblSettings.feedRate = -1;
    GrblSettings.seekRate = -1;
    GrblSettings.arcSegment = -1;
    GrblSettings.stepInvert = 0;
    GrblSettings.acceleration = -1;
    GrblSettings.cornering = -1;
}

int ArduinoIO::ChangeAllSettings(float, float, float, float, float, float, float, float, uint8_t, uint8_t)
{
    QObject::connect(this, SIGNAL(ok()), this, SLOT());
    return -1;
}

bool ArduinoIO::IsReady()
{
    if(deviceState == READY)
	return 1;
    else
	return 0;
}

void ArduinoIO::RefreshSettings()
{
    GetDeviceGrblSettings();
}

void ArduinoIO::SerialException()
{
    qDebug() << "SERIAL ERROR!!";
}
