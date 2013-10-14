/****************************************************************************
**
** Copyright (C) 2013 Jeffrey Lee.
** All rights reserved.
** Contact: vipjeffreylee@gmail.com
** Please keep the author contact information.
** 2013-10-10
**
****************************************************************************/
#ifndef WEBQQ_H
#define WEBQQ_H
#include <QObject>
#include <QList>
#include <QStandardItemModel>
#include <QStandardItem>
#include<QMutex>
#include "webqqnet.h"
#include "qqgroup.h"
#include "qqfriend.h"
#include "qqmsg.h"
//class WebQQNet;
class LoginDlg;
class MainWindow;
enum class QQstatus{online,offline};
namespace WebQQ {
class Msg : public QObject
{
    Q_OBJECT
public:
    explicit Msg(QObject *parent = 0);

signals:

public slots:
};
extern QQstatus status;
extern WebQQNet* webQQNet;
extern LoginDlg* loginDlg;
extern MainWindow* mainWindow;
extern void login();
extern void loginFinished(bool ok);
extern QString qqnum;
extern QHash<QString,QQgroup*> qqGroups;
extern QHash<QString,QQfriend*> qqFriends;
extern QList<QQmsg*> qqmsgs;
extern QStandardItemModel *qqGroupListModel;
extern QStandardItemModel *qqFriendListModel;
extern bool isclose;
extern QMutex mutex;
}
#endif // WEBQQ_H
