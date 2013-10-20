/*
 *location:城市名
 *设置上城市名就可以自动获取天气了、、、
 *获取完成发送sentWeather(QString Weather)信号。
*/

#ifndef BAIDUWEATHER_H
#define BAIDUWEATHER_H

#include <QObject>
#include <QtCore>
#include <QtNetwork>
#include <QThread>

class BaiduWeather : public QObject
{
    Q_OBJECT
public:
    explicit BaiduWeather(const QString & location,QObject *parent = 0);
    explicit BaiduWeather(QObject *parent = 0);

    void setLocation(const QString & location);//设置城市
    QString getWeather();


signals:
    void sentWeather(QString Weather);//完成后发送信号

protected slots:
    void analysisJson();//解析返回的json

private:
    QNetworkRequest request;
    QNetworkReply * reply;
    QNetworkAccessManager  manger;
    QJsonDocument jsonDocument;
    QString  data;
    QString cityname;
public:
    static QString getWeather(const QString & location);
};

#endif // BAIDUWEATHER_H
