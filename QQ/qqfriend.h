/****************************************************************************
**
** Copyright (C) 2013 Jeffrey Lee.
** All rights reserved.
** Contact: vipjeffreylee@gmail.com
** Please keep the author contact information.
** 2013-10-10
**
****************************************************************************/
#ifndef QQFRIEND_H
#define QQFRIEND_H
#include<QString>
#include<QVector>
class QQfriend
{
public:
    QQfriend();
public:
    QString uin;
    QString txuin;
    QString nick;
    QString markname;
    QVector<int> robots;//robot list的下标作为ID
};

#endif // QQFRIEND_H
