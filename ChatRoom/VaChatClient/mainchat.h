#ifndef MAINCHAT_H
#define MAINCHAT_H
//主界面类
#include <QWidget>
#include <QDateTime>
#include <QListWidgetItem>
#include <QHash>

#include "protocol.h"
#include "network.h"
#include "onechat.h"

namespace Ui {
class MainChat;
}

class MainChat : public QWidget {
    Q_OBJECT

public:
    explicit MainChat(QWidget *parent = 0);
    ~MainChat();
    QString getTime();

     void setNameList(QString namelist);
     void addNameid(QString nameid);
     void addDisNameid(QString nameid);
     void removeName(QString name);
     void setoneChatInfo(QString data);
     void setmynum(QString id);
     void getUnLineMsg();
     void addfriend(QString data);
     QString getmynum();

private slots:
    void on_SendBtn_clicked();

    void processChatData(QString);

    void on_SendLine_returnPressed();

    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void on_pushButton_clicked();

private:
    Ui::MainChat *ui;
    QString myid;

    // 管理聊天窗口
    QHash<OneChat*,QString> hash;
    // 管理好友ip地址 chatnum-ip
    QHash<QString, QString> iphash;
};

#endif // MAINCHAT_H
