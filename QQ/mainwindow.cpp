/****************************************************************************
**
** Copyright (C) 2013 Jeffrey Lee.
** All rights reserved.
** Contact: vipjeffreylee@gmail.com
** Please keep the author contact information.
** 2013-10-10
**
****************************************************************************/
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QDateTime>
#include<QMessageBox>
#include<QDir>
#include<QPluginLoader>
#include<QCheckBox>
#include<QButtonGroup>
#include "webqq.h"
#include<QDebug>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->sendButton,SIGNAL(clicked()),this,SLOT(sendMsg()));
    connect(ui->textBrowser,SIGNAL(selectionChanged()),this,SLOT(selChgGetTXUIN()));
    connect(WebQQ::webQQNet,SIGNAL(msgReceived()),this,SLOT(msgReceived()));
    connect(WebQQ::webQQNet,SIGNAL(sendMsgFinished(QString,QString,bool)),this,SLOT(sendMsgFinished(QString,QString,bool)));
    connect(WebQQ::webQQNet,SIGNAL(sysMsg(QString)),this,SLOT(sysMsg(QString)));

    ui->qqGroupList->setModel(WebQQ::qqGroupListModel);
    ui->qqFriendList->setModel(WebQQ::qqFriendListModel);
    connect(ui->qqGroupList,SIGNAL(pressed(QModelIndex)),this,SLOT(qqGroupSelected(QModelIndex)));
    connect(ui->qqFriendList,SIGNAL(pressed(QModelIndex)),this,SLOT(qqFriendSelected(QModelIndex)));
    if (!loadPlugin()) {
        //QMessageBox::information(this, "Error", "Could not load the robot plugin");
        ;//没有机器人也不应该报错
    }
    ui->plainTextEdit->installEventFilter(this);
}
MainWindow::~MainWindow()
{
    delete ui;
}
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->plainTextEdit) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Return && (keyEvent->modifiers() & Qt::ControlModifier)){
                sendMsg();
                return true;
            }
        }
        return false;
    } else {
        // pass the event on to the parent class
        return QMainWindow::eventFilter(obj, event);
    }
}
void MainWindow::closeEvent(QCloseEvent *event){
    if(WebQQ::status==QQstatus::online){
        WebQQ::isclose=true;
        WebQQ::webQQNet->loginout();
        event->ignore();
    }else{
        event->accept();
    }
}
void MainWindow::qqFriendSelected(QModelIndex mi){
    ui->txuinLineEdit->setText(mi.data(Qt::UserRole).toString());
    currModelIndex=mi;
    QQfriend *f=WebQQ::qqFriends.value(ui->txuinLineEdit->text().mid(3),nullptr);
    if(f!=nullptr){
        ui->receiverName->setText(f->markname.isEmpty()?f->nick:QString("%1(%2)")
                                                        .arg(f->nick)
                                                        .arg(f->markname));
        robotButtonGroup->button(0)->setChecked(true);
        for(int i=0;i<robots.size();i++){
            if(f->robots.contains(i)){
                robotButtonGroup->button(i+1)->setChecked(true);
            }else{
                robotButtonGroup->button(i+1)->setChecked(false);
                robotButtonGroup->button(0)->setChecked(false);
            }
        }
    }
}

void MainWindow::qqGroupSelected(QModelIndex mi){
    //ui->receiverName->setText(mi.data().toString());
    ui->txuinLineEdit->setText(mi.data(Qt::UserRole).toString());
    currModelIndex=mi;
    QQgroup *g=WebQQ::qqGroups.value(ui->txuinLineEdit->text().mid(3),nullptr);
    if(g!=nullptr){
        ui->receiverName->setText(g->name);
        robotButtonGroup->button(0)->setChecked(true);
        for(int i=0;i<robots.size();i++){
            if(g->robots.contains(i)){
                robotButtonGroup->button(i+1)->setChecked(true);
            }else{
                robotButtonGroup->button(i+1)->setChecked(false);
                robotButtonGroup->button(0)->setChecked(false);
            }
        }
    }

}

