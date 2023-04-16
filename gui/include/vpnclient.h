#ifndef VPNCLIENT_H
#define VPNCLIENT_H

#include <QString>
#include <QProcess>
#include <QTcpSocket>
#include <QThread>

class OpenVPNClient : public QThread
{
    Q_OBJECT

    public:
        static const QString DISCONNECTED;
        static const QString CONNECTING;
        static const QString CONNECTED;
        static const QString RECONNECTING;

        explicit OpenVPNClient(QObject *parent = nullptr);
        ~OpenVPNClient();

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

        void telnetConnectAndAuthenticate();
protected:
        void run() override;

Q_SIGNALS:
        void logSignal(const QString &msg);
        void connected(const QString &state);
        void startTelnetConnectAndAuthenticate();
};

#endif // VPNCLIENT_H
