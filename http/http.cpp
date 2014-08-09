#include "http.h"

Http::Http(QObject* parent) :
    QObject(parent)
{
}

Http::~Http()
{
    delete m_pReply;
}

bool Http::get(const QString& url)
{
    m_Data.clear();
    m_Request.setUrl(url);
    m_pReply = m_NetworkManager.get(m_Request);

    QEventLoop loop;
    connect(m_pReply, SIGNAL(finished()), &loop, SLOT(quit()));
    connect(m_pReply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(onDownloadProgress(qint64,qint64)));
    loop.exec();

    m_Data.append(m_pReply->readAll());
    m_Error = m_pReply->error();
    m_Message = m_pReply->errorString();

    return m_Error == QNetworkReply::NoError;
}

QByteArray& Http::data()
{
    return m_Data;
}

QString Http::error()
{
    return QString("%1: %2").arg(m_Error).arg(m_Message);
}

void Http::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    emit downloadProgress(bytesReceived, bytesTotal);
}

