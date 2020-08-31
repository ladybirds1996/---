#include "server.h"
#include "ui_server.h"
// 消息队列
QQueue<QTcpSocket*> MsgQueue;

// 全局的UIDATA传递容器
static UIDATA uidata;


Thread::Thread() {
    qDebug() << "current thread id :" << QThread::currentThreadId();
}

Thread::~Thread(){

}


// 发送消息就刷新缓存
bool Thread::writeFlush(QTcpSocket *s, const QByteArray &data) {
    QByteArray out_buf;
    QDataStream out(&out_buf, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_9);

    out << (qint64) 0;
    // 把数据加上去
    out_buf += data;
    // 数据传入后，重新定位到头部记录总体长度
    out.device()->seek(0);
    // 写入数据大小
    out << (qint64)(out_buf.size() - sizeof(qint64));

    qDebug() << "out_buf:" << out_buf;
    s->write(out_buf);
    return s->flush();
}

//通过id找name
QString Thread::getName(QString id) {
    QSqlQuery query;
    QString cmd = tr("select name from user_info where chatnum = '%1';").arg(id);
    query.exec(cmd);
    query.next();
    return query.value(0).toString();
}

//获取时间戳
QString Thread::getTime() {
    QString str = QDateTime::currentDateTime().toString("yy-MM-dd hh:mm:ss");
    return str;
}

QString Thread::getIp(const QString &chatnum) {
    // 从hash表中找到socket的IP地址
    return hash.key(chatnum)->peerAddress().toString();
}

