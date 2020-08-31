#ifndef ONECHAT_H
#define ONECHAT_H
//一对一聊天
#include <QWidget>
#include <QDateTime>
#include <QMessageBox>

#include "protocol.h"
#include "network.h"
namespace Ui {
class OneChat;
}

class OneChat : public QWidget
{
    Q_OBJECT

public:
    explicit OneChat(QWidget *parent = 0);
    ~OneChat();
    void setInfoName(QString data);
    void SetIp(QString ip);
    void setInfo(QString title,QString time,QString name,QString text);
    void setMyId(QString id);

private slots:
    void on_SendBtn_clicked();

private:
    Ui::OneChat *ui;
    QString ip;
    QString myid;
};

#endif // ONECHAT_H
