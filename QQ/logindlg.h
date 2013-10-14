/****************************************************************************
**
** Copyright (C) 2013 Jeffrey Lee.
** All rights reserved.
** Contact: vipjeffreylee@gmail.com
** Please keep the author contact information.
** 2013-10-10
**
****************************************************************************/
#ifndef LOGINDLG_H
#define LOGINDLG_H

#include <QDialog>

namespace Ui {
class LoginDlg;
}
class LoginDlg : public QDialog
{
    Q_OBJECT
    
public:
    explicit LoginDlg(QWidget *parent = 0);
    ~LoginDlg();
    
private:
    Ui::LoginDlg *ui;
public slots:
    void checkVerifycode();
    void checkVerifycodeFinished(bool getCode,QString code);
    void getVerifyImgFinished(const QByteArray imgdata);
    void loginFinished(bool loginok,QString message);
    void rtnButtonClk();
private:
    void login();
    QString uinhex;
};

#endif // LOGINDLG_H
