#include <QCoreApplication>
#include <QDebug>
#include <QProcess>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Device>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/WiredDevice>

void myMessageHandler(QtMsgType type,const QMessageLogContext &context,const QString &msg)
{
    const QString homeFolder = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QFile outFile(homeFolder + "/" + "gwapplog.txt");

    if (outFile.exists()) {
        qint64 sizeInByte = outFile.size();
        if (sizeInByte > 10485760) {
            outFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
        } else {
            outFile.open(QIODevice::WriteOnly | QIODevice::Append);
        }
    } else {
         outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    }


    QTextStream ts(&outFile);
    QString txt;
    switch (type) {
    case QtInfoMsg:
        txt = QString("Info: %1 %2 %3:").arg(context.file).arg(context.line).arg(context.function);
        ts << txt <<"\t"<< QString(msg)<<endl;
        break;
    case QtDebugMsg:
        txt = QString("Debug: %1 %2 %3:").arg(context.file).arg(context.line).arg(context.function);
        ts << txt <<"\t"<< QString(msg)<<endl;
        break;
    case QtWarningMsg:
        txt = QString("Warning: %1 %2 %3:").arg(context.file).arg(context.line).arg(context.function);
        ts << txt <<"\t"<< QString(msg)<<endl;
    break;
    case QtCriticalMsg:
        txt = QString("Critical: %1 %2 %3:").arg(context.file).arg(context.line).arg(context.function);
        ts << txt <<"\t"<< QString(msg)<<endl;
    break;
    case QtFatalMsg:
        txt = QString("Fatal: %1 %2 %3:").arg(context.file).arg(context.line).arg(context.function);
        ts << txt <<"\t"<< QString(msg)<<endl;
        abort();
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QProcess m_Process;
#ifdef LOG_TO_FILE
    qInstallMessageHandler(myMessageHandler);
#endif
    NetworkManager::Device::List deviceList = NetworkManager::networkInterfaces();
    NetworkManager::WirelessDevice::Ptr wifiDevice;

    a.connect(&m_Process, &QProcess::readyReadStandardOutput, [&m_Process] (){
        qDebug() << m_Process.readAllStandardOutput();
    });

    a.connect(&m_Process, &QProcess::readyReadStandardError, [&m_Process] (){
        qDebug() << m_Process.readAllStandardError();
    });

    a.connect(&m_Process, &QProcess::stateChanged, [] (QProcess::ProcessState newState) {
        qDebug() << "Process Changed State: " << newState;
    });

    a.connect(NetworkManager::notifier(), &NetworkManager::Notifier::primaryConnectionChanged, [&m_Process] (QString uni) {
        qDebug() << "primaryConnectionChanged: " << uni;
        qDebug() << "Restart Packet Forder...";
        QString program = "systemctl";
        QStringList arguments;
        arguments << "restart" << "packetfw";
        m_Process.start(program, arguments);
        //Call Restart
    });

    return a.exec();
}
