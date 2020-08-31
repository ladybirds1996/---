#ifndef REGISTER_H
#define REGISTER_H
//登录注册类
#include <QWidget>
#include <QMessageBox>

#include "protocol.h"
#include "network.h"
#include "mainchat.h"

namespace Ui {
class Register;
}

class Register : public QWidget
{
    Q_OBJECT

public:
    explicit Register(QWidget *parent = 0);
    ~Register();

private slots:
    void on_RokBtn_clicked();

    void processReceiveData(QString data);

    void on_RegBtn_clicked();

    void on_RconsBtn_clicked();

    void on_LokBtn_clicked();

private:
    Ui::Register *ui;
    MainChat *mainchat;
};

#endif // REGISTER_H
