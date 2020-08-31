#include "onechat.h"
#include "ui_onechat.h"

OneChat::OneChat(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OneChat)
{
    ui->setupUi(this);
    // 初始化ip
    ip = "null";
}

OneChat::~OneChat() {
    delete ui;
}

void OneChat::setInfoName(QString data) {
    ui->nameLabel->setText(data);
}

//配置窗口ip
void OneChat::SetIp(QString ip) {
    this->ip = ip;
}

//配置我的id
void OneChat::setMyId(QString id) {
    myid = id;
}

//设置信息
void OneChat::setInfo( QString sorid, QString time, QString sorname, QString text) {

    // 发送者id
    ui->nameLabel->setText(sorid);
    ui->textEdit->append(time);
    // 这句话要加到消息中间，不然会把上面的那条消息也变到这里
    ui->textEdit->setAlignment(Qt::AlignLeft);
    ui->textEdit->append(sorname +"："+text);
}

//发送
void OneChat::on_SendBtn_clicked() {
    QString revid,time,text;
    // 接收者
    revid = ui->nameLabel->text();//name(id)
    qDebug() << "revid" << revid;
    time = QDateTime::currentDateTime().toString("yy-MM-dd hh:mm:ss");
    //拿到发送信息，同时做信息长度判断
    text = ui->SendLine->text();
    if(text.toLatin1().length() > 50000) {
        // 显示数据过长
        // 新信息提示
        QMessageBox::information(this, "错误", "发送数据过长");
        return;
    }
    //打包
    Protocol pro;
    QString str = pro.packData("<onechat>",revid,myid,time,text);
    qDebug() << "to one:" << str;
    // 把信息显示在页面上
    ui->textEdit->append(time);
    ui->textEdit->setAlignment(Qt::AlignRight);
    ui->textEdit->append("我："+ text);

    ui->SendLine->clear();
    // 如果配置了udpip,说明对方还在线
    if(ip != "null"){
        //采用udp直接发送到对方电脑
        Network::getNetPtr()->sendtoData(ip, str);
    }
    // 如果没有配置udpip，就说明是离线消息
    else {
        // 采用tcp发送给服务器
        Network::getNetPtr()->sendData(str);
    }
}
