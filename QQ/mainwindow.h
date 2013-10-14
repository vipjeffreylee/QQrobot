/****************************************************************************
**
** Copyright (C) 2013 Jeffrey Lee.
** All rights reserved.
** Contact: vipjeffreylee@gmail.com
** Please keep the author contact information.
** 2013-10-10
**
****************************************************************************/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QModelIndex>
#include<QStandardItemModel>
#include "robotinterface.h"
class QButtonGroup;
class QQmsg;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
protected:
    bool eventFilter(QObject *obj, QEvent *ev);
private:
    Ui::MainWindow *ui;
protected:
    void closeEvent(QCloseEvent *event);
public slots:
    void msgReceived();
    void selChgGetTXUIN();
    void sendMsg();
    void qqFriendSelected(QModelIndex mi);
    void qqGroupSelected(QModelIndex mi);
    void robotsSelected(int id);
    void sendMsgFinished(QString txuin,QString msg,bool isok);
    void sysMsg(QString msg);
private:
    QButtonGroup *robotButtonGroup;
    QString formatMsg(QQmsg *msg);
    bool loadPlugin();
    QList<RobotInterface *> robots;
    QModelIndex currModelIndex;
   };

#endif // MAINWINDOW_H
