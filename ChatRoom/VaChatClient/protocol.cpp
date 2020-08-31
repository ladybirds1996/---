#include "protocol.h"

Protocol::Protocol(QObject *parent) : QObject(parent)
{
    end = "<end>";
    flag = "<$^$>";
}
//打包函数
QString Protocol::packData(QString head, QString data1, QString data2, QString data3, QString data4, QString data5)
{
    QString temp;
    temp.append(head);
    temp.append(flag);
    temp.append(data1);
    temp.append(flag);
    temp.append(data2);
    temp.append(flag);
    temp.append(data3);
    temp.append(flag);
    temp.append(data4);
    temp.append(flag);
    temp.append(data5);
    temp.append(flag);
    temp.append(end);

    return temp;
}
//协议解析函数
void Protocol::parseData(QByteArray temp, QString &data1, QString &data2, QString &data3, QString &data4, QString &data5)
{
    //1.删除数据头部
    temp.remove(0,temp.indexOf(flag)+flag.size());
    //2.提取数据
    data1 = temp.mid(0,temp.indexOf(flag));//mid 截取数据
    //3.再删除已经用过的数据
    temp.remove(0,temp.indexOf(flag)+flag.size());
    data2 = temp.mid(0,temp.indexOf(flag));
    //再删除已经用过的数据
    temp.remove(0,temp.indexOf(flag)+flag.size());
    data3 = temp.mid(0,temp.indexOf(flag));
    //再删除已经用过的数据
    temp.remove(0,temp.indexOf(flag)+flag.size());
    data4 = temp.mid(0,temp.indexOf(flag));
    //再删除已经用过的数据
    temp.remove(0,temp.indexOf(flag)+flag.size());
    data5 = temp.mid(0,temp.indexOf(flag));
}
