#ifndef SOUND_H
#define SOUND_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>

class Sound : public QObject
{
    Q_OBJECT
public:
    explicit Sound(QObject *parent = 0);
    int start();

private:
    QTcpServer* clientInfoServer;
    QTcpServer* clientSoundServer;
    QTcpServer* regSoundServer;

    QVector<QTcpSocket*> clientInfoSockets;
    QVector<QTcpSocket*> clientSoundSockets;

    QTcpSocket* regSoundSocket;

private slots:
    void onClientInfoConnected();
    void onClientSoundConnected();
    void onRegSoundConnected();

    void onClientInfoData();
    void onClientSoundData();
};

#endif // SOUND_H
