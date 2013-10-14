#include "logindlg.h"
#include "ui_logindlg.h"
#include "webqqnet.h"
#include <QDebug>
#include "webqq.h"
LoginDlg::LoginDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDlg)
{
    ui->setupUi(this);
    connect(ui->qqnum,SIGNAL(editingFinished()),this,SLOT(checkVerifycode()));
    connect(ui->rtnButton,SIGNAL(clicked()),this,SLOT(rtnButtonClk()));

    connect(WebQQ::webQQNet,SIGNAL(checkVerifycodeFinished(bool,QString)),this,SLOT(checkVerifycodeFinished(bool,QString)));
    connect(WebQQ::webQQNet,SIGNAL(getVerifyImgFinished(QByteArray)),this,SLOT(getVerifyImgFinished(QByteArray)));
    connect(WebQQ::webQQNet,SIGNAL(loginFinished(bool,QString)),this,SLOT(loginFinished(bool,QString)));
    ui->verifyCode->hide();ui->verifyImg->hide();
}

LoginDlg::~LoginDlg()
{
    delete ui;
}
void LoginDlg::checkVerifycode(){
    qDebug()<<"checkVerifycode"<<endl;
    if(ui->qqnum->text().trimmed().isEmpty()){
        return;
    }
    WebQQ::webQQNet->checkVerifyCode(ui->qqnum->text());
}
void LoginDlg::checkVerifycodeFinished(bool getCode, QString code){
    qDebug()<<getCode<<code<<endl;
    if(getCode){
        ui->verifyCode->setText(code);
        ui->verifyImg->setText("不需要验证码");
        ui->verifyCode->hide();ui->verifyImg->hide();
    }else{
        ui->verifyCode->clear();
        WebQQ::webQQNet->getVerifyImg(ui->qqnum->text().trimmed());
    }
}
void LoginDlg::getVerifyImgFinished(QByteArray imgdata){
    //qDebug()<<"getVerifyImgFinished"<<endl;
    //qDebug()<<imgdata.length()<<endl;
    QPixmap p;
    p.loadFromData(imgdata);
    ui->verifyImg->setPixmap(p);
    ui->verifyCode->show();ui->verifyImg->show();
}
void LoginDlg::rtnButtonClk(){
    //QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    WebQQ::webQQNet->login(ui->qqnum->text().trimmed(),ui->qqpass->text().trimmed(),ui->verifyCode->text().trimmed());
}
void LoginDlg::loginFinished(bool loginok, QString message){
    qDebug()<<loginok<<message<<endl;
    if(loginok){
        WebQQ::qqnum=ui->qqnum->text().trimmed();
        WebQQ::loginFinished(loginok);
    }else{
        checkVerifycode();
    }
}

