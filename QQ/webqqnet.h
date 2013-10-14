/****************************************************************************
**
** Copyright (C) 2013 Jeffrey Lee.
** All rights reserved.
** Contact: vipjeffreylee@gmail.com
** Please keep the author contact information.
** 2013-10-10
**
****************************************************************************/
#ifndef WEBQQNET_H
#define WEBQQNET_H
#include <QObject>
#include<QTimer>
#include<QPair>
class QNetworkAccessManager;
class QNetworkRequest;
class QNetworkReply;
class QNetworkCookieJar;
enum class HttpAction{NoAction,CheckVerirycodeAction,GetVerifyImgAction,LoginAction,LoginOutAction,Check_sigAction,Login2Action,
                      GetFriendInfoAction,GetUserFriendsAction,GetGroupNameListAction,GetMsgAction,
                     SendBuddyMsgAction,SendGroupMsgAction};
class WebQQNet : public QObject
{
    Q_OBJECT
public:
    explicit WebQQNet(QObject *parent = 0);
    
signals:
    void checkVerifycodeFinished(bool getCode,QString code);
    void getVerifyImgFinished(const QByteArray imgdata);
    void loginFinished(bool loginok,QString message);
    void msgReceived();
    void sendMsgFinished(QString txuin,QString msg,bool isok);
    void sysMsg(QString msg);
public slots:
    void httpFinished(QNetworkReply*);
    void httpPollFinished(QNetworkReply*);
    void httpExtInfoFinished(QNetworkReply*);
    void sendMsg();
public:
    void checkVerifyCode(QString qqnum);
    void getVerifyImg(QString qqnum);
    void login(QString qqnum,QString qqpass,QString verifyCode);
    void loginout();
    void getFriendInfo(QString qqnum);
    void getUserFriends();
    void getGroupNameList();
    void sendMsg(QString txuin,QString msg);
    void getGroupMemberInfo(QString groupTXUIN);
    qint32 uin;
private:
    void pollMsg();

    QString currGroupTXUIN;
    QTimer *timer;
    QList<QPair<QString,QString>> sendMsgList;
    QPair<QString,QString> currSendMsg;
    void sendBuddyMsg(QString uin,QString msg);
    void sendGroupMsg(QString groupuin,QString msg);
    QNetworkAccessManager * http;
    QNetworkAccessManager * httpPoll;
    QNetworkAccessManager * httpExtInfo;//获得群内成员信息等
    QNetworkCookieJar * pCookieJar;
    HttpAction httpAction;
    QString getCookie(const QString &name);
    QString uinhexstr;
    QString getP(QString qqpass,QString verifyCode,QString uinhexstr);
    QString getHash(qint32 uin,QString ptwebqq);
    void login2();
    void check_sig(QString urlstr);
    qint32 msgId;
private://cookie clientid
    QString ptwebqq;
    QString vfwebqq;
    QString psessionid;
    QString clientid;
};

#endif // WEBQQNET_H