void Thread::processMsg(QTcpSocket *s,const QByteArray data) {
    // 协议对象，用于解析数据
    Protocol pro;
    //contains 检测指定字符段
    // 如果是注册信息
    if(data.contains("<register>")) {
        QByteArray temp = data;
        QString chatnum,passwd,name,gender,null;
        // 调用协议解析函数
        pro.parseData(temp,chatnum,passwd,name,gender,null);
        qDebug()<<"chatnum: "<< chatnum <<"passwd: "<< passwd <<"name: "<< name <<"gender: "<< gender;
        // 数据比对
        QSqlQuery query;
        // 试着查找这个注册用户的信息
        QString cmd = tr("select * from user_info where chatnum = '%1' ").arg(chatnum);
        // 如果是已经存在的用户信息，就发送不要重复注册，如果是新用户，就添加到用户信息表里面，发送注册
        if(query.exec(cmd)) {
            // 如果有当前的chatnum，表示重复注册
            if(query.next()) {
                qDebug()<< qDebug_Chinese("重复注册");
                QString head = "<register-have>";
                QByteArray res;
                res.append(pro.packData(head));
                writeFlush(s, res);
                return;
            } else {
                //插入数据库 user_info，进行注册
                query.clear();
                QString cmd = tr("insert into user_info values('%1','%2','%3','%4');").arg(chatnum).arg(passwd).arg(name).arg(gender);
                if(query.exec(cmd)) {
                    //注册成功
                    qDebug()<< qDebug_Chinese("注册成功。。");
                    QString head = "<register-succ>";
                    QByteArray res;
                    res.append(pro.packData(head));
                    writeFlush(s,res);
                } else {
                    // 注册失败
                    qDebug()<< qDebug_Chinese("注册失败。。");
                    QString head = "<register-error>";
                    QByteArray res;
                    res.append(pro.packData(head));
                    writeFlush(s,res);
                }
            }
        }else {
            // sql出错
            qDebug()<< qDebug_Chinese("注册失败。。");
            QString head = "<register-error>";
            QByteArray res;
            res.append(pro.packData(head));
            writeFlush(s,res);
        }
        return;
    }
    // 如果是登陆信息
    else if(data.contains("<login>")) {
        //登录数据
        QByteArray temp = data;
        QString chatnum, passwd, null;
        // 拿到登陆的用户，及输入的密码等
        pro.parseData(temp, chatnum, passwd, null, null, null);
        qDebug()<<"chatnum:"<< chatnum<< "passwd:"<< passwd;
        // 信息验证
        QSqlQuery query;
        // 从数据库中拿到登陆用户的注册密码
        QString cmd = tr("select password from user_info where chatnum = '%1';").arg(chatnum);
        if(query.exec(cmd)) {
            // 如果有这个用户，进行数据匹配
            if(query.next()) {
                if(query.value(0).toString() == passwd) {
                    //避免重复登录
                    if(hash.contains(hash.key(chatnum))) {
                        //重复登录
                        QString head = "<login-have>";
                        QByteArray res;
                        res.append(pro.packData(head));
                        writeFlush(s, res);
                        return;
                    }
                    //登录成功,构建消息容器
                    QByteArray str;
                    str.append(pro.packData("<login-succ>"));

                    //往hash中插入数据,记录socket-chatnum
                    hash.insert(s,chatnum);
                    QSqlQuery query;
                    // 获取该用户的所有好友信息，群信息，群的客户信息反馈给当前用户
                    query.clear();
                    QString getInfoCmd = tr("select id_friend from friend where id_my = '%1';").arg(chatnum);
                    // 拿到所有的好友
                    QByteArray namelist;
                    namelist.append("<namelist-all>");
                    namelist.append("$");
                    // 先将自己添加进去
                    namelist.append(getName(chatnum));
                    namelist.append("(");
                    namelist.append(chatnum + "-online");
                    namelist.append(")");
                    namelist.append("@");
                    // 在线输入对应的ip地址
                    namelist.append(getIp(chatnum));
                    namelist.append("$");
                    if(query.exec(getInfoCmd)) {
                        // 将所有好友拿到后传给客户端，同时标注状态
                        while(query.next()) {
                            //打包信息
                            namelist.append(getName(query.value(0).toString()));
                            namelist.append("(");
                            namelist.append(query.value(0).toString());
                            // 如果这个chatnum在在线hash表中存在，就说明是在线用户
                            if(hash.contains(hash.key(query.value(0).toString()))) {
                                namelist.append("-online");
                                namelist.append(")");
                                namelist.append("@");
                                // 在线输入对应的ip地址
                                namelist.append(getIp(query.value(0).toString()));

                                // 同时发送当前用户的上线信息和IP地址给在线好友
                                QByteArray nameid;
                                nameid.append("<nameid-y>");
                                nameid.append("$");
                                nameid.append(getName(chatnum));
                                nameid.append("(");
                                nameid.append(chatnum + "-online");
                                nameid.append(")");
                                nameid.append("@");
                                nameid.append(getIp(chatnum));
                                nameid.append("$");
                                writeFlush(hash.key(query.value(0).toString()), nameid);
                            } else {
                                namelist.append("-notconc");
                                namelist.append(")");
                                namelist.append("@");
                                // 非在线输入不能连接
                                namelist.append("null");
                            }
                            namelist.append("$");
                        }
                    }
                    qDebug() << "namelist:" << namelist;
                    str += namelist;
                    //发送给自己
                    writeFlush(s, str);
                    uidata.ChatNum = hash.size();
                    uidata.TxtInfo = "";
                    uidata.TxtInfo.append(getTime());
                    uidata.TxtInfo.append("\n");
                    uidata.TxtInfo.append(getName(hash.value(s)) + "上线");
                    emit hashChangeSetUi();
                } else {
                    //登录失败
                    QByteArray str;
                    str.append(pro.packData("<login-fail>"));
                    writeFlush(s, str);
                }
            } else {
                // 没有这个用户，发送用户不存在
                QByteArray str;
                str.append(pro.packData("<login-nothave>"));
                writeFlush(s, str);
            }
        }
    return;
    }
    // 群聊数据
    else if(data.contains("<wechat>")) {
        QByteArray temp = data;
        QString name,time,text,null;
        //解包
        pro.parseData(temp,time,text,null,null,null);
        name = getName(hash.value(s));
        //打包
        QByteArray str;
        str.append(pro.packData("<wechar-all>",name,time,text));
        //群发,遍历hash表
        for(auto it=hash.begin();it!=hash.end();it++)
        {
            if(it.key()!= s)
                writeFlush(it.key(), str);
        }
    return;
    }
    // 私聊离线数据
    else if(data.contains("<onechat>")) {
        QByteArray temp = data;
        //解析拿到目的num
        QString tarID,sendID,time,text,null;
        pro.parseData(temp,tarID,sendID,time,text,null);
        //设置接收方的文件路径
        QString filepath = tr("E:/summer_job_workers/week7/831493ba/ServerChatfile/%1.txt").arg(tarID);
        QFile writefile(filepath);
        // 存在打开，不存在创建
        if(writefile.open(QIODevice::Append | QIODevice::ReadWrite | QIODevice::Text)){
            QTextStream out(&writefile);
            out.setCodec("UTF-8");  //unicode UTF-8  ANSI
            //qDebug() << "writedata:" << data;
            // 把数据写入
            out << data << endl;;
            out.flush();
            qDebug() << filepath << "已经写入";
        }
        writefile.close();
    }
    // 如果客户端登陆查找是否有离线消息，有就发送
    else if(data.contains(("<getchat>"))) {
        QString dataLine;
        //解析拿到请求者num
        QString tarID,null;
        pro.parseData(data,tarID,null,null,null,null);
        //设置接收方的文件路径
        QString filepath = tr("E:/summer_job_workers/week7/831493ba/ServerChatfile/%1.txt").arg(tarID);
        QFile readfile(filepath);
        // 存在打开，不存在创建
        if(readfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&readfile);
            in.setCodec("UTF-8");
            while(!in.atEnd()) {
                dataLine = in.readAll();
                qDebug() << "dataall" << dataLine;
                writeFlush(s, dataLine.toStdString().data());
            }
        }
        readfile.close();
        // 删除文件->清空文件
        QFile::remove(filepath);
    }
    // 如果是加好友
    else if (data.contains(("<addchatnum>"))) {
        //解析拿到发送者，接受者，加好友结果
        QString sorID,tarID,result,null;
        pro.parseData(data,sorID,tarID,result,null,null);
        // 如果result为空，说明是请求，如果不为空，说明是结果
        if(result.isEmpty()){
            //查找数据库有没有此人，如果有，看看是不是已经是好友了，查找在线列表有没有此人
            QSqlQuery query;
            // 在数据库中查查这个人的chatnum
            QString cmd = tr("select chatnum from user_info where chatnum = '%1';").arg(tarID);
            if(query.exec(cmd)) {
                // 如果有这个用户，查找在线列表
                if(query.next()) {
                    // 有这个用户，如果在线
                    if(hash.contains(hash.key(query.value(0).toString()))){
                        // 转发这个好友请求
                        writeFlush(hash.key(query.value(0).toString()), data);
                    }
                    // 没有在线，放在文件里面
                    else {
                        //设置接收方的文件路径
                        QString filepath = tr("E:/summer_job_workers/week7/831493ba/ServerChatfile/%1add.txt").arg(tarID);
                        QFile writefile(filepath);
                        // 存在打开，不存在创建
                        if(writefile.open(QIODevice::Append | QIODevice::ReadWrite | QIODevice::Text)){
                            QTextStream out(&writefile);
                            out.setCodec("UTF-8");  //unicode UTF-8  ANSI
                            //qDebug() << "writedata:" << data;
                            // 把数据写入
                            out << data << endl;;
                            out.flush();
                            qDebug() << filepath << "加好友消息已经写入";
                        }
                        writefile.close();
                    }
                }
                // 如果没有这个用户，直接拒绝，说没有有这个用户
                else {
                    result = "<noexits>";
                    QString rejectdata = pro.packData("<addchatnum>",tarID, sorID, result);
                    writeFlush(s, rejectdata.toStdString().data());
                }
            }
        }
        // result不为空，说明是结果
        else {
            // 如果为同意，写数据库，把好友给他们加上，同时发送好友信息给两边
            if(result.contains("YES")) {
                // 写数据库，把好友信息写到friend表里面
                QSqlQuery query;
                // 直接插入两条语句
                QString cmd = tr("INSERT INTO friend SELECT '%1','%2' UNION ALL SELECT '%3','%4';")\
                        .arg(tarID).arg(sorID).arg(sorID).arg(tarID);
                if (query.exec(cmd)) {
                // 先给sorID发好友更更新
                // 给两边发送好友列表更新（发送当前的好友状态，发送IP)
                if(hash.contains(hash.key(sorID))) {
                    // 如果tarID在线
                    if(hash.contains((hash.key(tarID)))) {
                        //组装字符串
                        QString tarIdInfo = pro.packData("<addchatnum>",tarID, sorID, result,getName(tarID),getIp(tarID));
                        writeFlush(hash.key(sorID), tarIdInfo.toStdString().data());
                    }
                    // 如果tarID不在线
                    else {
                        //组装字符串
                        QString tarIdInfo = pro.packData("<addchatnum>",tarID, sorID, result,getName(tarID),"null");
                        writeFlush(hash.key(sorID), tarIdInfo.toStdString().data());
                    }
                }
                // 给tarID发好友消息
                if(hash.contains(hash.key(tarID))) {
                    // 如果sorID在线
                    if(hash.contains((hash.key(sorID)))) {
                        //组装字符串
                        QString tarIdInfo = pro.packData("<addchatnum>",sorID, tarID, result,getName(sorID),getIp(sorID));
                        writeFlush(hash.key(tarID), tarIdInfo.toStdString().data());
                    }
                    // 如果tarID不在线
                    else {
                        //组装字符串
                        QString tarIdInfo = pro.packData("<addchatnum>",sorID, tarID, result,getName(sorID),"null");
                        writeFlush(hash.key(tarID), tarIdInfo.toStdString().data());
                    }
                }
                }
            }
            // 如果不同意，直接转发给请求者
            else if (result.contains("NO")) {
                // 如果请求者在线
                if(hash.contains(hash.key(tarID))) {
                    writeFlush(hash.key(tarID), data);
                }
                // 如果请求者不在线,存入文件
                else {
                    //设置接收方的文件路径
                    QString filepath = tr("E:/summer_job_workers/week7/831493ba/ServerChatfile/%1add.txt").arg(tarID);
                    QFile writefile(filepath);
                    // 存在打开，不存在创建
                    if(writefile.open(QIODevice::Append | QIODevice::ReadWrite | QIODevice::Text)){
                        QTextStream out(&writefile);
                        out.setCodec("UTF-8");  //unicode UTF-8  ANSI
                        //qDebug() << "writedata:" << data;
                        // 把数据写入
                        out << data << endl;;
                        out.flush();
                        qDebug() << filepath << "拒绝好友消息已经写入";
                    }
                    writefile.close();
                }
            }
        }
    }
    return;
}

