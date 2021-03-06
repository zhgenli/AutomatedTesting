#include "serialporttool.h"
#include "ml_file.h"

serialportTool::serialportTool(QObject* parent) : QObject(parent)
{
    serialPort = new QSerialPort();
    connect(this, &serialportTool::sendAndRecvSignals, this, &serialportTool::onSendAndRecv,
            Qt::BlockingQueuedConnection);
    connect(serialPort, SIGNAL(readyRead()), this, SLOT(receiveData()));
    m_threadID = GetCurrentThreadId();
}

serialportTool::~serialportTool()
{
    if (NULL != serialPort) {
        delete serialPort;
        serialPort = NULL;
    }
}

bool serialportTool::open(const DeviceInfo& info)
{
    suffix = QString::fromStdString(info.suffix);
    serialPort->setPortName(QString::fromStdString(info.path));

    if (serialPort->open(QIODevice::ReadWrite)) {

        serialPort->setBaudRate(info.num);
        serialPort->setParity(QSerialPort::NoParity);
        serialPort->setDataBits(QSerialPort::Data8);
        serialPort->setStopBits(QSerialPort::OneStop);
        serialPort->setFlowControl(QSerialPort::NoFlowControl);
        serialPort->clearError();
        serialPort->clear();

        return true;

    } else {

        return false;
    }
}

void serialportTool::closeDevice()
{
    if (NULL != serialPort && serialPort->isOpen()) {
        serialPort->clearError();
        serialPort->close();
        response.clear();
    }
}

void serialportTool::sendData(const QString& msg)
{
    QString command = msg + "\r\n";
    serialPort->write(command.toLocal8Bit());
}

void serialportTool::onSendAndRecv(const QString& msg, const float& timeout, const QString& suffixStr)
{
    if (!serialPort->isWritable() || !serialPort->isReadable()) {
        emit errorHappend("SerialPort status is incorrect. Read/write operations are prohibited");
        return;
    }

    QString command = msg + "\r\n";
    serialPort->write(command.toLocal8Bit());

    float temp_timeout = 0;
    QString res;
    while (temp_timeout < timeout) {

        Delay_MSec(10);
        temp_timeout = temp_timeout + 0.01;

        if (response.contains(suffixStr)) {
            qDebug() << "---1001----";
            break;
        }

        if (msg.contains("[0x1,0x82,0x41,0x3") && !msg.contains("SET_pcb[0x1,0x82,0x41,0x3,0x0,0x0,0x0,0x12]")) {
            response = "[Set motor_Alpha][OK][DONE]\r\n";
            break;
        }
    }

    if (response.contains(suffixStr)) {
        return;
    }

    response = QString("serialport %1 send: %2 failed, timeout %3").arg(serialPort->portName()).arg(msg).arg(timeout);
    emit errorHappend(response);
}

QString serialportTool::sendDataWithResponse(const QString& msg, const float& timeout, const QString& suffixStr)
{
    std::unique_lock<std::mutex> lock(_mutex);
    response.clear();
    if (GetCurrentThreadId() == m_threadID)
        onSendAndRecv(msg, timeout, suffixStr);
    else
        emit sendAndRecvSignals(msg, timeout, suffixStr);
    return response;
}

void serialportTool::clearBuffer()
{
    response.clear();
}

void serialportTool::receiveData()
{
    try {
        QByteArray returnData = serialPort->readAll();
        if (!returnData.isEmpty()) {
            response.append(returnData);
        }
    }
    catch (...) {
        emit errorHappend(QString("serialportTool error aaaaaa"));
    }
}

void serialportTool::Delay_MSec(unsigned int msec)
{
    QEventLoop loop;
    QTimer::singleShot(msec, &loop, SLOT(quit()));
    loop.exec();
}
