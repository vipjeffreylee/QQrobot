/****************************************************************************
**
** Copyright (C) 2013 Jeffrey Lee.
** All rights reserved.
** Contact: vipjeffreylee@gmail.com
** Please keep the author contact information.
** 2013-10-10
**
****************************************************************************/
#ifndef QQMSG_H
#define QQMSG_H
#include<QString>
class QQmsg
{
public:
    QQmsg();
public:
    int type;
    QString from_uin;//发送者TX号，type:0是好友TX号，type:1是群TX号
    QString send_uin;//群内发送者TX号，type:1时有意义
    uint time;
    QString content;
};

#endif // QQMSG_H