// 从消息队列侦听，如果有消息就拿出来进行处理
void Thread::processPendingTcpData() {
    qDebug() << "process thread id :" << QThread::currentThreadId();
    if (!MsgQueue.isEmpty()) {
        // 拿到消息
        QTcpSocket *s = MsgQueue.dequeue();
        qint64 datasize = 0;
        QDataStream in(s);
        in.setVersion(QDataStream::Qt_5_9);
        if(datasize == 0){
            // 如果接收到的数据还没到数据头大小，先不传给datasize
            if(s->bytesAvailable() < (int)sizeof(qint64))
                return;
            in >> datasize;
            qDebug() << "datasize:" << datasize;
        }
        // 如果还没有接收到指定的数据大小就先返回等他传完
        while(s->bytesAvailable() < datasize);

        QByteArray data;
        //in >> data;
        // 只要这个socket中还有可获得的数据（按字节数算）
        while(s->bytesAvailable()) {
            data.append(s->readAll());
        }
        qDebug() << "receive CLIENT data:" << data;
        // 处理信息
        processMsg(s,data);
    }
}

// Tcp连接断掉，或者客户端下线(TcpSocket还在连接，单纯的客户下线)
void Thread::clientdisconnect() {
    qDebug() << "xiaxianle";
    QTcpSocket* s = (QTcpSocket* )sender();
    uidata.TxtInfo = getName(hash.value(s)) + "下线";
    //告诉好友客户端自己下线了
    QByteArray removeName;
    removeName.append("<removeName-one>");
    removeName.append("$");
    removeName.append(getName(hash.value(s)));
    removeName.append("(");
    removeName.append(hash.value(s) + "-notconc");
    removeName.append(")");
    removeName.append("@");
    removeName.append("null");
    removeName.append("$");
    //拿到下线用户的所有好友num,如果在线，就发送好友下线信息
    QSqlQuery query;
    QString getInfoCmd = tr("select id_friend from friend where id_my = '%1';").arg(hash.value(s));
    if(query.exec(getInfoCmd)) {
        // 给所有的在线好友发信息，更新状态
        while(query.next()) {
            if(hash.contains(hash.key(query.value(0).toString()))) {
                writeFlush(hash.key(query.value(0).toString()), removeName);
            }
        }
    }
    //删除hash表中对应的内容
    hash.remove(s);
//    for(auto it=hash.begin();it!=hash.end();it++) {
//        writeFlush(it.key(), removeName);
//    }
    uidata.ChatNum = hash.size();
    emit hashChangeSetUi();
}

