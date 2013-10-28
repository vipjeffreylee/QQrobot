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
        loadjoke();
        loadchat();
    }
}
Robot::~Robot(){
    closedb();
    qDebug()<<"rebot close"<<endl;
    //db->close();
    //delete db;
}
void Robot::loadjoke(){
    QSqlQuery query("select * from joke;");
    while(query.next()){
        jokelist<<query.value("content").toString();
        //qDebug()<<jokelist<<endl;
    }
}
void Robot::loadchat(){
    QSqlQuery query("select * from chat;");
    QString id,question,answerstr;
    QStringList answerList;
    while(query.next()){
        id=query.value("id").toString();
        question=query.value("question").toString();
        answerstr=query.value("answer").toString();
        answerList.clear();
        answerList<<id<<answerstr.split("#A#");
        chat.insert(question,answerList);
        //qDebug()<<jokelist<<endl;
    }
}
void Robot::savechat(QString question, QString answer, QString creator){
    QSqlQuery query;
    QStringList answerList;
    QString id;
    if(chat.contains(question)){
        answerList=chat.value(question);
        answerList<<answer+"#C#"+creator;
        chat.insert(question,answerList);
        id=answerList.takeFirst();
        query.exec(QString("UPDATE chat SET answer='%1' where id=%2").arg(answerList.join("#A#")).arg(id));
    }else{
        answerList<<answer+"#C#"+creator;
        if(query.exec(QString("INSERT INTO chat (question,answer) values ('%1','%2')").arg(question).arg(answer+"#C#"+creator))){
            id=query.lastInsertId().toString();
            answerList.push_front(id);
            chat.insert(question,answerList);
        }else{
            qDebug()<<query.lastError()<<endl;
        }
    }
}
void Robot::deletechat(QString id){
    QSqlQuery query;
    if(!query.exec(QString("DELETE FROM chat where id=").arg(id))){
        qDebug()<<query.lastError()<<endl;
    }
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
                query.exec("CREATE TABLE chat (  id INTEGER PRIMARY KEY,question TEXT,answer TEXT);");
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
    }else if(message.indexOf("#谢谢")>-1){
        replyMsg.content= QString("%1 你太客气了！随时为你效劳。").arg(msgSender.friendName);
    }else if(message.left(2)=="#问"){
        int pos=message.indexOf("#答");
        if(pos>0){
            QString question=message.mid(2,pos-2).trimmed();
            QString answer=message.mid(pos+2);
            savechat(question,answer,msgSender.friendName);
            replyMsg.content=QString("谢谢，我明白了，问我:%1,我回答:%2,现有知识库%3条。").arg(question).arg(answer).arg(chat.size());
        }else{
            replyMsg.content="有问没答。";
        }
    }else if(message.left(2)=="#查"){
        QString question=message.mid(2).trimmed();
        QStringList answerList=chat.value(question);
        if(answerList.size()>0){
            QString id=answerList.takeFirst();
            replyMsg.content=QString("id=%1 问:%2 答:%3").arg(id).arg(question).arg(answerList.join(';'));
        }else{
            replyMsg.content=QString("没有查询到：%1").arg(question);
        }

    }else if(message.left(2)=="#删"){

    }else{
        QStringList answerList=chat.value(message);
        if(answerList.size()>1){
            answerList.removeFirst();
            QString answerstr=answerList.at(qrand()%(answerList.size()));
            int pos=answerstr.indexOf("#C#");
            replyMsg.content=answerstr.left(pos);
        }
    }
    return replyMsg;
}
