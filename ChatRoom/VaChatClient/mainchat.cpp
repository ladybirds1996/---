#include "mainchat.h"
#include "ui_mainchat.h"
#include <QMessageBox>

MainChat::MainChat(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainChat) {
    ui->setupUi(this);

    // 如果接收到服务器的消息，就启动消息处理函数
    connect(Network::getNetPtr(),SIGNAL(receiveData(QString)),this,SLOT(processChatData(QString)));
    // 如果接收到其他好友发过来的消息
    connect(Network::getNetPtr(),SIGNAL(udpreciveData(QString)),\
            this,SLOT(processChatData(QString)));
}

MainChat::~MainChat() {
    delete ui;
}
//时间戳
QString MainChat::getTime() {
    return QDateTime::currentDateTime().toString("yy-MM-dd hh:mm:ss");
}

//设置我的num
void MainChat::setmynum(QString id) {
    this->myid = id;
}

//得到我的num
QString MainChat::getmynum() {
    return myid;
}

// 向服务端要离线数据
void MainChat::getUnLineMsg() {
    //打包
    Protocol pro;
    QString str = pro.packData("<getchat>",myid);
    // 采用tcp发送给服务器
    Network::getNetPtr()->sendData(str);
}

//设置在线列表
void MainChat::setNameList(QString namelist) {
    // 删除<namelist-all>
    namelist.remove(0,namelist.indexOf("<namelist-all>"));
    namelist.remove(0,namelist.indexOf("$") + 1);
    //显示
    while(1) {
        QString nameid = namelist.mid(0,namelist.indexOf("@"));
        namelist.remove(0,namelist.indexOf("@") + 1);
        QString nameip = namelist.mid(0,namelist.indexOf("$"));
        qDebug() << "nameid:" << nameid << "ip" << nameip;

        QString name = nameid;
        // 在iphash里面只存储num,不存储状态
        int s = name.indexOf("(") + 1;
        name.remove(0, s);
        int e = name.indexOf("-");
        name = name.mid(0, e);

        // 将好友的ip保存在hash表中
        iphash.insert(name, nameip);
        //添加到listWidget
        ui->listWidget->addItem(nameid);
        namelist.remove(0,namelist.indexOf("$")+1);
        if(namelist.size() == 0)
            break;
    }
}

//添加上线用户信息
void MainChat::addNameid(QString data) {
    qDebug() << "namelistdata" << data;
    data.remove(0,data.indexOf("$") + 1);
    // 拿到name(id-状态)
    QString nameliststr = data.mid(0, data.indexOf("@"));
    // 去掉name(id-状态)
    data.remove(0, data.indexOf("@") + 1);
    // 拿到name的ip
    QString ip = data.mid(0, data.indexOf("$"));
    qDebug() << "name(id-状态):" << nameliststr << "ip" << ip;

    //先删除当初未上线的状态
    QString oldnameliststr = nameliststr.mid(0,(nameliststr.length() - 8)) + "-notconc)";
    //不要状态
    QString name_tmp = oldnameliststr;
    // 在iphash里面只存储num,不存储状态
    int s = name_tmp.indexOf("(") + 1;
    // 删掉前面的name(
    name_tmp.remove(0, s);
    int e = name_tmp.indexOf("-");
    // 存储上线用户的chatnum
    QString chatnum = name_tmp.mid(0, e);
    // 删除原来这个chatnum的信息
    iphash.remove(chatnum);

    // 加入新的这个chatnum  chatnum-ip
    iphash.insert(chatnum, ip);
    qDebug() << "addchatnum:" << chatnum;
    for(int i=0;i<ui->listWidget->count();i++) {
        if(ui->listWidget->item(i)->text() == oldnameliststr) {
            ui->listWidget->takeItem(i);
        }
    }

    // 同时如果有窗口存在的话，更新窗口的ip和chatnum信息
    if(hash.contains(hash.key(chatnum))) {
        hash.key(chatnum)->SetIp(ip);
    }

    // 更新为上线状态
    ui->listWidget->addItem(nameliststr);
}