bool MainWindow::loadPlugin()
{
    QDir pluginsDir(qApp->applicationDirPath());
#if defined(Q_OS_WIN)
    if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release")
        pluginsDir.cdUp();
#elif defined(Q_OS_MAC)
    if (pluginsDir.dirName() == "MacOS") {
        pluginsDir.cdUp();
        pluginsDir.cdUp();
        pluginsDir.cdUp();
    }
#endif
    pluginsDir.cd("plugins");
    foreach (QString fileName, pluginsDir.entryList(QStringList()<<"*.so"<<"*.dll",QDir::Files)) {
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        qDebug()<<fileName<<pluginsDir.absoluteFilePath(fileName)<<endl;
        QObject *plugin = pluginLoader.instance();
        if (plugin) {
            RobotInterface *robotInterface = qobject_cast<RobotInterface *>(plugin);
            if (robotInterface){
                robots.append(robotInterface);
            }
        }
    }
    if(!robots.isEmpty()){
        robotButtonGroup=new QButtonGroup(this);
        robotButtonGroup->setExclusive(false);
        QCheckBox *checkbox=new QCheckBox(this);
        checkbox->setText("所有机器人");
        ui->robotLayout->addWidget(checkbox);
        int id=0;
        robotButtonGroup->addButton(checkbox,id);
        foreach (auto robot,robots) {
            checkbox=new QCheckBox(this);
            checkbox->setText(robot->name());
            ui->robotLayout->addWidget(checkbox);
            robotButtonGroup->addButton(checkbox,++id);
        }
        ui->robotLayout->addStretch();
        connect(robotButtonGroup,SIGNAL(buttonClicked(int)),this,SLOT(robotsSelected(int)));
    }
    return !robots.isEmpty();
}
void MainWindow::robotsSelected(int id){
    if(id==0){
        foreach(auto button,robotButtonGroup->buttons()){
            button->setChecked(robotButtonGroup->button(id)->isChecked());
        }
    }
    QVector<int> tmpvector;
    bool isallsel=true;
    for(int i=1;i<robotButtonGroup->buttons().length();i++){
        if(robotButtonGroup->button(i)->isChecked()){
            tmpvector<<i-1;
            qDebug()<<robots.at(i-1)->name()<<endl;
        }else{
            isallsel=false;
        }
    }
    robotButtonGroup->button(0)->setChecked(isallsel);
    QString txuin=ui->txuinLineEdit->text();
    if(txuin.isEmpty()) return;
    QString txuinflag=ui->txuinLineEdit->text().left(3);
    if(txuinflag=="FTX"){
        txuin=txuin.mid(3);
        QQfriend *f=WebQQ::qqFriends.value(txuin);
        if(f) {
            f->robots=tmpvector;
            WebQQ::qqFriendListModel->itemFromIndex(currModelIndex)->setText(QString("%1%2").arg(f->markname.isEmpty()?f->nick:QString("%1(%2)")
                                                                                                                       .arg(f->nick)
                                                                                                                       .arg(f->markname))
                                                                            .arg(f->robots.size()>0?QString("(%1个机器人)").arg(f->robots.size()):""));
        }
    }else if(txuinflag=="GTX"){
        txuin=txuin.mid(3);
        QQgroup *g=WebQQ::qqGroups.value(txuin);
        if(g) {
            g->robots=tmpvector;
            WebQQ::qqGroupListModel->itemFromIndex(currModelIndex)->setText(QString("%1%2").arg(g->name)
                                                                            .arg(g->robots.size()>0?QString("(%1个机器人)").arg(g->robots.size()):""));
        }
    }
}
void MainWindow::selChgGetTXUIN(){
    QString selectedtxt=ui->textBrowser->textCursor().selectedText().trimmed();
    if(selectedtxt.indexOf("TX")>-1){
        QString txuinflag=selectedtxt.left(3);
        QString txuin=selectedtxt.mid(3);
        if(txuinflag=="FTX"){
            QQfriend *f=WebQQ::qqFriends.value(txuin,nullptr);
            if(f==nullptr){
                ui->txuinLineEdit->setText(selectedtxt);
                ui->receiverName->setText("不是好友关系，不能为其指派机器人");
                foreach (auto button, robotButtonGroup->buttons()) {
                    button->setChecked(false);
                }
            }else{
                ui->txuinLineEdit->setText(selectedtxt);
                ui->receiverName->setText(f->markname.isEmpty()?f->nick:QString("%1(%2)")
                                                                .arg(f->nick)
                                                                .arg(f->markname));
                for (int row = 0; row < WebQQ::qqFriendListModel->rowCount(); ++row) {
                    QStandardItem *item=WebQQ::qqFriendListModel->item(row);
                    if(item->data(Qt::UserRole)==selectedtxt){
                        ui->qqFriendList->selectionModel()->clearSelection();
                        qDebug()<<"getTXUIN currModelIndex1"<<currModelIndex.data()<<endl;
                        currModelIndex=WebQQ::qqFriendListModel->indexFromItem(item);
                        qDebug()<<"getTXUIN currModelIndex2"<<currModelIndex.data()<<endl;
                        ui->qqFriendList->pressed(currModelIndex);
                        ui->qqFriendList->selectionModel()->select(currModelIndex,QItemSelectionModel::Select);
                        break;
                    }
                }

            }
        }else if(txuinflag=="GTX"){
            QQgroup *g=WebQQ::qqGroups.value(txuin,nullptr);
            if(g!=nullptr){
                ui->txuinLineEdit->setText(selectedtxt);
                ui->receiverName->setText(g->name);
                for (int row = 0; row < WebQQ::qqGroupListModel->rowCount(); ++row) {
                    QStandardItem *item=WebQQ::qqGroupListModel->item(row);
                    if(item->data(Qt::UserRole)==selectedtxt){
                        ui->qqGroupList->selectionModel()->clearSelection();
                        qDebug()<<"getTXUIN currModelIndex1"<<currModelIndex.data()<<endl;
                        currModelIndex=WebQQ::qqGroupListModel->indexFromItem(item);
                        qDebug()<<"getTXUIN currModelIndex2"<<currModelIndex.data()<<endl;
                        ui->qqGroupList->pressed(currModelIndex);
                        ui->qqGroupList->selectionModel()->select(currModelIndex,QItemSelectionModel::Select);
                        break;
                    }
                }
            }else{
                ui->txuinLineEdit->setText("");
                ui->receiverName->setText("");
                foreach (auto button, robotButtonGroup->buttons()) {
                    button->setChecked(false);
                }
            }


        }
    }
}

