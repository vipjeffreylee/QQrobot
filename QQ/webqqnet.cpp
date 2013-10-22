/****************************************************************************
**
** Copyright (C) 2013 Jeffrey Lee.
** All rights reserved.
** Contact: vipjeffreylee@gmail.com
** Please keep the author contact information.
** 2013-10-10
**
****************************************************************************/
#include "webqqnet.h"
#include "webqq.h"
#include <QtNetwork>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QCryptographicHash>
WebQQNet::WebQQNet(QObject *parent) :
    QObject(parent)
{
    pCookieJar=new QNetworkCookieJar(this);
    http=new QNetworkAccessManager(this);
    httpPoll=new QNetworkAccessManager(this);
    httpExtInfo=new QNetworkAccessManager(this);
    http->setCookieJar(pCookieJar);
    httpPoll->setCookieJar(pCookieJar);
    httpExtInfo->setCookieJar(pCookieJar);
    connect(http,SIGNAL(finished(QNetworkReply*)),this,SLOT(httpFinished(QNetworkReply*)));
    connect(httpPoll,SIGNAL(finished(QNetworkReply*)),this,SLOT(httpPollFinished(QNetworkReply*)));
    connect(httpExtInfo,SIGNAL(finished(QNetworkReply*)),this,SLOT(httpExtInfoFinished(QNetworkReply*)));
    QTime time;
    time= QTime::currentTime();
    qsrand(time.msec()+time.second()*1000);
    clientid=QString::number(qrand());
    msgId=(qrand()%1000)*10000+1;
    timer=new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(sendMsg()));
    //qDebug()<<"msgId="<<msgId<<endl;
    //clientid="30770623";
}
void WebQQNet::pollMsg(){
    if(WebQQ::status==QQstatus::offline) return;
    QNetworkRequest request;
    request.setUrl(QUrl("http://d.web2.qq.com/channel/poll2"));
    request.setRawHeader("Accept","*/*");
    request.setRawHeader("Accept-Encoding","gzip,deflate,sdch");
    request.setRawHeader("Accept-Language","zh-CN,zh;q=0.8,en-US;q=0.6,en;q=0.4");
    request.setRawHeader("Accept-Charset","GBK,utf-8;q=0.7,*;q=0.3");
    request.setRawHeader("Connection","keep-alive");
    request.setRawHeader("Content-Type","application/x-www-form-urlencoded");
    request.setRawHeader("Host","d.web2.qq.com");
    request.setRawHeader("Origin","http://d.web2.qq.com");
    request.setRawHeader("Referer","http://d.web2.qq.com/proxy.html?v=20110331002&callback=1&id=2");
    request.setRawHeader("User-Agent","Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/29.0.1547.62 Safari/537.36");
    QByteArray postData;
    postData.append("r="+QUrl::toPercentEncoding(QString("{\"clientid\":\"%1\",\"psessionid\":\"%2\",\"key\":0,\"ids\":[]}").arg(clientid).arg(psessionid)));
    postData.append(QString("&clientid=%1&psessionid=%2").arg(clientid).arg(psessionid));
    request.setHeader(QNetworkRequest::ContentLengthHeader,postData.size());
    //httpAction=HttpAction::GetMsgAction;
    qDebug()<<"PollMsg"<<QDateTime::currentMSecsSinceEpoch()<<endl;
    httpPoll->post(request,postData);
}

void WebQQNet::httpPollFinished(QNetworkReply* reply){

    QByteArray replyData=reply->readAll();
    reply->deleteLater();
    QString replystr=QString::fromUtf8(replyData);
    QJsonDocument jsonDoc;
    QJsonObject jsonObj;
    QJsonArray jsonArray;
    int retcode=0;
    jsonDoc=QJsonDocument::fromJson(replyData);
    qDebug()<<"httpPollFinished jsonDoc="<<jsonDoc<<endl<<"replystr="<<replystr<<replystr.length()<<endl;
    if(jsonDoc.isObject()){
        jsonObj=jsonDoc.object();
        retcode=jsonObj.value("retcode").toDouble();
        if(retcode==0){
            WebQQ::mutex.lock();
            jsonArray=jsonObj.value("result").toArray();
            for(int i=0;i<jsonArray.size();i++){
                jsonObj=jsonArray.at(i).toObject();
                QQmsg* msg=new QQmsg();
                if(jsonObj.value("poll_type").toString()==QString("group_message")){
                    jsonObj=jsonObj.value("value").toObject();
                    msg->type=1;//群信息
                    //send_uin群内发言者TXUIN（通迅号）
                    msg->send_uin=QString::number(jsonObj.value("send_uin").toDouble(),'f',0);
                    //from_uin群TXUIN（群消息）或者好友TXUIN（私聊时）
                    msg->from_uin=QString::number(jsonObj.value("from_uin").toDouble(),'f',0);
                }else if(jsonObj.value("poll_type").toString()==QString("message")){
                    jsonObj=jsonObj.value("value").toObject();
                    msg->type=0;//好友信息
                    //from_uin群TXUIN（群消息）或者好友TXUIN（私聊时）
                    msg->from_uin=QString::number(jsonObj.value("from_uin").toDouble(),'f',0);
                }else{
                    delete msg;
                    break;
                }
                msg->time=jsonObj.value("time").toDouble();
                jsonArray=jsonObj.value("content").toArray();
                for(int i=1;i<jsonArray.size();i++){
                    if(jsonArray.at(i).isString()){
                        msg->content+=jsonArray.at(i).toString();
                    }else if(jsonArray.at(i).isArray()){
                        msg->content+="[图片]";
                    }
                }
                WebQQ::qqmsgs.append(msg);
                qDebug()<<msg->type<<msg->from_uin<<msg->time<<QDateTime::fromTime_t(msg->time)<<endl<<msg->content<<endl;
            }
            WebQQ::mutex.unlock();
            emit msgReceived();

        }else if(retcode==116){
            ptwebqq=jsonObj.value("p").toString();
            qDebug()<<"httpPollFinished retcode="<<retcode<<endl;
        }
    }
    if(retcode==0||retcode==102||retcode==116){
        this->pollMsg();
    }else{
        WebQQ::status=QQstatus::offline;
        emit sysMsg("掉线了，请重新登录。");
    }
}


