/****************************************************************************
**
** Copyright (C) 2013 Jeffrey Lee.
** All rights reserved.
** Contact: vipjeffreylee@gmail.com
**
** This file is part of the ShanbayDict.
**
** $BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
**
**
** $END_LICENSE$
**
****************************************************************************/
#include <QtNetwork>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include "baidutranslate.h"
#include <QDebug>
Baidutranslate::Baidutranslate(QObject *parent) :
    QObject(parent)
{
   http=new QNetworkAccessManager(this);
   connect(http,SIGNAL(finished(QNetworkReply*)),this,SLOT(httpfinished(QNetworkReply*)));
}
Baidutranslate::~Baidutranslate(){

}

void Baidutranslate::baidutranslate(QString text){
    QNetworkRequest request;
    request.setUrl(QUrl("http://openapi.baidu.com/public/2.0/bmt/translate"));
    request.setRawHeader("Accept","text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    request.setRawHeader("Accept-Charset","GBK,utf-8;q=0.7,*;q=0.3");
    //request.setRawHeader("Accept-Encoding","gzip,deflate,sdch");
    request.setRawHeader("Accept-Language","zh-CN,zh;q=0.8");
    request.setRawHeader("Cache-Control","max-age=0");
    request.setRawHeader("Connection","keep-alive");
    request.setRawHeader("Host","www.shanbay.com");
    request.setRawHeader("User-Agent","Mozilla/5.0 (Windows NT 6.1) AppleWebKit/535.7 (KHTML, like Gecko) Chrome/16.0.912.63 Safari/535.7");
     request.setRawHeader("Content-Type","application/x-www-form-urlencoded");
    QByteArray postData;
    postData.append("client_id=RsZ0PWGTA4TXEhqhmhhaTQBO&q=");
    postData.append(text.toUtf8());
    postData.append("&from=auto&to=auto");
    request.setHeader(QNetworkRequest::ContentLengthHeader,postData.size());
    http->post(request,postData);
    QEventLoop evenloop;
    connect(this, SIGNAL(translateFinshed()), &evenloop, SLOT(quit()));
    evenloop.exec();
}

void Baidutranslate::httpfinished(QNetworkReply* reply){
    qDebug()<<"end translate!!!!!";
    //qDebug()<<reply->errorString()<<QString::fromUtf8(reply->readAll());
    QJsonDocument jsonDoc;
    QJsonObject jsonObj;
    QString resutstr="";
    jsonDoc=QJsonDocument::fromJson(reply->readAll());
    if(!jsonDoc.isNull()){
        jsonObj=jsonDoc.object();
        if(jsonObj.contains("error_msg")){
            translateTXT=QString(tr("服务器返回错误：%1")).arg(jsonObj.value("error_msg").toString());
        }else{
            translateTXT=jsonObj.value("trans_result").toArray()[0].toObject().value("dst").toString();
        }
    }
    emit translateFinshed();
}
QString Baidutranslate::translate(QString txt){
    static Baidutranslate *baidutranlate=nullptr;
    if(baidutranlate==nullptr){
        baidutranlate=new Baidutranslate();
    }
    baidutranlate->baidutranslate(txt);
    return baidutranlate->translateTXT;
}