void MainWindow::msgReceived(){
    WebQQ::mutex.lock();
    QList<QQmsg*>::Iterator iMsg;
    QQmsg * pMsg;
    for(iMsg=WebQQ::qqmsgs.begin();iMsg!=WebQQ::qqmsgs.end();){
        pMsg=*iMsg;
        if(pMsg->type==1){
            if(WebQQ::qqGroups.value(pMsg->from_uin)->members.isEmpty()&&WebQQ::status==QQstatus::online){
                qDebug()<<WebQQ::qqGroups.value(pMsg->from_uin)->name<<"下载群成员信息....."<<endl;
                WebQQ::webQQNet->getGroupMemberInfo(pMsg->from_uin);
                ++iMsg;
                continue;//群成员信息还未获得，本条信息先保留，成员信息拉取后再显示，才可保证提供发送者信息
            }
        }
        ui->textBrowser->append(formatMsg(pMsg));
        ////////////////////////////////////////////机器人
        QVector<int> robotIds;
        QString fromTXUIN=pMsg->from_uin;
        MsgSender msgSender;
        if(pMsg->type==0){
            fromTXUIN.prepend("FTX");
            QQfriend *f=WebQQ::qqFriends.value(pMsg->from_uin);
            if(f){
                robotIds=f->robots;
                msgSender.friendTXUIN=fromTXUIN;
                msgSender.friendName=f->markname.isEmpty()?f->nick:QString("%1(%2)")
                                                           .arg(f->nick)
                                                           .arg(f->markname);
            }
        }else if(pMsg->type==1){
            fromTXUIN.prepend("GTX");
            QQgroup *p=WebQQ::qqGroups.value(pMsg->from_uin);
            if(p){
                robotIds=p->robots;
                msgSender.groupTXUIN=fromTXUIN;
                msgSender.groupName=p->name;
                QQfriend *f=p->members.value(pMsg->send_uin,nullptr);
                if(f!=nullptr){
                    msgSender.friendTXUIN="FTX"+f->txuin;
                    msgSender.friendName=f->markname.isEmpty()?f->nick:QString("%1(%2)")
                                                               .arg(f->nick)
                                                               .arg(f->markname);
                }
            }
        }
        foreach(int id ,robotIds){
            ReplyMsg rmsg=robots.at(id)->listenandsay(msgSender,pMsg->content);
            if(rmsg.content.isEmpty()){
                continue;
            }else{
                WebQQ::webQQNet->sendMsg(rmsg.recTXUIN,rmsg.content);
            }
        }
        //////////////////////////////////////
        iMsg=WebQQ::qqmsgs.erase(iMsg);
        delete pMsg;
    }
    WebQQ::mutex.unlock();
}
QString MainWindow::formatMsg(QQmsg *msg){
    QString retstr;
    QQgroup *g;
    QQfriend *f=nullptr;
    const char * fchar="<font color='dimgray'>%1</font> <font color='darkolivegreen'>%2</font> <font color='black'>%3</font><br> <b>%4</b><br>";
    switch(msg->type){
    case 0://好友信息
        f=WebQQ::qqFriends.value(msg->from_uin,nullptr);
        retstr=QString(fchar)
                .arg(QDateTime::fromTime_t(msg->time).toString("yyyy-MM-dd hh:mm:ss"))
                .arg("FTX"+msg->from_uin)
                .arg(f==nullptr?"":f->markname.isEmpty()?f->nick:QString("%1(%2)")
                                                         .arg(f->nick)
                                                         .arg(f->markname))
                .arg(msg->content);
        break;
    case 1://群
        g=WebQQ::qqGroups.value(msg->from_uin,nullptr);
        if(g!=nullptr){
            f=g->members.value(msg->send_uin,nullptr);
        }
        retstr=QString(fchar)
                .arg(QDateTime::fromTime_t(msg->time).toString("yyyy-MM-dd hh:mm:ss"))
                .arg("GTX"+msg->from_uin)
                .arg(g==nullptr?"":
                                g->name+(f==nullptr?"":
                                                    QString(" [FTX%1 %2]")
                                                    .arg(f->txuin)
                                                    .arg(f->markname.isEmpty()?f->nick:QString("%1(%2)")
                                                                               .arg(f->nick)
                                                                               .arg(f->markname))))
                .arg(msg->content);
        break;
    default:
        retstr=msg->content;
        break;
    }
    return retstr.replace('\n',"<br>");
}
void MainWindow::sysMsg(QString msg){
    ui->textBrowser->append(QString("%1 <font color='red'>%2</font><br>")
                            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                            .arg(msg));
}

