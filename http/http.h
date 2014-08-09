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
    Http(QObject* parent = 0);
    ~Http();
    bool get(const QString& url);
    QByteArray& data();
    QString error();
    QNetworkReply* getReply();

private:
    QNetworkAccessManager m_NetworkManager;
    QNetworkRequest m_Request;
    QNetworkReply* m_pReply;
    QByteArray m_Data;
    QNetworkReply::NetworkError m_Error;
    QString m_Message;

private slots:
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

signals:
    void downloadProgress(qint64, qint64);
};

#endif // HTTP_H
