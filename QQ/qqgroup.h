/****************************************************************************
**
** Copyright (C) 2013 Jeffrey Lee.
** All rights reserved.
** Contact: vipjeffreylee@gmail.com
** Please keep the author contact information.
** 2013-10-10
**
****************************************************************************/
#ifndef QQGROUP_H
#define QQGROUP_H
#include <QString>
#include <QHash>
#include<QVector>
#include<qqfriend.h>
class QQgroup
{
public:
    QQgroup();
public:
    QString name;
    QString uin;
    QString txuin;
    QString code;
    QHash<QString,QQfriend*> members;
    QVector<int> robots;//robot list的下标作为ID
};

#endif // QQGROUP_H