void MainWindow::sendMsgFinished(QString txuin, QString msg, bool isok){
    QString txuinflag=txuin.left(3);
    const char *fchar="<font color='dimgray'>%1</font> 你对 %2 %3 说：<br>%4<br><font color='red'>%5</font>";
    QString saystr;
    txuin=txuin.mid(3);
    qDebug()<<"sendMsgFinished"<<txuinflag<<"txuin="<<txuin<<msg<<isok<<endl;
    if(txuinflag=="FTX"){
        QQfriend *f=WebQQ::qqFriends.value(txuin,nullptr);
        saystr=QString(fchar)
                .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                .arg("FTX"+txuin)
                .arg(f==nullptr?"":f->markname.isEmpty()?f->nick:QString("%1(%2)")
                                                         .arg(f->nick)
                                                         .arg(f->markname))
                .arg(msg)
                .arg(isok?"":"发送失败！可能已经掉线，请重新登录！");
    }else{//"GTX"
        QQgroup *g=WebQQ::qqGroups.value(txuin,nullptr);
        saystr=QString(fchar)
                .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                .arg("GTX"+txuin)
                .arg(g==nullptr?"":g->name)
                .arg(msg)
                .arg(isok?"":"发送失败！可能已经掉线，请重新登录！");
    }
    ui->textBrowser->append(saystr.replace('\n',"<br>"));
}

void MainWindow::sendMsg(){
    if(ui->plainTextEdit->toPlainText().isEmpty()) return;
    QString txuinflag=ui->txuinLineEdit->text().left(3);
    if(txuinflag=="FTX"||txuinflag=="GTX"){
        WebQQ::webQQNet->sendMsg(ui->txuinLineEdit->text(),ui->plainTextEdit->toPlainText());
        ui->plainTextEdit->clear();
    }else{
        ui->textBrowser->append("<font color='red'>请先选择一个接收对象！</font>");
    }

}
