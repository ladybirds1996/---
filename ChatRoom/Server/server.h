#ifndef SERVER_H
#define SERVER_H
//服务器主界面
#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>
#include <QList>
#include <QHash> //哈希表类
#include <QDateTime>
#include <QtNetwork>
#include <QHostAddress>

#include <QFile>
#include <QMessageBox>
#include <QTextStream>

#include <qdatastream.h>
#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QtDebug>
#include <QSqlQuery> //数据库执行类
#include "protocol.h"

// 转换中文
#define qDebug_Chinese(string) QString(QStringLiteral("%1")).arg(QString(QStringLiteral(string)));

// UI信息传递结构体
typedef struct {
    int ChatNum;
    QString TxtInfo;
}UIDATA;

namespace Ui {
    class Server;
}

// 创建第二个线程来处理消息
class Thread : public QObject {
    Q_OBJECT
public:
    Thread();
    ~Thread();

    // tcpsocket write
    bool writeFlush(QTcpSocket *s, const QByteArray &data);

    // 从hash中获得指定的ip地址
    QString getIp(const QString& chatnum);
    QString getName(QString id);
    QString getTime();

signals:
    void hashChangeSetUi();

public slots:
    // 拿到socket数据,调用执行函数开始执行socket消息处理
    void processPendingTcpData();
    // Tcpsocket连接断掉，清掉hash数据
    void clientdisconnect();

private:
    // 消息处理
    void processMsg(QTcpSocket* s,const QByteArray msg);
    // hash表存储客户端数目
    QHash<QTcpSocket*, QString> hash;
};




class Server : public QWidget {
    Q_OBJECT

public:
    explicit Server(QWidget *parent = 0);
    ~Server();
    void initServer();
    void initSql();
    void initDir();
    void CreateUserInfoTable();

//    void processClientData(QTcpSocket* s,const QByteArray data);
//    QString getName(QString id);
//    QString getTime();


    QThread* socketprocessthread;
    Thread* workthread;

signals:
    void intoMsgQueueOK();

public slots:
    void connectSlot();
    void msgIntoQueue();
    void disconnectSlot();
    void setUiInfo();

private:
    Ui::Server *ui;
    QTcpServer *server;
//    QTcpSocket *socket;
    // 这个表用来装socket
    QList<QTcpSocket*> list;
    // hash表存储客户端数目
    //<QTcpSocket*,QString> hash;
};

#endif // SERVER_H
