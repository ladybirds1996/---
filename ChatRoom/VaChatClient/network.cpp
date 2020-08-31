#include "network.h"
#include <QDebug>

//得到网络指针
Network* Network::getNetPtr() {
    //这里是在程序初始化的时候就运行
    //存储数据区
    static Network net;
    //返回对象的指针
    return &net;
}

void Network::initNet() {
    // 读取配置信息
    saveSetting();
    readSetting();
    socket = NULL;
    // 和服务器连接socket建立
    if(socket == NULL) {
        socket = new QTcpSocket(this);
    }
    if(udpsocket == NULL) {
        // 启动UDP套接字，用于用户之间的P2P通信
        udpsocket = new QUdpSocket();
        udpport = 55520;
    }
    // 绑定服务器socket，和服务器建立连接
    socket->connectToHost(serverip,serverport);
    // 连接成功waitForConnected()就会返回true
    if(!socket->waitForConnected(3000)) {
        qDebug()<<"网络未连接";
    }
    // 建立槽函数，如果收到服务器消息，就读取消息
    connect(socket,SIGNAL(readyRead()),this,SLOT(readSlot()));

    // UDP配置绑定,接收任何IPv4地址的 `55520`端口的信息
    udpsocket->bind(QHostAddress::AnyIPv4,udpport);
    connect(udpsocket,SIGNAL(readyRead()),this,SLOT(udpReadSlot()));
}

//保存配置文件
void Network::saveSetting() {
    QSettings set("./net.ini",QSettings::IniFormat);
    set.beginGroup("net");
    // 设置为本地IP
    //set.setValue("ip","127.0.0.1");
    set.setValue("ip","172.33.22.195");
    set.setValue("port",10086);
    set.endGroup();
    qDebug() << "net set over!";
}

//读取配置文件
void Network::readSetting() {
    // set为QT的iniformat对象，直接拿到了ini文件句柄
    QSettings set("./net.ini",QSettings::IniFormat);
    serverip = set.value("net/ip").toString();
    serverport = set.value("net/port").toInt();
}

// 返回服务器的ip
QString Network::getServerIp() {
    return serverip;
}

//向服务器发送数据（接口函数）
void Network::sendData(QString data) {
    QByteArray out_buf;
    QDataStream out(&out_buf, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_9);

    out << (qint64) 0;
    // 把数据加上去
    out_buf += data.toStdString().data();
    // 数据传入后，重新定位到头部记录总体长度
    out.device()->seek(0);
    out << (qint64) (out_buf.size() - sizeof(qint64));
    if(socket != NULL) {
        // 发送消息
        qDebug() << "out_buf:" << out_buf;
        socket->write(out_buf);
        socket->flush();
    }
}

//给指定用户发送信息
void Network::sendtoData(QString recip, QString data) {
    qDebug() << "recip" << recip << "data" << data;
    // 给对方发消息
    udpsocket->writeDatagram(data.toUtf8(), (QHostAddress)recip, udpport);
}

//读取服务器数据
void Network::readSlot() {
    qint64 datasize = 0;
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_9);
    if(datasize == 0){
        // 如果接收到的数据还没到数据头大小，先不传给datasize
        if(socket->bytesAvailable() < (int)sizeof(qint64))
            return;
        in >> datasize;
    }
    // 如果还没有接收到指定的数据大小就先返回等他传完
    while(socket->bytesAvailable() < datasize);

    //信息容器
    QByteArray data;
    //in >> data;
    while(socket->bytesAvailable()) {
        data.append(socket->readAll());
    }
    qDebug() << "receive SERVER data:" << data;
    // 发送消息接收完毕的信号
    emit receiveData(data);
}

void Network::udpReadSlot() {
    //拥有等待的数据报
    while(udpsocket->hasPendingDatagrams()) {
        //拥于存放接收的数据报
        QByteArray datagram;
        //放udp的IP地址
        QHostAddress targetaddr;
        //让datagram的大小为等待处理的数据报的大小，这样才能接收到完整的数据
        datagram.resize(udpsocket->pendingDatagramSize());
        //接收数据报，将其存放到datagram中
        udpsocket->readDatagram(datagram.data(),datagram.size(),&targetaddr);
        qDebug() << "udp targetaddr:" << targetaddr;
        qDebug() << "udpdata:" << datagram;


        //将数据报内容给mainchat
        //ui->label->setText(datagram);
        emit udpreciveData(datagram);
    }
}


Network::Network(QObject *parent) : QObject(parent) {
    // 初始化网络建立
    initNet();
}

Network::~Network() {

}