void WebQQNet::getGroupMemberInfo(QString groupTXUIN){
    if(WebQQ::status==QQstatus::offline) return;
    if(!currGroupTXUIN.isEmpty()){
        return;
    }
    currGroupTXUIN=groupTXUIN;
    QString groupCode=WebQQ::qqGroups.value(groupTXUIN)->code;
    QNetworkRequest request;
    QString urlstr=QString("http://s.web2.qq.com/api/get_group_info_ext2?gcode=%1&cb=undefined&vfwebqq=%2&t=%3").arg(groupCode).arg(vfwebqq).arg(QDateTime::currentMSecsSinceEpoch());
    request.setUrl(QUrl(urlstr));
    request.setRawHeader("Accept","*/*");
    request.setRawHeader("Accept-Encoding","gzip,deflate,sdch");
    request.setRawHeader("Accept-Language","zh-CN,zh;q=0.8,en-US;q=0.6,en;q=0.4");
    request.setRawHeader("Accept-Charset","GBK,utf-8;q=0.7,*;q=0.3");
    request.setRawHeader("Connection","keep-alive");
    request.setRawHeader("Content-Type","utf-8");
    request.setRawHeader("Host","s.web2.qq.com");
    request.setRawHeader("Origin","http://d.web2.qq.com");
    request.setRawHeader("Referer","http://s.web2.qq.com/proxy.html?v=20110412001&callback=1&id=3");
    request.setRawHeader("User-Agent","Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/29.0.1547.62 Safari/537.36");

    qDebug()<<"getGroupMemberInfo"<<urlstr<<endl;
    httpExtInfo->get(request);
}

void WebQQNet::httpExtInfoFinished(QNetworkReply* reply){
    qDebug()<<"httpPollFinished"<<endl;
    QByteArray replyData=reply->readAll();

   // replyData.replace("\xe2\x80\xae","");//字符串中包含这几个字符，会莫名其妙的反转显示，非常奇特！
   //  replyData.replace("\xe2\x80\xa8","");//导致回车换行
//由于有些群成员名片是非法utf-8字符，导致json数据解析错误只好自己使用replystr.toUtf8()再转一次
    reply->deleteLater();
    QString replystr=QString::fromUtf8(replyData);
    QQgroup*pGroup=WebQQ::qqGroups.value(currGroupTXUIN,nullptr);
    if(pGroup==nullptr){
        qDebug()<<"httpExtInfoFinished 查无此群！"<<endl;
        return;
    }
    QJsonDocument jsonDoc;
    QJsonObject jsonObj;
    QJsonArray jsonArray;
    int retcode=0;
    QJsonParseError jsonError;
    jsonDoc=QJsonDocument::fromJson(replystr.toUtf8(),&jsonError);
    // qDebug()<<"httpPollFinished"<<jsonDoc<<endl;
    if(jsonDoc.isObject()){
        jsonObj=jsonDoc.object();
        retcode=jsonObj.value("retcode").toDouble();
        if(retcode==0){
            jsonObj=jsonObj.value("result").toObject();
            jsonArray=jsonObj.value("minfo").toArray();
            foreach(QJsonValue jv,jsonArray){
                QJsonObject jo=jv.toObject();
                QQfriend *f=new QQfriend();
                f->txuin=QString::number(jo.value("uin").toDouble(),'f',0);
                f->nick=jo.value("nick").toString();
                pGroup->members.insert(f->txuin,f);
            }
            jsonArray=jsonObj.value("cards").toArray();
            foreach(QJsonValue jv,jsonArray){
                QJsonObject jo=jv.toObject();
                QString txuin=QString::number(jo.value("muin").toDouble(),'f',0);;
                QQfriend* f=pGroup->members.value(txuin,nullptr);
                if(f==nullptr) f=new QQfriend();
                f->txuin=txuin;
                f->markname=jo.value("card").toString();
                pGroup->members.insert(f->txuin,f);
            }
        }else{//有时候返回 {"retcode":100000}，得不到成员信息
            pGroup->members.insert("0",new QQfriend);
        }
    }else{
        qDebug()<<"httpExtInfoFinished failure"<<replystr<<jsonError.errorString()<<endl<<replyData.data()<<endl;
    }
    emit sysMsg(QString("获取%1群成员信息完成，成员数量：%2").arg(WebQQ::qqGroups.value(currGroupTXUIN)->name)
                .arg(WebQQ::qqGroups.value(currGroupTXUIN)->members.size()));
    emit msgReceived();
    currGroupTXUIN.clear();
    qDebug()<<"httpExtInfoFinished /////////////////////////////////////////////////////////////////////////////"<<endl;
    //this->pollMsg();
}