// ===============================================SERVER==================================================//

Server::Server(QWidget *parent) :
    QWidget(parent), ui(new Ui::Server) {

    ui->setupUi(this);
    server = NULL;
    socketprocessthread = NULL;
    workthread = NULL;
    initDir();
    initServer();
    initSql();
}

Server::~Server() {
    delete ui;
}

// 这里应该开多线程来跑
// 线程一:接收socket，发送socket
// 线程二:处理socket的信息，进行数据库操作

QString getserverIP()  //获取ip地址
{
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    foreach (QHostAddress address, list)
    {
       //我们使用IPv4地址
       if(address.protocol() == QAbstractSocket::IPv4Protocol)
           return address.toString();
    }
    return 0;
}

void Server::initDir() {
    //实例QDir
    QDir *folder = new QDir;
    //判断创建文件夹是否存在
    bool exist = folder->exists("E:/summer_job_workers/week7/831493ba/ServerChatfile");
    if(exist) {
        QMessageBox::warning(this,tr("创建文件夹"),tr("文件夹已经存在！"));
    }
    //如果不存在，创建文件夹
    else {
    //创建文件夹
    bool ok = folder->mkdir("E:/summer_job_workers/week7/831493ba/ServerChatfile");
    //判断是否成功
    if(ok) {
         QMessageBox::warning(this,tr("创建文件夹"),tr("文件夹创建成功！"));
    }
    else {
         QMessageBox::warning(this,tr("创建文件夹"),tr("文件夹创建失败！"));
     }
    }
}

