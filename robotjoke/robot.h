/****************************************************************************
**
** Copyright (C) 2013 Jeffrey Lee.
** All rights reserved.
** Contact: vipjeffreylee@gmail.com
** Please keep the author contact information.
** 2013-10-10
**
****************************************************************************/
#ifndef ROBOT_H
#define ROBOT_H
#include <QObject>
#include <QtPlugin>
#include<QStringList>
#include<QMap>
#include "robotinterface.h"
class  Robot : public QObject, RobotInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID RobotInterface_iid)
    Q_INTERFACES(RobotInterface)
public:
    Robot();
    ~Robot();
    QString name();
    ReplyMsg listenandsay(const MsgSender msgSender, const QString &message);
private:
    // QStringList joke;

    bool loaddb();
    void closedb();
    QStringList jokelist;
    void loadjoke();
    bool savejoke(QString joke,QString creator);
    QMap<QString,QStringList> chat;
    void loadchat();
    void savechat(QString question,QString answer,QString creator);
    void deletechat(QString id);

};

#endif // ROBOT_H
