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
#include <QDebug>
#include <QDateTime>
#include <QtNetwork>
#include <QEventLoop>
#include <baiduweather.h>
#include <baidutranslate.h>
Robot::Robot(){
    http=new QNetworkAccessManager(this);
    connect(http,SIGNAL(finished(QNetworkReply*)),this,SLOT(replyFinished(QNetworkReply*)));
}
QString Robot::name(){
    return QString("生活小百科");
}
ReplyMsg Robot::listenandsay(const MsgSender msgSender, const QString &message){
    ReplyMsg replyMsg;
    if(message.at(0)!='#'){
        return replyMsg;
    }
    if(msgSender.groupTXUIN.isEmpty()){
        replyMsg.recTXUIN=msgSender.friendTXUIN;
    }else{
        replyMsg.recTXUIN=msgSender.groupTXUIN;
    }
    QString tmpstr;
    if(message.indexOf("#help")==0){
        replyMsg.content="我是生活小百科，我能告诉你：\n#几点了、#time\n#天气 城市名\n#translate 空格间隔翻译内容";
    }else if(message.indexOf("#几点了")==0||message.indexOf("#time")==0){
        replyMsg.content=QString("%1 现在时间是：\n%2")
                .arg(msgSender.friendName)
                .arg(QDateTime::currentDateTime().toString("yyyy年MM月dd日 hh点mm分ss秒"));
    }else if(message.indexOf("#天气")==0){
        tmpstr=message.mid(3).trimmed();
        if(tmpstr.isEmpty()){
            replyMsg.content="请告诉我，你要哪里的天气预报？";
        }else{
            replyMsg.content=BaiduWeather::getWeather(tmpstr);
        }

    }else if(message.indexOf("#translate")==0||message.indexOf("#fy")==0||message.indexOf("#翻译")==0){
        tmpstr=message.mid(message.indexOf(' ')).trimmed();
        if(tmpstr.isEmpty()){
            replyMsg.content="请告诉我，你要我帮你翻译什么？";
        }else{
            replyMsg.content=Baidutranslate::translate(tmpstr);
        }

    }
    return replyMsg;
}
QString Robot::getWeather(){
    QEventLoop q;
    connect(this, SIGNAL(OnEventLoopQuit()), &q, SLOT(quit()));  //异步调用完成退
    weather.clear();
    http->get(QNetworkRequest(QUrl(
                                  QString("http://cgi.appx.qq.com/cgi/qqweb/weather/wth/weather.do?retype=1&t=%1")
                                  .arg(QDateTime::currentMSecsSinceEpoch()))));
    q.exec();
    return weather;
}
void Robot::replyFinished(QNetworkReply *reply){
    weather=QString::fromUtf8(reply->readAll());
    reply->deleteLater();
    emit OnEventLoopQuit();
    return;
}
