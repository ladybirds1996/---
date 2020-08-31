#ifndef PROTOCOL_H
#define PROTOCOL_H

//协议类
#include <QObject>

class Protocol : public QObject
{
    Q_OBJECT
public:
    explicit Protocol(QObject *parent = 0);
    QString packData(QString head,QString data1="",QString data2="",\
                     QString data3="",QString data4="",QString data5="");

    void parseData(QByteArray temp,QString &data1,QString &data2,\
                   QString &data3,QString &data4,QString &data5);
signals:

public slots:
private:
    QString end;
    QString flag;
};

#endif // PROTOCOL_H
