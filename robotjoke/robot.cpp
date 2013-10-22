/****************************************************************************
**
** Copyright (C) 2013 Jeffrey Lee.
** All rights reserved.
** Contact: vipjeffreylee@gmail.com
** Please keep the author contact information.
** 2013-10-10
**
****************************************************************************/
#include "robot.h"
#include<QTime>
#include<QDebug>
#include<QDateTime>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDir>
Robot::Robot(){
    QTime time;
    time= QTime::currentTime();
    qsrand(time.msec()+time.second()*1000);
    qDebug()<<"rebot start"<<endl;
    if(loaddb()){
        QSqlQuery query("select * from joke;");
        while(query.next()){
            jokelist<<query.value("content").toString();
            //qDebug()<<jokelist<<endl;
        }
    }
}
Robot::~Robot(){
    closedb();
    qDebug()<<"rebot close"<<endl;
    //db->close();
    //delete db;
}
bool Robot::loaddb(){
    QSqlDatabase db=QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(QString(".%1db%1robotjoke.db3").arg(QDir::separator()));
    if(db.open()){
        qDebug()<<"数据库打开成功！"<<endl;
    }else{
        qDebug()<<"数据库不存在，现在新建数据库"<<endl;
        QDir::current().mkdir("db");
        if(db.open()){
            qDebug()<<"数据库新建成功！"<<db.databaseName()<<endl;
            QSqlQuery query(db);
            if (query.exec("CREATE TABLE joke (  id INTEGER PRIMARY KEY,content TEXT,creator TEXT);")) {
                qDebug() << "create table joke success.";
            } else {
                qDebug() << "create table joke failure" << query.lastError();
                return false;
            }
        }else{
            qDebug()<<"数据库打开失败！"<<db.databaseName()<<endl;
            return false;
        }
    }
    return true;
}
void Robot::closedb(){
    QString name;
    {
        name = QSqlDatabase::database().connectionName();
    }
    QSqlDatabase::removeDatabase(name);
}
bool Robot::savejoke(QString joke,QString creator){
    QSqlQuery query;
    return query.exec(QString("INSERT INTO joke (content,creator) values('%1','%2');").arg(joke).arg(creator));
}

QString Robot::name(){

    return QString("我爱讲笑话");
}
ReplyMsg Robot::listenandsay(const MsgSender msgSender, const QString &message){
    ReplyMsg replyMsg;
    if(msgSender.groupTXUIN.isEmpty()){
        replyMsg.recTXUIN=msgSender.friendTXUIN;
    }else{
        replyMsg.recTXUIN=msgSender.groupTXUIN;
    }
    if(message.indexOf("#help")>-1){
        replyMsg.content="我爱讲笑话：\n  #joke、#笑话;#addjoke 增加笑话";

    }else if(message.indexOf("#joke")>-1||message.indexOf("#笑话")>-1){
        if(jokelist.isEmpty()){
            replyMsg.content="对不起，我现在没收集到任何笑话，请用#addjoke命令添加笑话。";
        }else{
             replyMsg.content=jokelist.at(qrand()%(jokelist.length()));
        }
    }else if(message.left(8)=="#addjoke"){
        jokelist<<message.mid(8);
        replyMsg.content=QString("新笑话已经增加,现在共收集笑话%1条。").arg(jokelist.length());
        savejoke(jokelist.last(),msgSender.friendName);
    }else if(message.indexOf("谢谢")>-1){
        replyMsg.content= QString("%1 你太客气了！随时为你效劳。").arg(msgSender.friendName);
    }
    return replyMsg;
}