// 添加下线用户信息
void MainChat::addDisNameid(QString data) {
    qDebug() << "namelistdata" << data;
    // 删掉<removeName-one>头部
    data.remove(0,data.indexOf("$") + 1);
    // 拿到name(id-状态)
    QString nameliststr = data.mid(0, data.indexOf("@"));
    // 去掉name(id-状态)
    data.remove(0, data.indexOf("@") + 1);
    // 拿到name的ip
    QString ip = data.mid(0, data.indexOf("$"));
    qDebug() << "name(id-状态):" << nameliststr << "ip" << ip;

    // 先删除当初上线的状态
    QString oldnameliststr = nameliststr.mid(0,(nameliststr.length() - 9)) + "-online)";
    //不要状态
    QString name_tmp = oldnameliststr;
    // 在iphash里面只存储num,不存储状态
    int s = name_tmp.indexOf("(") + 1;
    // 删掉前面的name(
    name_tmp.remove(0, s);
    int e = name_tmp.indexOf("-");
    // 存储上线用户的chatnum
    QString chatnum = name_tmp.mid(0, e);

    iphash.remove(chatnum);
    iphash.insert(chatnum, ip);

    qDebug() << "dischatnum:" << chatnum;
    for(int i=0;i<ui->listWidget->count();i++) {
        if(ui->listWidget->item(i)->text() == oldnameliststr) {
            ui->listWidget->takeItem(i);
        }
    }
    // 更新为下线状态
    ui->listWidget->addItem(nameliststr);
}

//将信息显示到一对一的聊天框
void MainChat::setoneChatInfo(QString data) {
    //解析
    qDebug() << "one to one:" << data;
    QString tarid,sorid,time,text,null;
    Protocol pro;
    pro.parseData(data.toUtf8(),tarid,sorid,time,text,null);
    // 如果不是自己的消息丢掉
    if (tarid != myid) {
        return;
    }
    //判断是否有对应的发送者聊天框
    if(hash.contains(hash.key(sorid))) {
        hash.key(sorid)->setInfo(sorid,time,sorid,text);
        return;
    }
    //新信息提示
    QMessageBox::information(this,"新信息",sorid + "发来信息");
    //打开一个新的聊天框
    OneChat *one = new OneChat;
    one->show();
    // 拿到对方ip
    one->SetIp(iphash.value(sorid));
    one->setInfoName(sorid);
    one->setMyId(myid);
    //在hash表中插入新聊天框信息
    hash.insert(one,sorid);
    // 设置窗口信息
    one->setInfo(sorid,time,sorid,text);
}

// 处理加好友消息，可能是请求，可能是结果
void MainChat::addfriend(QString data) {
    qDebug() << "receive add friend" << data;
    //解析拿到发送者，接受者，加好友结果
    QString sorID,tarID,result,sorName,sorIp;

    Protocol pro;
    pro.parseData(data.toUtf8(),sorID,tarID,result,sorName,sorIp);
    qDebug() << tarID;
    // 如果不是加的自己的就丢掉
    if (tarID != myid) {
        return;
    }
    qDebug() << "result:" << result;
    // 如果结果为空，说明是一个请求
    if (result.isEmpty()) {
        QMessageBox msg(this);
        msg.setText(sorID + "请求添加你为好友");
        msg.setIcon(QMessageBox::Question);
        msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

        switch(msg.exec()) {
            case (QMessageBox::Yes) : {
              qDebug() << "YES button is clicked!";
              result = "YES";
              break;
            }
            case (QMessageBox::No) : {
              qDebug() << "NO button is clicked!";
              result = "NO";
              break;
            }
        }
        // 如果客户端有选择，那就把消息回复给服务器
        if (result == "YES" || result == "NO") {
            QString str = pro.packData("<addchatnum>", tarID, sorID, result);
            qDebug() << str;
            // 采用tcp发送给服务器
            Network::getNetPtr()->sendData(str);
        }
    }
    // 结果，拿到好友信息
    else {
        // 如果好友不存在
        if(result.contains("<noexits")) {
            QMessageBox::information(this,"新信息",sorID + "不存在");
        }
        // 添加好友成功消息
        // 只有我在线发送请求，同时他在线得到请求，才会收到这条信息
        else if (result.contains("YES")) {
            // 拿到新好友状态
            QString name = sorName;
            QString nameid = sorID;
            QString ip = sorIp;

            qDebug() << "addname" << name << "addnameid" << sorID << "addip" << sorIp;

            // 把num和ip放入
            iphash.insert(nameid, ip);
            QString nameAndStatus;
            // 通过ip判断状态
            if (ip == "null") {
                nameAndStatus = name + "(" + nameid + "-notconc)";
            }
            else {
                nameAndStatus = name + "(" + nameid + "-online)";
            }

            qDebug() << "nameAnd:" << nameAndStatus;
            // 更新好友列表
            ui->listWidget->addItem(nameAndStatus);
        }
        // 其他的都是拒绝信息
        else {
            QMessageBox::information(this,"抱歉",sorID + "似乎不认识您");
        }
    }
}

