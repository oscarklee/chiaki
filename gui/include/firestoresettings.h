#ifndef FIRESTORESETTINGS_H
#define FIRESTORESETTINGS_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

class FirestoreSettings : public QObject
{
    Q_OBJECT

public:
    explicit FirestoreSettings(QObject *parent = nullptr);
    void setValue(const QString &key, const QVariant &value);
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;

    int beginReadArray(const QString &prefix);
    void beginWriteArray(const QString &prefix);
    void setArrayIndex(int i);
    QStringList allKeys() const;
    void remove(const QString &key);
    void endArray();
    bool contains(const QString &key) const;
    void startAuthenticator();

private:
    QString password;
    QString email;
    QString projectId;
    QString apiKey;
    QString token;
    QString refreshToken;
    QNetworkAccessManager *networkManager;
    QString currentArrayPrefix;
    int currentArrayIndex;
    QVariantList currentList;

    QJsonObject valueToFirestoreObject(const QVariant &value) const;
    QVariant firestoreObjectToValue(const QJsonObject &object) const;
    QUrl buildUrl(QString &key) const;
};

#endif // FIRESTORESETTINGS_H
