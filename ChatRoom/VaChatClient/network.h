#ifndef NETWORK_H
#define NETWORK_H

//网络类....单利类
#include <QObject>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QSettings>
#include <qdatastream.h>
#include <QTextCodec>


class Network : public QObject
{
    Q_OBJECT
public:
    // 单例模式
    static Network* getNetPtr();
    void initNet();
    void saveSetting();
    void readSetting();
    //
    void sendData(QString data);
    void sendtoData(QString chatnum, QString data);
    QString getServerIp();


signals:
    void receiveData(QString str);
    void udpreciveData(QString datagram);

public slots:
    void readSlot();
    void udpReadSlot();

private:
    explicit Network(QObject *parent = 0);
    ~Network();

    QUdpSocket *udpsocket;
    quint16 udpport;
    QTcpSocket *socket;
    QString serverip;
    int serverport;

};

#endif // NETWORK_H