void WebQQNet::httpFinished(QNetworkReply* reply){
    QByteArray replyData=reply->readAll();
    reply->deleteLater();
    QString replystr=QString::fromUtf8(replyData);
    QJsonDocument jsonDoc;
    QJsonObject jsonObj;
    QJsonArray jsonArray;
    bool isok;
    switch(httpAction){
    case HttpAction::NoAction:
        break;
    case HttpAction::CheckVerirycodeAction:
        if(replystr.indexOf("'0'")>-1){
            emit checkVerifycodeFinished(true,replystr.mid(18,4));
            httpAction=HttpAction::NoAction;
        }else{
            emit checkVerifycodeFinished(false,"");
        }
        replystr.chop(3);
        uinhexstr=replystr.right(32).remove("\\x");
        qDebug()<<replystr<<uinhexstr<<endl;
        break;
    case HttpAction::GetVerifyImgAction:
        qDebug()<<reply->header(QNetworkRequest::ContentTypeHeader).toString();
        emit getVerifyImgFinished(replyData);
        httpAction=HttpAction::NoAction;
        break;
    case HttpAction::LoginAction:
        qDebug()<<replystr<<endl;
        if(replystr.indexOf("ptuiCB('0'")>-1){
            ptwebqq=getCookie("ptwebqq");
            qDebug()<<"get cookie ptwebqq="<<ptwebqq<<endl;
        }
        if(!ptwebqq.isEmpty()){
            check_sig(replystr);
        }else{
            emit loginFinished(false,"登录失败！");
        }
        break;
    case HttpAction::Check_sigAction:
        login2();
        break;
    case HttpAction::Login2Action:
        qDebug()<<"Login2Action"<<replystr<<endl;
        jsonDoc=QJsonDocument::fromJson(replyData);
        if(jsonDoc.isNull()){
            emit loginFinished(false,"登录失败！");
        }else{
            jsonObj=jsonDoc.object();
            if(jsonObj.value("retcode").toDouble()==0){
                jsonObj=jsonObj.value("result").toObject();
                uin=jsonObj.value("uin").toDouble();
                qDebug()<<"uin="<<uin<<endl<<jsonObj;
                vfwebqq=jsonObj.value("vfwebqq").toString();
                psessionid=jsonObj.value("psessionid").toString();
                //qDebug()<<"登录成功！vfwebqq="<<vfwebqq<<"psessionid="<<psessionid<<endl;
                emit loginFinished(true,"成功！");
                this->getUserFriends();
            }
        }
        //qDebug()<<replystr<<endl;
        //qDebug()<<replystr<<endl<<http->cookieJar()->cookiesForUrl(QUrl("http://www.qq.com/"));
        break;
    case HttpAction::GetFriendInfoAction:
        qDebug()<<replystr<<endl;
        break;
    case HttpAction::GetUserFriendsAction:
        jsonDoc=QJsonDocument::fromJson(replyData);
        qDebug()<<"GetUserFriendsAction finished"<<endl;

        if(jsonDoc.isObject()){
            jsonObj=jsonDoc.object();
            if(jsonObj.value("retcode").toDouble()==0){
                jsonObj=jsonObj.value("result").toObject();
                jsonArray=jsonObj.value("marknames").toArray();
                for(int i=0;i<jsonArray.size();i++){
                    //jsonObj=jsonArray.at(i).toObject();
                    QQfriend* f=new QQfriend();
                    f->txuin=QString::number(jsonArray.at(i).toObject().value("uin").toDouble(),'f',0);
                    f->markname=jsonArray.at(i).toObject().value("markname").toString();
                    WebQQ::qqFriends.insert(f->txuin,f);
                }
                jsonArray=jsonObj.value("info").toArray();
                for(int i=0;i<jsonArray.size();i++){
                    //jsonObj=jsonArray.at(i).toObject();
                    QString txuin=QString::number(jsonArray.at(i).toObject().value("uin").toDouble(),'f',0);
                    QQfriend* f=WebQQ::qqFriends.value(txuin,nullptr);
                    if(f==nullptr) f=new QQfriend();
                    f->txuin=txuin;
                    f->nick=jsonArray.at(i).toObject().value("nick").toString();
                    WebQQ::qqFriends.insert(f->txuin,f);
                    QStandardItem *item=new QStandardItem();
                    item->setText(f->markname.isEmpty()?f->nick:QString("%1(%2)").arg(f->nick).arg(f->markname));
                    item->setData("FTX"+f->txuin,Qt::UserRole);
                    WebQQ::qqFriendListModel->appendRow(item);
                }
            }
            emit sysMsg("完成好友信息下载！");

        }else{
            emit sysMsg("好友信息下载失败！");
        }


        this->getGroupNameList();
        qDebug()<<WebQQ::qqFriends.size()<<WebQQ::qqFriends<<endl;
        break;
    case HttpAction::GetGroupNameListAction:
        qDebug()<<"GetGroupNameListAction finished"<<endl;
        jsonDoc=QJsonDocument::fromJson(replyData);
        qDebug()<<"GetGroupNameListAction"<<jsonDoc<<endl;
        if(jsonDoc.isObject()){
            jsonObj=jsonDoc.object();
            if(jsonObj.value("retcode").toDouble()==0){
                jsonObj=jsonObj.value("result").toObject();
                jsonArray=jsonObj.value("gnamelist").toArray();
                for(int i=0;i<jsonArray.size();i++){
                    jsonObj=jsonArray.at(i).toObject();
                    QQgroup* g=new QQgroup();
                    g->name=jsonObj.value("name").toString();
                    g->txuin=QString::number(jsonObj.value("gid").toDouble(),'f',0);
                    g->code=QString::number(jsonObj.value("code").toDouble(),'f',0);
                    WebQQ::qqGroups.insert(g->txuin,g);
                    qDebug()<<g->txuin<<g->name<<endl;
                    QStandardItem *item=new QStandardItem();
                    item->setText(g->name);
                    item->setData("GTX"+g->txuin,Qt::UserRole);
                    WebQQ::qqGroupListModel->appendRow(item);
                }
            }
            emit sysMsg("完成群信息下载！开始接收聊天信息......");

        }else{
            emit sysMsg("群信息下载失败！尝试接收聊天信息......");

        }
        this->pollMsg();
        httpAction=HttpAction::NoAction;
        break;
    case HttpAction::SendBuddyMsgAction:
        //{"retcode":0,"result":"ok"}
        isok=false;
        jsonDoc=QJsonDocument::fromJson(replyData);
        if(jsonDoc.isObject()){
            jsonObj=jsonDoc.object();
            if(jsonObj.value("retcode").toDouble()==0){
                isok=true;
            }
        }
        emit sendMsgFinished(currSendMsg.first,currSendMsg.second,isok);
        qDebug()<<"SendBuddyMsgAction"<<currSendMsg<<isok<<replystr<<endl;
        httpAction=HttpAction::NoAction;
        break;
    case HttpAction::SendGroupMsgAction:
        isok=false;
        jsonDoc=QJsonDocument::fromJson(replyData);
        if(jsonDoc.isObject()){
            jsonObj=jsonDoc.object();
            if(jsonObj.value("retcode").toDouble()==0){
                isok=true;
            }
        }
        //qDebug()<<"SendGroupMsgAction"<<currSendMsg<<isok<<endl;
        emit sendMsgFinished(currSendMsg.first,currSendMsg.second,isok);
        qDebug()<<"SendGroupMsgAction"<<currSendMsg<<isok<<replystr<<endl;
        httpAction=HttpAction::NoAction;
        break;
    case HttpAction::LoginOutAction:

        if(WebQQ::isclose){
            qApp->quit();
        }else{
            WebQQ::status=QQstatus::offline;

            emit sysMsg("账号已经注销登录。");
        }
        qDebug()<<"LoginOutAction"<<reply->error()<<replystr<<endl;
        httpAction=HttpAction::NoAction;
        break;
    default:
        break;
    }
    //qDebug()<<replystr<<endl<<http->cookieJar()->cookiesForUrl(QUrl("http://www.qq.com/"));

    reply->deleteLater();
}

