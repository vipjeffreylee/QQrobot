/****************************************************************************
**
** Copyright (C) 2013 Jeffrey Lee.
** All rights reserved.
** Contact: vipjeffreylee@gmail.com
** Please keep the author contact information.
** 2013-10-10
**
****************************************************************************/
#ifndef ROBOTINTERFACE_H
#define ROBOTINTERFACE_H
#include <QString>


class MsgSender{
public:
    //friend group信息都有值是群信息发送者，只有friend信息是个人
    QString friendTXUIN;//发送者通迅号 FTX开头
    QString friendUIN;//发送者QQ号，未支持
    QString friendName;//发送者名字
    QString groupTXUIN;//发送群通迅号 GTX开头
    QString groupUIN;//发送群QQ号 未支持
    QString groupName;//发送群名称
};
class ReplyMsg{
public:
    QString recTXUIN;//回复接收者通迅号 GTX或者 FTX开头
    QString content;
};

class RobotInterface
{
public:
    virtual ~RobotInterface() {}
    virtual QString name() = 0;
    virtual ReplyMsg listenandsay(const MsgSender msgSender,const QString &message) = 0;
};
#define RobotInterface_iid "org.qt-project.Qt.RobotInterface"
Q_DECLARE_INTERFACE(RobotInterface, RobotInterface_iid)
#endif // ROBOTINTERFACE_H