// 处理接收的消息
void MainChat::processChatData(QString data) {
    qDebug() << "into  main chat processChatData processing!";
    // 登陆成功
    if(data.contains("<login-succ>")) {
        setNameList(data);
    }
    // 上线了一个好友
    else if(data.contains("<nameid-y>")) {
        addNameid(data);
    }
    // 下线了一个好友
    else if(data.contains("<removeName-one>")) {
        addDisNameid(data);
    }
    // 好友端对端消息
    else if(data.contains("<onechat>")) {
        while(data.size() != 0){
            // 有可能会有多条数据
            QString temp = data.mid(0, data.indexOf("<end>") + 5);
            qDebug() <<"data" << data;
            data.remove(0, data.indexOf("<end>") + 5);
            setoneChatInfo(temp);
        }
    }
    // 如果是一条加好友相关的信息
    else if(data.contains("<addchatnum>")) {
        while(data.size() != 0){
            // 有可能会有多条数据
            QString temp = data.mid(0, data.indexOf("<end>") + 5);
            qDebug() <<"data" << data;
            data.remove(0, data.indexOf("<end>") + 5);
            addfriend(temp);
        }
    }
}


//双击在线列表
void MainChat::on_listWidget_itemDoubleClicked(QListWidgetItem *item) {
    //聊天对象,也就是 chatnum(chatnum-online)

    QString nameliststr = item->text();
    // 在iphash里面只存储num,不存储状态
    int s = nameliststr.indexOf("(") + 1;
    nameliststr.remove(0, s);
    int e = nameliststr.indexOf("-");
    QString nameid = nameliststr.mid(0, e);

    qDebug() << "mainchat info:" << nameid;
    //先判断有没有当前对话框,如果有，就把那个页面放在页面最前面
    if(hash.contains(hash.key(nameid))) {
        //设置当前界面显示在最上层
        hash.key(nameid)->setWindowFlags(Qt::WindowStaysOnTopHint);
        hash.key(nameid)->show();
        hash.key(nameid)->setWindowFlags(hash.key(nameid)->windowFlags() & ~Qt::WindowStaysOnTopHint);
        hash.key(nameid)->show();
        return;
    }
    //显示对话框
    OneChat *one = new OneChat;
    one->show();
    // 拿到对方ip
    one->SetIp(iphash.value(nameid));
    one->setInfoName(nameid);
    one->setMyId(myid);
    //把窗口指针和用户信息放入窗口hash中
    hash.insert(one,nameid);
}

// 加好友
void MainChat::on_pushButton_clicked() {
    // 拿到药搜索好友的num
    QString chatnum = ui->addchatnum->text();
    // 如果没有输入的话
    if(chatnum.isEmpty()){
        return;
    }
    //打包
    Protocol p;
    QString data = p.packData("<addchatnum>",myid,chatnum);
    QMessageBox::information(this,"发送成功","向"+ chatnum +"发出好友请求");
    ui->addchatnum->clear();
    //发送给服务器
    Network::getNetPtr()->sendData(data);
}