//初始化服务器侦听
void Server::initServer() {
    // 把当前对象转换为一个TcpServer对象
    server = new QTcpServer(this);
    qDebug() << "SERVER IP: " << getserverIP();
    // 监听任何IPv4的发到10086端口的连接
    server->listen(QHostAddress::AnyIPv4,10086);
    // QTcpServer对象，只要发现有新的连接就会发出newConnection()信号
    connect(server,SIGNAL(newConnection()),this,SLOT(connectSlot()));

    // 创建消息执行线程
    socketprocessthread = new QThread();
    workthread = new Thread();
    // 官方历程，如果不绑定这个槽函数，就会发生内存泄漏
    connect(socketprocessthread, &QThread::finished, socketprocessthread, &QObject::deleteLater);
    connect(socketprocessthread, &QThread::finished, workthread, &QObject::deleteLater);
    // 线程通信
    connect(this,SIGNAL(intoMsgQueueOK()),workthread,SLOT(processPendingTcpData()));
    connect(workthread,SIGNAL(hashChangeSetUi()),this,SLOT(setUiInfo()));
    workthread->moveToThread(socketprocessthread);
    socketprocessthread->start();
}

//初始化数据库
void Server::initSql() {
    QSqlDatabase db=QSqlDatabase::addDatabase("QMYSQL");
    //连接数据库主机名，这里需要注意（若填的为”127.0.0.1“，出现不能连接，则改为localhost)
    db.setHostName("127.0.0.1");
    //连接数据库端口号，与设置一致
    db.setPort(3306);
    //连接数据库名，与设置一致
    db.setDatabaseName("webchatdata");
    //数据库用户名，与设置一致
    db.setUserName("root");
    //数据库密码，与设置一致
    db.setPassword("123456789");
    db.open();
    if(!db.open()) {
        qDebug()<<"不能连接"<<"connect to mysql error"<< db.lastError().text();
        return ;
    } else {
         qDebug()<<"连接成功"<<"connect to mysql OK";
         // 创建数据库表结构
         CreateUserInfoTable();
    }

}

