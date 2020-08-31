#include "register.h"
#include "ui_register.h"

Register::Register(QWidget *parent) :
    QWidget(parent),\
    // 这句话，把设计的UI拿过来
    ui(new Ui::Register) {
    // 激活UI界面，标准形式
    ui->setupUi(this);
    // 跳转到登陆页面
    ui->stackedWidget->setCurrentIndex(1);
    //设置模式 password模式
    ui->RpwLine->setEchoMode(QLineEdit::Password);
    ui->Rpw2Line->setEchoMode(QLineEdit::Password);
    ui->LpwLine->setEchoMode(QLineEdit::Password);
    // Network::getNetPtr()初始化网络链接连接到服务器，这里采用单例模式
    // 设置信号槽，如果网络接收到数据，就启动接收数据的槽函数
    connect(Network::getNetPtr(),SIGNAL(receiveData(QString)),this,SLOT(processReceiveData(QString)));
}

Register::~Register() {
    delete ui;
}


void Register::on_RokBtn_clicked() {
    QString id,name,passwd,passwd2,gender;
    //
    id = ui->RidLine->text();
    name = ui->RnameLine->text();
    passwd = ui->RpwLine->text();
    passwd2 = ui->Rpw2Line->text();
    gender = ui->comboBox->currentText();

    qDebug() << "this is a test";
    //判断所有信息不能为空
    if(id==""||name==""||passwd=="")
    {
        QMessageBox::warning(this,"警告","信息不能为空!");
        return;
    }
    if(passwd != passwd2)
    {
        QMessageBox::warning(this,"警告","两次密码不一致!");
        return;
    }
    //打包
    Protocol p;
    QString head = "<register>";
    QString data = p.packData(head,id,passwd,name,gender);
    qDebug() << "this is two test";
    //借助网络类发送
    Network::getNetPtr()->sendData(data);
}

//接收网络类发来的数据
void Register::processReceiveData(QString data) {
    qDebug() << "into ReceiveMsg processing!";
    qDebug() << "data:" << data;
    if(data.contains("<register-succ>")) {
        QMessageBox::information(this,"提示","注册成功");
        return;
    } else if(data.contains("<register-error>")) {
        QMessageBox::information(this,"提示","信息错误");
        return;
    }else if(data.contains("<register-have>")) {
        QMessageBox::information(this,"提示","该账号已注册");
        return;
    }else if(data.contains("<register-havenot>")) {
        QMessageBox::warning(this,"警告","账号不存在，不允许注册");
        return;
    }else if(data.contains("<login-succ>")) {
        //登录成功，显示好友列表
        mainchat = new MainChat;
        mainchat->show();
        mainchat->setNameList(data);
        //设置我的num
        mainchat->setmynum(ui->LidLine->text());
        //拿到离线消息
        mainchat->getUnLineMsg();
        //关闭登陆界面
        this->close();
        //this->hide();
    }else if(data.contains("<login-fail>")) {
        QMessageBox::warning(this,"警告","密码错误！");
    }else if(data.contains("<login-nothave>")){
        QMessageBox::warning(this,"警告","账号不存在");
    }else if(data.contains("<login-have>")) {
        QMessageBox::warning(this,"警告","该账号已登录");
    }
}
//登录界面注册按钮，跳转到注册页面
void Register::on_RegBtn_clicked() {
    // 这里设置页面
    ui->stackedWidget->setCurrentIndex(0);
    ui->RidLine->clear();
    ui->RpwLine->clear();
    ui->Rpw2Line->clear();
    ui->RnameLine->clear();
}

//注册界面取消按钮，跳转到登录页面
void Register::on_RconsBtn_clicked() {
    ui->stackedWidget->setCurrentIndex(1);
}

//登录按钮，给服务器发送消息
void Register::on_LokBtn_clicked() {
    QString id,passwd;
    id = ui->LidLine->text();
    passwd = ui->LpwLine->text();
    //打包
    Protocol p;
    // id 是发的登陆的号码
    QString data = p.packData("<login>",id,passwd);
    //发送给服务器
    Network::getNetPtr()->sendData(data);
}
