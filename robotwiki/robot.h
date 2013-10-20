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
#include <QStringList>
#include "robotinterface.h"
class QNetworkAccessManager;
class QNetworkReply;
class  Robot : public QObject, RobotInterface
{
        Q_OBJECT
        Q_PLUGIN_METADATA(IID RobotInterface_iid)
        Q_INTERFACES(RobotInterface)
public:
    Robot();
    QString name();
    ReplyMsg listenandsay(const MsgSender msgSender, const QString &message);
private slots:
    void replyFinished(QNetworkReply * reply);
signals:
    void OnEventLoopQuit();
private:
    QNetworkAccessManager *http;
    QString getWeather();
    QString weather;

};

#endif // ROBOT_H
