#include <firestoresettings.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QUrlQuery>
#include <QNetworkRequest>

FirestoreSettings::FirestoreSettings(QObject *parent)
    : QObject(parent), networkManager(new QNetworkAccessManager(this))
{
    projectId = "chiaki-383411";
    apiKey = "AIzaSyC0MFBWDKB9vzGa0W7C6UcC-9E3szUxbGs";
    email = "user1@chiaki.com";
    password = "scdBN2b8TC4Q6j";
}

void FirestoreSettings::setValue(const QString &key, const QVariant &value)
{

    QStringList parts = key.split("/");
    if (parts.size() < 1) {
        return;
    }

    QString targetParent;
    QString targetKey;
    if (parts.size() == 1) {
        if (currentArrayPrefix != "") {
            targetParent = QString("%1/%2").arg(currentArrayPrefix, QString::number(currentArrayIndex));
            targetKey = key;
        } else {
            targetParent = "";
            targetKey = key;
        }
    }

    if (parts.size() == 2) {
        targetParent = parts.at(0);
        targetKey = parts.at(1);
    }

    QUrl url = buildUrl(targetParent);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QString convertedValue = value.toString();
    if (value.metaType().id() == QMetaType::QByteArray) {
        QByteArray base64Encoded = value.toByteArray().toBase64();
        QString valueEncoded = QString::fromUtf8(base64Encoded);
        convertedValue = QString("@ByteArray::%1").arg(valueEncoded);
    }

    QJsonObject json =  QJsonObject { { targetKey, convertedValue } };
    QByteArray data = QJsonDocument(json).toJson();

    QEventLoop loop;
    QNetworkReply *reply = networkManager->sendCustomRequest(request, "PATCH", data);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::errorOccurred, &loop, &QEventLoop::quit);
    loop.exec();
}

QVariant FirestoreSettings::value(const QString &key, const QVariant &defaultValue) const
{
    if (!currentList.empty()) {
        if (currentList.size() > currentArrayIndex) {
            QMap map = currentList[currentArrayIndex].toMap();
            if (map.contains(key)) {
                QString value = map.value(key).toString();
                if (value.startsWith("@ByteArray::")) {
                    QString base64String = value.mid(12);
                    QByteArray base64Encoded = QByteArray::fromStdString(base64String.toStdString());
                    QByteArray byteArray = QByteArray::fromBase64(base64Encoded);
                    return byteArray;
                }

                return QVariant(value);
            }
        }

        return defaultValue;
    }

    QStringList parts = key.split("/");
    QString targetParent = parts.at(0);
    QUrl url = buildUrl(targetParent);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QEventLoop loop;
    QNetworkReply *reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::errorOccurred, &loop, &QEventLoop::quit);
    loop.exec();

    QByteArray response = reply->readAll();
    QJsonDocument json = QJsonDocument::fromJson(response);
    if (json.isObject()) {
        QJsonObject fields = json.object();
        QString targetKey = parts.at(1);
        if (fields.contains(targetKey)) {
            return fields[targetKey].toVariant();
        }
    }

    if (json.isArray()) {
        QJsonArray array = json.array();
        return array.toVariantList();
    }

    return defaultValue;
}

int FirestoreSettings::beginReadArray(const QString &prefix)
{
    currentArrayPrefix = prefix;
    currentArrayIndex = 0;
    currentList = value(prefix).toList();
    return currentList.size();
}

void FirestoreSettings::beginWriteArray(const QString &prefix)
{
    currentArrayPrefix = prefix;
    currentArrayIndex = 0;
    currentList = value(prefix).toList();
}

void FirestoreSettings::setArrayIndex(int i)
{
    currentArrayIndex = i;
}

QStringList FirestoreSettings::allKeys() const
{
    // todo: implement
    return QStringList();
}

void FirestoreSettings::remove(const QString &key)
{
    // todo implement
}

void FirestoreSettings::endArray()
{
    currentList.clear();
    currentArrayPrefix.clear();
    currentArrayIndex = -1;
}

bool FirestoreSettings::contains(const QString &key) const
{
    // todo implement
    return false;
}

QJsonObject FirestoreSettings::valueToFirestoreObject(const QVariant &value) const
{
    // Convierte un QVariant a un objeto Firestore compatible (solo admite tipos básicos)
    // ...
}

QVariant FirestoreSettings::firestoreObjectToValue(const QJsonObject &object) const
{
    // Convierte un objeto Firestore a QVariant (solo admite tipos básicos)
    // ...
}

QUrl FirestoreSettings::buildUrl(QString &key) const
{
    QString urlStr = QString("https://%1-default-rtdb.firebaseio.com/%2.json?auth=%3").arg(projectId, key, token);
    QUrl url = QUrl(urlStr);
    return url;
}

void FirestoreSettings::startAuthenticator()
{
    QString urlStr = QString("https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword?key=%1").arg(apiKey);
    QUrl url = QUrl(urlStr);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["email"] = email;
    json["password"] = password;
    json["returnSecureToken"] = true;
    QByteArray data = QJsonDocument(json).toJson();

    QEventLoop loop;
    QNetworkReply *reply = networkManager->post(request, data);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::errorOccurred, &loop, &QEventLoop::quit);

    loop.exec();

    QByteArray response = reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(response);
    QJsonObject fields = jsonResponse.object();
    if (fields.contains("refreshToken")) {
        refreshToken = fields["refreshToken"].toString();
    }
    if (fields.contains("idToken")) {
        token = fields["idToken"].toString();
    }
}
