#ifndef HTTP_H
#define HTTP_H

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QByteArray>
#include <QObject>

class Http : public QObject
{
    Q_OBJECT

public:
    Http(QObject *parent = 0);
    bool get(const QString& url);
    QByteArray& data();
    QString error();

private:
    QNetworkAccessManager m_NetworkManager;
    QNetworkRequest m_Request;
    QNetworkReply* m_pReply;
    QByteArray m_Data;
    QNetworkReply::NetworkError m_Error;
    QString m_Message;
};

#endif // HTTP_H
