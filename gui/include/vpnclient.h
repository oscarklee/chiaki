#ifndef VPNCLIENT_H
#define VPNCLIENT_H

#include <QString>
#include <QProcess>
#include <QTcpSocket>

class OpenVPNClient : public QObject
{
    Q_OBJECT

    public:
        static const QString DISCONNECTED;
        static const QString CONNECTING;
        static const QString CONNECTED;
        static const QString RECONNECTING;

        OpenVPNClient(QObject *parent = nullptr);
        ~OpenVPNClient();

        QString init();
        QString getState();

    private:
        void write(QString cmd);
        bool telnetConnect();
        bool authenticate();
        void onReadyRead();
        void log(QString msg) {
            emit logSignal(msg);
        }

        QProcess *process;
        QString *currentDir;
        QTcpSocket *socket;
        QString *state;
Q_SIGNALS:
        void logSignal(const QString &msg);
};

#endif // VPNCLIENT_H
