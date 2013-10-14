/****************************************************************************
**
** Copyright (C) 2013 Jeffrey Lee.
** All rights reserved.
** Contact: vipjeffreylee@gmail.com
** Please keep the author contact information.
** 2013-10-10
**
****************************************************************************/
#include "webqq.h"
#include "webqqnet.h"
#include "logindlg.h"
#include "mainwindow.h"
namespace WebQQ {
Msg::Msg(QObject *parent) :
    QObject(parent){
}
QQstatus status;
WebQQNet* webQQNet=0;
LoginDlg* loginDlg=0;
MainWindow* mainWindow=0;
QString qqnum;
QHash<QString,QQgroup*> qqGroups;
QHash<QString,QQfriend*> qqFriends;
QList<QQmsg*> qqmsgs;
QStandardItemModel *qqGroupListModel=new QStandardItemModel();
QStandardItemModel *qqFriendListModel=new QStandardItemModel();
bool isclose=false;
QMutex mutex;
void login(){
    webQQNet=new WebQQNet();
    loginDlg=new LoginDlg();
    loginDlg->show();
}
void loginFinished(bool ok){
    if(ok){
        mainWindow=new MainWindow();
        mainWindow->show();
        loginDlg->hide();loginDlg->close();loginDlg->deleteLater();
    }
}
}