void WebQQNet::checkVerifyCode(QString qqnum){
    QNetworkRequest request;
    request.setUrl(QUrl(QString("http://check.ptlogin2.qq.com/check?uin=%1&appid=1003903&js_ver=10041").arg(qqnum)));
    request.setRawHeader("Accept","text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    request.setRawHeader("Accept-Charset","GBK,utf-8;q=0.7,*;q=0.3");
    request.setRawHeader("Accept-Encoding","gzip,deflate,sdch");
    request.setRawHeader("Accept-Language","zh-CN,zh;q=0.8");
    request.setRawHeader("Cache-Control","max-age=0");
    request.setRawHeader("Connection","keep-alive");
    request.setRawHeader("Host","check.ptlogin2.qq.com");
    request.setRawHeader("User-Agent","Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/29.0.1547.62 Safari/537.36");
    httpAction=HttpAction::CheckVerirycodeAction;
    http->get(request);
}
void WebQQNet::getVerifyImg(QString qqnum){
    QNetworkRequest request;
    request.setUrl(QUrl(QString("http://captcha.qq.com/getimage?aid=1003903&uin=%1").arg(qqnum)));
    request.setRawHeader("Accept","text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    //request.setRawHeader("Accept-Charset","GBK,utf-8;q=0.7,*;q=0.3");
    request.setRawHeader("Accept-Encoding","gzip,deflate,sdch");
    request.setRawHeader("Accept-Language","zh-CN,zh;q=0.8");
    request.setRawHeader("Cache-Control","max-age=0");
    request.setRawHeader("Connection","keep-alive");
    request.setRawHeader("Host","captcha.qq.com");
    request.setRawHeader("User-Agent","Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/29.0.1547.62 Safari/537.36");
    httpAction=HttpAction::GetVerifyImgAction;
    http->get(request);
}
void WebQQNet::login(QString qqnum,QString qqpass, QString verifyCode){
    qDebug()<<getP(qqpass,verifyCode,uinhexstr);
    QNetworkRequest request;
    QString urlstr=QString("http://ptlogin2.qq.com/login?u=%1&p=%2&verifycode=%3&webqq_type=10&remember_uin=1&login2qq=1&aid=1003903&u1=%4&h=1&ptredirect=0&ptlang=2052&daid=164&from_ui=1&pttype=1&dumy=&fp=loginerroralert&action=2-11-12939&mibao_css=m_webqq&t=1&g=1&js_type=0&js_ver=10041&login_sig=Ip4V0YQezz6DEwpsE*-Cq*4NWVecBqWZYbNS22JCEQIT-sp20PyQtj8f-pqSIREa").arg(qqnum).arg(getP(qqpass,verifyCode,uinhexstr)).arg(verifyCode).arg("http%3A%2F%2Fweb2.qq.com%2Floginproxy.html%3Flogin2qq%3D1%26webqq_type%3D10");
    request.setUrl(QUrl(urlstr));
    // qDebug()<<urlstr<<endl;
    request.setRawHeader("Accept","text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    request.setRawHeader("Accept-Charset","GBK,utf-8;q=0.7,*;q=0.3");
    request.setRawHeader("Accept-Encoding","gzip,deflate,sdch");
    request.setRawHeader("Accept-Language","zh-CN,zh;q=0.8");
    request.setRawHeader("Cache-Control","max-age=0");
    request.setRawHeader("Connection","keep-alive");
    request.setRawHeader("Host","ptlogin2.qq.com");
    request.setRawHeader("User-Agent","Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/29.0.1547.62 Safari/537.36");
    httpAction=HttpAction::LoginAction;
    http->get(request);
}
void WebQQNet::check_sig(QString urlstr){
    urlstr=urlstr.mid(16);
    urlstr=urlstr.left(urlstr.indexOf('\''));
    //qDebug()<<urlstr<<endl;return;
    QNetworkRequest request;
    request.setUrl(QUrl(urlstr));
    // qDebug()<<urlstr<<endl;
    request.setRawHeader("Accept","text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    request.setRawHeader("Accept-Charset","GBK,utf-8;q=0.7,*;q=0.3");
    request.setRawHeader("Accept-Encoding","gzip,deflate,sdch");
    request.setRawHeader("Accept-Language","zh-CN,zh;q=0.8");
    request.setRawHeader("Cache-Control","max-age=0");
    request.setRawHeader("Connection","keep-alive");
    request.setRawHeader("Host","ptlogin2.qq.com");
    request.setRawHeader("User-Agent","Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/29.0.1547.62 Safari/537.36");
    httpAction=HttpAction::Check_sigAction;
    http->get(request);
}

void WebQQNet::login2(){
    QNetworkRequest request;
    request.setUrl(QUrl("http://d.web2.qq.com/channel/login2"));
    request.setRawHeader("Accept","*/*");
    request.setRawHeader("Accept-Encoding","gzip,deflate,sdch");
    request.setRawHeader("Accept-Language","zh-CN,zh;q=0.8,en-US;q=0.6,en;q=0.4");
    request.setRawHeader("Accept-Charset","GBK,utf-8;q=0.7,*;q=0.3");
    request.setRawHeader("Connection","keep-alive");
    request.setRawHeader("Content-Type","application/x-www-form-urlencoded");
    request.setRawHeader("Host","d.web2.qq.com");
    request.setRawHeader("Origin","http://d.web2.qq.com");
    request.setRawHeader("Referer","http://d.web2.qq.com/proxy.html?v=20110331002&callback=2");
    request.setRawHeader("User-Agent","Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/29.0.1547.62 Safari/537.36");
    QByteArray postData;
    postData.append("r="+QUrl::toPercentEncoding(QString("{\"status\":\"online\",\"ptwebqq\":\"%1\",\"passwd_sig\":\"\",\"clientid\":\"%2\",\"psessionid\":null}").arg(ptwebqq).arg(clientid)));
    postData.append(QString("&clientid=%1&psessionid=null").arg(clientid));
    request.setHeader(QNetworkRequest::ContentLengthHeader,postData.size());
    httpAction=HttpAction::Login2Action;
    qDebug()<<postData<<endl;
    http->post(request,postData);
}
void WebQQNet::loginout(){

    emit  sysMsg("正在注销登录信息...........");
    QNetworkRequest request;
    QString urlstr=QString("http://d.web2.qq.com/channel/change_status2?newstatus=offline&clientid=%1&psessionid=%2&t=%3").arg(clientid).arg(psessionid).arg(QDateTime::currentMSecsSinceEpoch());
    request.setUrl(QUrl(urlstr));
    // qDebug()<<urlstr<<endl;
    request.setRawHeader("Accept","text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    request.setRawHeader("Accept-Charset","GBK,utf-8;q=0.7,*;q=0.3");
    request.setRawHeader("Accept-Encoding","gzip,deflate,sdch");
    request.setRawHeader("Accept-Language","zh-CN,zh;q=0.8");
    request.setRawHeader("Cache-Control","max-age=0");
    request.setRawHeader("Connection","keep-alive");
    request.setRawHeader("Host","d.web2.qq.com");
    request.setRawHeader("Referer","http://d.web2.qq.com/proxy.html?v=20110331002&callback=1&id=3");
    request.setRawHeader("User-Agent","Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/29.0.1547.62 Safari/537.36");
    httpAction=HttpAction::LoginOutAction;
    http->get(request);
}

void WebQQNet::getFriendInfo(QString qqnum){
    QNetworkRequest request;
    QString urlstr=QString("http://s.web2.qq.com/api/get_friend_info2?tuin=%1&verifysession=&code=&vfwebqq=%2&t=1377829770859").arg(qqnum).arg(vfwebqq);
    request.setUrl(QUrl(urlstr));
    // qDebug()<<urlstr<<endl;
    request.setRawHeader("Accept","text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    request.setRawHeader("Accept-Charset","GBK,utf-8;q=0.7,*;q=0.3");
    request.setRawHeader("Accept-Encoding","gzip,deflate,sdch");
    request.setRawHeader("Accept-Language","zh-CN,zh;q=0.8");
    request.setRawHeader("Cache-Control","max-age=0");
    request.setRawHeader("Connection","keep-alive");
    request.setRawHeader("Host","s.web2.qq.com");
    request.setRawHeader("Referer","http://d.web2.qq.com/proxy.html?v=20110412001&callback=2");
    request.setRawHeader("User-Agent","Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/29.0.1547.62 Safari/537.36");
    httpAction=HttpAction::GetFriendInfoAction;
    http->get(request);
}
void WebQQNet::getUserFriends(){

    emit sysMsg("获取好友信息...........");
    QNetworkRequest request;
    request.setUrl(QUrl("http://s.web2.qq.com/api/get_user_friends2"));
    request.setRawHeader("Accept","*/*");
    request.setRawHeader("Accept-Encoding","gzip,deflate,sdch");
    request.setRawHeader("Accept-Language","zh-CN,zh;q=0.8,en-US;q=0.6,en;q=0.4");
    request.setRawHeader("Accept-Charset","GBK,utf-8;q=0.7,*;q=0.3");
    request.setRawHeader("Connection","keep-alive");
    request.setRawHeader("Content-Type","application/x-www-form-urlencoded");
    request.setRawHeader("Host","s.web2.qq.com");
    request.setRawHeader("Origin","http://s.web2.qq.com");
    request.setRawHeader("Referer","http://s.web2.qq.com/proxy.html?v=20110412001&callback=1&id=3");
    request.setRawHeader("User-Agent","Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/29.0.1547.62 Safari/537.36");
    QByteArray postData;
    postData.append("r="+QUrl::toPercentEncoding(QString("{\"h\":\"hello\",\"hash\":\"%1\",\"vfwebqq\":\"%2\"}").arg(getHash(uin,ptwebqq)).arg(vfwebqq)));
    request.setHeader(QNetworkRequest::ContentLengthHeader,postData.size());
    httpAction=HttpAction::GetUserFriendsAction;
    qDebug()<<postData<<endl;
    http->post(request,postData);
}
void WebQQNet::getGroupNameList(){

    emit sysMsg("获取群信息...........");
    QNetworkRequest request;
    request.setUrl(QUrl("http://s.web2.qq.com/api/get_group_name_list_mask2"));
    request.setRawHeader("Accept","*/*");
    request.setRawHeader("Accept-Encoding","gzip,deflate,sdch");
    request.setRawHeader("Accept-Language","zh-CN,zh;q=0.8,en-US;q=0.6,en;q=0.4");
    request.setRawHeader("Accept-Charset","GBK,utf-8;q=0.7,*;q=0.3");
    request.setRawHeader("Connection","keep-alive");
    request.setRawHeader("Content-Type","application/x-www-form-urlencoded");
    request.setRawHeader("Host","s.web2.qq.com");
    request.setRawHeader("Origin","http://s.web2.qq.com");
    request.setRawHeader("Referer","http://s.web2.qq.com/proxy.html?v=20110412001&callback=1&id=3");
    request.setRawHeader("User-Agent","Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/29.0.1547.62 Safari/537.36");
    QByteArray postData;
    postData.append("r="+QUrl::toPercentEncoding(QString("{\"vfwebqq\":\"%1\"}").arg(vfwebqq)));
    request.setHeader(QNetworkRequest::ContentLengthHeader,postData.size());
    httpAction=HttpAction::GetGroupNameListAction;
    qDebug()<<postData<<endl;
    http->post(request,postData);
}

void WebQQNet::sendMsg(QString txuin, QString msg){
    if(httpAction==HttpAction::NoAction){
        QString txuinflag=txuin.left(3);
        currSendMsg.first=txuin;
        currSendMsg.second=msg;
        //对json 特殊字符进行替换
        qDebug()<<"对json 特殊字符进行替换__前"<<msg<<endl;
        msg.replace("\\", "\\\\")
                //.replace("\'", "")
                .replace("\t", " ")
                .replace("\r", " ")
                .replace("\n", "\\\\n")
                .replace("\"", "'");
        qDebug()<<"对json 特殊字符进行替换__后"<<msg<<endl;
        if(txuinflag=="FTX"){
            WebQQ::webQQNet->sendBuddyMsg(txuin.mid(3),msg);
        }else if(txuinflag=="GTX"){
            WebQQ::webQQNet->sendGroupMsg(txuin.mid(3),msg);
        }

    }else{
        sendMsgList.append(QPair<QString,QString>(txuin,msg));
        if(!timer->isActive()){
            timer->start(1000);
        }
    }
}
void WebQQNet::sendMsg(){
    if(sendMsgList.isEmpty()){
        timer->stop();
        return;
    }
    if(httpAction==HttpAction::NoAction){
        currSendMsg=sendMsgList.takeLast();
        sendMsg(currSendMsg.first,currSendMsg.second);
    }
}

void WebQQNet::sendBuddyMsg(QString uin, QString msg){
    QNetworkRequest request;
    request.setUrl(QUrl("http://d.web2.qq.com/channel/send_buddy_msg2"));
    request.setRawHeader("Accept","*/*");
    request.setRawHeader("Accept-Encoding","gzip,deflate,sdch");
    request.setRawHeader("Accept-Language","zh-CN,zh;q=0.8,en-US;q=0.6,en;q=0.4");
    request.setRawHeader("Accept-Charset","GBK,utf-8;q=0.7,*;q=0.3");
    request.setRawHeader("Connection","keep-alive");
    request.setRawHeader("Content-Type","application/x-www-form-urlencoded");
    request.setRawHeader("Host","d.web2.qq.com");
    request.setRawHeader("Origin","http://d.web2.qq.com");
    request.setRawHeader("Referer","http://d.web2.qq.com/proxy.html?v=20110331002&callback=1&id=2");
    request.setRawHeader("User-Agent","Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/29.0.1547.62 Safari/537.36");
    QByteArray postData;
    QString tmpdata=QString("{\"to\":%1,\"face\":492,\"content\":\"[\\\"%2\\\",\\\"\\\",[\\\"font\\\",{\\\"name\\\":\\\"宋体\\\",\\\"size\\\":\\\"9\\\",\\\"style\\\":[0,0,0],\\\"color\\\":\\\"FF0000\\\"}]]\",\"msg_id\":%3,\"clientid\":\"%4\",\"psessionid\":\"%5\"}").arg(uin).arg(msg).arg(msgId++).arg(clientid).arg(psessionid);
    qDebug()<<tmpdata<<endl;
    postData.append("r="+QUrl::toPercentEncoding(tmpdata));
    postData.append(QString("&clientid=%1&psessionid=%2").arg(clientid).arg(psessionid));
    request.setHeader(QNetworkRequest::ContentLengthHeader,postData.size());
    httpAction=HttpAction::SendBuddyMsgAction;
    qDebug()<<"sendBuddyMsg"<<msg<<endl;
    http->post(request,postData);

}
void WebQQNet::sendGroupMsg(QString groupuin, QString msg){
    QNetworkRequest request;
    request.setUrl(QUrl("http://d.web2.qq.com/channel/send_qun_msg2"));
    request.setRawHeader("Accept","*/*");
    request.setRawHeader("Accept-Encoding","gzip,deflate,sdch");
    request.setRawHeader("Accept-Language","zh-CN,zh;q=0.8,en-US;q=0.6,en;q=0.4");
    request.setRawHeader("Accept-Charset","GBK,utf-8;q=0.7,*;q=0.3");
    request.setRawHeader("Connection","keep-alive");
    request.setRawHeader("Content-Type","application/x-www-form-urlencoded");
    request.setRawHeader("Host","d.web2.qq.com");
    request.setRawHeader("Origin","http://d.web2.qq.com");
    request.setRawHeader("Referer","http://d.web2.qq.com/proxy.html?v=20110331002&callback=1&id=3");
    request.setRawHeader("User-Agent","Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/29.0.1547.62 Safari/537.36");
    QByteArray postData;
    QString tmpdata=QString("{\"group_uin\":%1,\"content\":\"[\\\"%2\\\",\\\"\\\",[\\\"font\\\",{\\\"name\\\":\\\"宋体\\\",\\\"size\\\":\\\"9\\\",\\\"style\\\":[0,0,0],\\\"color\\\":\\\"FF0000\\\"}]]\",\"msg_id\":%3,\"clientid\":\"%4\",\"psessionid\":\"%5\"}").arg(groupuin).arg(msg).arg(msgId++).arg(clientid).arg(psessionid);
    qDebug()<<tmpdata<<endl;
    postData.append("r="+QUrl::toPercentEncoding(tmpdata));
    postData.append(QString("&clientid=%1&psessionid=%2").arg(clientid).arg(psessionid));
    request.setHeader(QNetworkRequest::ContentLengthHeader,postData.size());
    httpAction=HttpAction::SendGroupMsgAction;
    qDebug()<<"sendGroupMsg"<<msg<<endl;
    http->post(request,postData);
}

QString WebQQNet::getP(QString qqpass, QString verifyCode, QString uinhexstr){
    QByteArray qqpassbyte=QCryptographicHash::hash(qqpass.toLatin1(),QCryptographicHash::Md5);
    QByteArray uinhexbyte = QByteArray::fromHex(uinhexstr.toLatin1());
    qDebug()<<"uinhexbyte="<<uinhexbyte<<"uinstr="<<uinhexstr<<endl;
    QByteArray qqpassuinbyte=QCryptographicHash::hash(qqpassbyte+uinhexbyte,QCryptographicHash::Md5).toHex().toUpper();
    QByteArray byteP=QCryptographicHash::hash(qqpassuinbyte+verifyCode.toUpper().toLatin1(),QCryptographicHash::Md5).toHex().toUpper();
    return QString::fromLatin1(byteP);
}

QString WebQQNet::getCookie(const QString &name){
    QString retstr;
    foreach(QNetworkCookie cookie , http->cookieJar()->cookiesForUrl(QUrl("http://www.qq.com/"))){
        // qDebug()<<cookie<<endl;
        if(cookie.name()==name){
            retstr=cookie.value();
        }
    }
    return retstr;
}
QString WebQQNet::getHash(qint32 uin, QString ptwebqq){
    QVector<qint8> r;
    r.append(uin>> 24 &255);r.append(uin>> 16 &255);r.append(uin>> 8 &255); r.append(uin&255);
    QVector<int>ja;
    for(int e=0;e<ptwebqq.length();++e){
        ja.push_back(ptwebqq.at(e).toLatin1());
        //qDebug()<<a.at(e)<<ja.at(e);
    }
    QVector<QPair<int,int> > e;
    for(e.push_back(QPair<int,int>(0,ja.size()-1));e.size()>0;){
        QPair<int,int>c=e.takeLast();
        if(!(c.first>=c.second||c.first<0||c.second>=ja.size())){
            if(c.first+1==c.second){
                if(ja[c.first]>ja[c.second]){
                    int l=ja[c.first];
                    ja[c.first]=ja[c.second];
                    ja[c.second]=l;
                }
            }else{
                int l,J,f;
                for(l=c.first,J=c.second, f=ja[c.first];c.first<c.second;){
                    for(;c.first<c.second&&ja[c.second]>=f;){
                        c.second--,r[0]=r[0]+(3&255);
                    }
                    (c.first<c.second)&&(ja[c.first]=ja[c.second],c.first++,r[1]=r[1]*13+(43&255));
                    for(;c.first<c.second&&ja[c.first]<=f;){
                        c.first++,r[2]=r[2]-(3&255);
                    }
                    c.first<c.second&&(ja[c.second]=ja[c.first],c.second--,r[3]=(r[0]^r[1]^r[2]^(r[3]+1))&255);
                }
                ja[c.first]=f;
                e.push_back(QPair<int,int>(l,c.first-1));
                e.push_back(QPair<int,int>(c.first+1,J));
            }
        }
    }
    char j[]="0123456789ABCDEF";
    QString hash;
    for(int c=0;c<r.size();c++){
        hash.append(j[r[c]>>4&15]),hash.append(j[r[c]&15]);
    }
    return hash;
}
/*
 *
错误代码说明:
ptuiCB('7','0','','0','很遗憾，网络连接出现异常，请您稍后再试。(612369104)');
cookie或qq号码问题
ptuiCB('0','0','http://aq.qq.com/cn/services/abnormal/abnormal_index? 。。。。。。！');
帐号冻结
{"retcode":102 ,"errmsg":""}
正常连接、没有消息。
{"retcode":103,"errmsg":""}
掉线
{"retcode":108,"errmsg":""}
{"retcode":114,"errmsg":""}
{"retcode":121,"t":"0"}
掉线
{"retcode":122,"errmsg":"wrong web client3"}
{"retcode":100001}
 群编号有问题
{"retcode":100006,"errmsg":""}
 *
 *
 */
