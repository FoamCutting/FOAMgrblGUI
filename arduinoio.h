#ifndef ARDUINOIO_H
#define ARDUINOIO_H

#include <QObject>
#include <QDebug>

//need to phase these out
#include "qextserialenumerator.h"
#include "qextserialport.h"

//and replace them with these
#include "serialdeviceenumerator.h"
#include "abstractserial.h"

#include "errorhandler.h"
#include <QTimer>
#include <QProcess>
#include "inttypes.h"
#include "Toolbox.h"
#include "settings.h"

class ArduinoIO : public QObject
{
    Q_OBJECT

private:
    AbstractSerial *arduinoPort2;
    QProcess *avrdude;
    QextSerialPort *arduinoPort;
    QString arduinoPortName;
    QList<QString> dataBuffer;
    int deviceState;
    ErrorHandler *err;
    void ClearGrblSettings();

public:
    ~ArduinoIO();

    Settings::grblSettings GrblSettings;
    void SetErrorHandler(ErrorHandler*);
    QList<QextPortInfo> ports;
    bool IsReady();
    explicit ArduinoIO(QObject *parent = 0);
    QList<QextPortInfo> GetPorts();
    bool OpenPort(int = 0);
    void ClosePort();
    bool SetBaudRate(int);
    QString portName();
    friend ArduinoIO* &operator<<(ArduinoIO*, QString);
    QString getLine();
    void Flush();
    void SetGrblVersion(int);
    int NumberConnected();
    int DeviceState();
    enum{DISCONNECTED, CONNECTED, CHECKING, BUSY, READY};

    void SeekRelative(double, double, double);
    void SeekAbsolute(double, double, double);
    void RefreshSettings();

    template<typename T>
    void ChangeSetting(int setting, T value)
    {
	this << QString("$").append(QString::number(setting)).append(" = ").append(QString::number(value).append("\n"));
//	qDebug() << "Changing Setting :: " << QString("$").append(QString::number(setting)).append(" = ").append(QString::number(value)).append("\n");
    }

//    ArduinoIO operator <<(QString);
    int ChangeAllSettings(float,float,float,float,float,float,float,float,uint8_t,uint8_t);

signals:
    void ok();
    void error();
    void settingChangeSucceeded();
    void newData();
    void portReady();
    void deviceStateChanged(int);
    void getDeviceGrblSettingsFinished();

public slots:

private slots:
    void onReadyRead();
    void onDSRChanged(bool x);
    void SetDeviceState(int);
    void GetDeviceGrblSettings();
    void GetDeviceGrblSettings2();
};

#endif // ARDUINOIO_H
