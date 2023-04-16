#include "vpnclient.h"

#include <QTcpSocket>
#include <QProcess>
#include <QDir>
#include <stdio.h>

const QString OpenVPNClient::DISCONNECTED = "DISCONNECTED";
const QString OpenVPNClient::CONNECTING = "CONNECTING";
const QString OpenVPNClient::CONNECTED = "CONNECTED";
const QString OpenVPNClient::RECONNECTING = "RECONNECTING";

OpenVPNClient::OpenVPNClient(QObject *parent) : QThread(parent)
{
    process = new QProcess();
    currentDir = new QString(QDir::currentPath());
    socket = new QTcpSocket();
    state = new QString(DISCONNECTED);

    connect(socket, &QTcpSocket::readyRead, this, &OpenVPNClient::onReadyRead);
    connect(this, &OpenVPNClient::startTelnetConnectAndAuthenticate, this, &OpenVPNClient::telnetConnectAndAuthenticate, Qt::QueuedConnection);
}

OpenVPNClient::~OpenVPNClient()
{
    //close connection
    write("signal SIGTERM");
}

void OpenVPNClient::run()
{
    emit startTelnetConnectAndAuthenticate();
}

void OpenVPNClient::telnetConnectAndAuthenticate()
{
    if (!telnetConnect()) {
        log("Starting VPN Client");
        process->setProgram("cmd.exe");
        process->setArguments(QStringList() << "/C" << QString("%1/openvpn.exe").arg(*currentDir)
                              << "--config" << QString("%1/openvpn.ovpn").arg(*currentDir)
                              << "--auth-nocache"
                              << "--auth-retry" << "interact"
                              << "--management" << "localhost" << "7505"
                              << "--management-query-passwords"
                              << "--management-hold");

        process->startDetached();

        int count = 3;
        while(!telnetConnect() && count > 0) {
            count--;
        }
    }

    log("Connecting to the VPN");
    if (authenticate()) {
        log("VPN Connected");
        emit connected(CONNECTED);
    } else {
        log("VPN Connection error");
        emit connected(getState());
    }
}

void OpenVPNClient::onReadyRead()
{
    QString out = socket->readAll();
    if (out.contains(DISCONNECTED)) {
        state = new QString(DISCONNECTED);
    } else if(out.contains(RECONNECTING)) {
        state = new QString(RECONNECTING);
    } else if(out.contains(CONNECTED)) {
        state = new QString(CONNECTED);
    } else if (out.contains(CONNECTING)) {
        state = new QString(CONNECTING);
    }

    log(out);
}

bool OpenVPNClient::telnetConnect()
{
    if (socket->isOpen() && socket->state() == QAbstractSocket::ConnectedState) {
        return true;
    }

    socket->connectToHost("localhost", 7505);
    return socket->waitForConnected(1000);
}

bool OpenVPNClient::authenticate()
{
    QString currentState = getState();
    if (socket->isOpen() && currentState != CONNECTED) {
        int authCount = 4;
        do {
            authCount--;

            write("hold release");
            write("username Auth vpn");
            write("password Auth e58c@OPo");

            int count = 5;
            while (getState() != CONNECTED && count > 0) {
                count--;
                QThread::sleep(2);
            }

            if (*state == RECONNECTING && authCount > 0) {
                log("Auth failed, reconnecting");
            }
        } while(*state != CONNECTED && authCount > 0);
        return *state == CONNECTED;
    }

    return currentState == CONNECTED;
}

QString OpenVPNClient::getState()
{
    write("state");
    return *state;
}

void OpenVPNClient::write(QString cmd) {
    if (socket->isOpen()) {
        socket->write(cmd.toUtf8() + "\n");
        socket->waitForReadyRead();
    } else {
        QString msg = QString("Socket is not open to write a command: '{}'").arg(cmd);
        log(msg);
    }
}
