#include "sound.h"

Sound::Sound(QObject *parent) : QObject(parent)
{
}

int Sound::start()
{
    int low  = 1024;
    int high = 65535;
    int port = qrand() % ((high + 1) - low) + low;

    clientInfoServer  = new QTcpServer(this);
    clientSoundServer = new QTcpServer(this);
    regSoundServer    = new QTcpServer(this);

    connect(clientInfoServer, SIGNAL(newConnection()), this, SLOT(onClientInfoConnected()));
    connect(clientSoundServer, SIGNAL(newConnection()), this, SLOT(onClientSoundConnected()));
    connect(regSoundServer, SIGNAL(newConnection()), this, SLOT(onRegSoundConnected()));

    clientInfoServer->listen(QHostAddress::Any, port);
    clientSoundServer->listen(QHostAddress::Any, port + 1);
    regSoundServer->listen(QHostAddress::Any, port + 2);

    return port;
}

void Sound::onClientInfoConnected()
{
    QTcpSocket* client = clientInfoServer->nextPendingConnection();
    connect(client, SIGNAL(readyRead()), this, SLOT(onClientInfoData()));

    clientInfoSockets.push_back(client);

    QString answer("{ \"_msg_id\" : \"COMPONENTS_LIST\", \"components\" : [ { \"activated\" : true, \"name\" : \"all\", \"optional\" : true }, { \"activated\" : true, \"name\" : \"base\", \"optional\" : false }, { \"activated\" : true, \"name\" : \"fr\", \"optional\" : true }, { \"activated\" : true, \"name\" : \"subscribed\", \"optional\" : true }, { \"activated\" : true, \"name\" : \"admin\", \"optional\" : true }, { \"activated\" : true, \"name\" : \"dofusjapan\", \"optional\" : true }, { \"activated\" : false, \"name\" : \"de\", \"optional\" : true }, { \"activated\" : false, \"name\" : \"en\", \"optional\" : true }, { \"activated\" : false, \"name\" : \"es\", \"optional\" : true }, { \"activated\" : false, \"name\" : \"it\", \"optional\" : true }, { \"activated\" : false, \"name\" : \"ja\", \"optional\" : true }, { \"activated\" : false, \"name\" : \"nl\", \"optional\" : true }, { \"activated\" : false, \"name\" : \"pt\", \"optional\" : true }, { \"activated\" : false, \"name\" : \"ru\", \"optional\" : true } ] }");
    client->write(answer.toLocal8Bit());
}

void Sound::onClientSoundConnected()
{
    QTcpSocket* client = clientSoundServer->nextPendingConnection();
    connect(client, SIGNAL(readyRead()), this, SLOT(onClientSoundData()));

    clientSoundSockets.push_back(client);
}

void Sound::onRegSoundConnected()
{
    regSoundSocket = regSoundServer->nextPendingConnection();
}

void Sound::onClientInfoData()
{
    int index = clientInfoSockets.indexOf(qobject_cast<QTcpSocket*>(sender()));

    if(index < 0)
    {
        return;
    }

    QTcpSocket* client = clientInfoSockets.at(index);
    QByteArray data = client->readAll();
}

void Sound::onClientSoundData()
{
    int index = clientSoundSockets.indexOf(qobject_cast<QTcpSocket*>(sender()));

    if(index < 0)
    {
        return;
    }

    QTcpSocket* client = clientSoundSockets.at(index);
    QByteArray data = client->readAll();

    regSoundSocket->write(data);
}
