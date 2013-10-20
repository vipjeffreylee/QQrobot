#ifndef CHINAWEATHER_H
#define CHINAWEATHER_H

#include <QtCore>
#include <QtNetwork>
#include <QtSql>

class ChinaWeather : public QObject
{
    Q_OBJECT
 public:
     explicit ChinaWeather(const QString & location,QObject *parent = 0);
     explicit ChinaWeather(QObject *parent = 0);
    ~ChinaWeather();

     void setLocation(const QString & location);//设置城市
     QString getWeather();
     QString getWeather(const QString & location);

 signals:
     void sentWeather(QString Weather);//完成后发送信号

 protected slots:
     void analysisJson();//解析返回的json
     QString getCityCode(const QString & city);
 private:
     void dbInit();
     QNetworkRequest request;
     QNetworkReply * reply;
     QNetworkAccessManager  manger;
     QJsonDocument jsonDocument;
     QSqlDatabase dbLite;
     QString  data;
};

#endif // CHINAWEATHER_H