// 建立用户信息表，好友列表，组群表，服务器状态表
void Server::CreateUserInfoTable() {
    //构造对象
    QSqlQuery query;
    // 创建前先查询，首先查看是否存在这三个表
    QString find_userinfo_cmd = "select t.table_name from information_schema.TABLES t \
                        where t.TABLE_SCHEMA = \"webchatdata\" \
                        and t.TABLE_NAME =\"user_info\";";
    QString find_friend_cmd = "select t.table_name from information_schema.TABLES t \
                        where t.TABLE_SCHEMA = \"webchatdata\" \
                        and t.TABLE_NAME =\"friend\";";
    QString find_groupinfo_cmd = "select t.table_name from information_schema.TABLES t \
                        where t.TABLE_SCHEMA = \"webchatdata\" \
                        and t.TABLE_NAME =\"group_info\";";
    QString find_group_cmd = "select t.table_name from information_schema.TABLES t \
                        where t.TABLE_SCHEMA = \"webchatdata\" \
                        and t.TABLE_NAME =\"group\";";
    // user_info表
    if (query.exec(find_userinfo_cmd)) {
        // 如果user_info表存在，就不新建
        if (query.next()){
            qDebug() << "user_info table exited";
        } else {
            //准备SQL命令
            QString cmd = " CREATE TABLE `webchatdata`.`user_info` (\
                      `chatnum` INT NOT NULL,\
                      `password` VARCHAR(45) NOT NULL,\
                      `name` VARCHAR(45) NULL,\
                      `gender` VARCHAR(45) NULL,\
                      PRIMARY KEY (`chatnum`));";
            if(query.exec(cmd)) {
                qDebug()<<"create user_info table success";
            } else {
                qDebug()<<"create fail";
            }
        }
    }
    query.clear();
    if (query.exec(find_friend_cmd)) {
        // 如果表存在，就不新建
        if (query.next()){
            qDebug() << "friend table exited";
        } else {
            //准备SQL命令
            QString cmd = "CREATE TABLE `webchatdata`.`friend` (\
                    `id_my` INT NOT NULL,\
                    `id_friend` INT NOT NULL,\
                    PRIMARY KEY (`id_my`, `id_friend`));";
            if(query.exec(cmd)) {
                qDebug()<<"create friend table success";
            } else {
                qDebug()<<"create fail";
            }
        }
    }
    query.clear();
    if (query.exec(find_group_cmd)) {
        // 如果表存在，就不新建
        if (query.next()){
            qDebug() << "group table exited";
        } else {
            //准备SQL命令
            QString cmd = "CREATE TABLE `webchatdata`.`group` (\
                        `groupnum` INT NOT NULL,\
                         `chatnum` INT NOT NULL,\
                         PRIMARY KEY (`groupnum`));";
            if(query.exec(cmd)) {
                qDebug()<<"create group table success";
            } else {
                qDebug()<<"create fail";
            }
        }
    }
    query.clear();
    if (query.exec(find_groupinfo_cmd)) {
        // 如果表存在，就不新建
        if (query.next()){
            qDebug() << "groupinfo table exited";
        } else {
            //准备SQL命令
            QString cmd = "CREATE TABLE `webchatdata`.`group_info` (\
                    `group_id` INT NOT NULL,\
                    `owner` INT NOT NULL,\
                    `group_name` VARCHAR(45) NULL,\
                    `describe` VARCHAR(100) NULL,\
                    `icon` VARCHAR(45) NULL,\
                    PRIMARY KEY (`group_id`));";
            if(query.exec(cmd)) {
                qDebug()<<"create group_info table success";
            } else {
                qDebug()<<"create fail";
            }
        }
    }


}

// 更新页面信息
void Server::setUiInfo() {
    ui->textEdit->append((uidata.TxtInfo));
    ui->onlineLabel->setText(QString::number(uidata.ChatNum));
}


void Server::msgIntoQueue(){
    // 创建一个Tcp的socket用来装客户端信息
    QTcpSocket *s = (QTcpSocket*)sender();

    // socket入队
    MsgQueue.enqueue(s);
    qDebug() << "msg into queue is OK!!";
    // 给子线程发消息，消息入队了
    emit intoMsgQueueOK();
}

//有新客户端连接
void Server::connectSlot() {
    // 获取已经建立连接的子套接字，相当于就是刚刚接收到的客户端的套接字
    QTcpSocket *socket = server->nextPendingConnection();
    qDebug() << socket;
    //将新的套接字添加到服务器的连接链表
    list.append(socket);
    // 得到连接socket的ip地址，这里可能会出现同一个IP地址建立两个连接的情况，
    // 因为同一台主机可以运行两个客户端程序
    // 所以应该是以用户为准，用户是唯一的
    QString ip = socket->peerAddress().toString();
    // 显示ip客户端上线信息
    ui->textEdit->append("["+ip+"]"+" 上线");
    // 只要接收到新的socket都会发出readyRead()这个信号
    // 把客户端的socket放入消息队列
    connect(socket,SIGNAL(readyRead()),this,SLOT(msgIntoQueue()));
    // 客户端TCP链接断掉
    connect(socket,SIGNAL(disconnected()),workthread,SLOT(clientdisconnect()));
    connect(socket,SIGNAL(disconnected()),this,SLOT(disconnectSlot()));
}

//释放客户端下线资源(list)
void Server::disconnectSlot() {
    //获取触发信号的对象
    QTcpSocket* s = (QTcpSocket*)sender();
    //删除指定对象
    list.removeOne(s);
    // 所以应该是以用户为准，用户是唯一的
    QString ip = s->peerAddress().toString();
    // 显示ip客户端上线信息
    ui->textEdit->append("["+ip+"]"+" 下线");
}
