#include "chinaweather.h"

ChinaWeather::ChinaWeather(QObject *parent) :
    QObject(parent)
{
    dbLite = QSqlDatabase::addDatabase("QSQLITE","weather");
    dbLite.setDatabaseName("WeatherCode.db");
    dbLite.open();
    dbInit();
}

ChinaWeather::ChinaWeather(const QString &location, QObject *parent):
    QObject(parent)
{
    dbLite = QSqlDatabase::addDatabase("QSQLITE","weather");
    dbLite.setDatabaseName("WeatherCode.db");
    dbLite.open();
    dbInit();
    this->setLocation(location);
}

ChinaWeather::~ChinaWeather()
{
    dbLite.close();
}

void ChinaWeather::setLocation(const QString &location)
{
    QString code  = getCityCode(location);
    if(code.isEmpty())
    {
        data = "对不起，找不到您输入的城市";
        emit sentWeather(data);
        return;
    }
    QString strSent = "http://m.weather.com.cn/data/"+code+".html";;
    request.setUrl(strSent);
    this->reply = this->manger.get(this->request);
//    connect(reply,&QNetworkReply::finished,this,&ChinaWeather::analysisJson,Qt::DirectConnection);
    QEventLoop eventLoop;
    connect(&this->manger, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
    eventLoop.exec();
    analysisJson();
}

void ChinaWeather::analysisJson()//解析json
{
    QString temp(this->reply->readAll());
    QJsonParseError error;
    QJsonObject jsonObj;
    jsonDocument = QJsonDocument::fromJson(temp.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError)
    {
        if (!(jsonDocument.isNull() || jsonDocument.isEmpty()))
        {
            if (jsonDocument.isObject())
            {
                jsonObj = jsonDocument.object();
                jsonObj = jsonObj.value("weatherinfo").toObject();
                data = jsonObj.value("city").toString();
                data = data + "(" + jsonObj.value("city_en").toString() + ")\n";
                data = data + jsonObj.value("date_y").toString() + "/" +jsonObj.value("week").toString() + "\n";
                data = data + "今天：\n";
                data = data + "  天气：" +jsonObj.value("weather1").toString() + "\n";
                data = data + "  温度：" +jsonObj.value("temp1").toString() + "(" + jsonObj.value("tempF1").toString() +")\n";
                data = data + "  风速：" +jsonObj.value("wind1").toString() + +"\n";
                data = data + "  紫外线：" +jsonObj.value("index_uv").toString() + "\n";
                data = data + "  穿衣指数：" +jsonObj.value("index_d").toString() + "\n";

                data = data + "\n明天：\n";
                data = data + "  天气：" +jsonObj.value("weather2").toString() + "\n";
                data = data + "  温度：" +jsonObj.value("temp2").toString() + "(" + jsonObj.value("tempF2").toString() +")\n";
                data = data + "  风速：" +jsonObj.value("wind2").toString() +"\n";
                data = data + "  紫外线：" +jsonObj.value("index48_uv").toString() + "\n";
                data = data + "  穿衣指数：" +jsonObj.value("index48_d").toString() + "\n";

                data = data + "\n后天：\n";
                data = data + "  天气：" +jsonObj.value("weather3").toString() + "\n";
                data = data + "  温度：" +jsonObj.value("temp3").toString() + "(" + jsonObj.value("tempF3").toString() +")\n";
                data = data + "  风速：" +jsonObj.value("wind3").toString() + "\n";
            }
            else
            {
              this->data = "对不起，找不到您输入的城市";
            }
        }
        else
        {
            this->data = "对不起，找不到您输入的城市";
        }
    }
    else
    {
        this->data = "对不起，找不到您输入的城市";
    }

    this->reply->deleteLater();
    emit sentWeather(data);
}

QString ChinaWeather::getWeather()
{
    return data;
}

QString ChinaWeather::getWeather(const QString & location)
{
    this->setLocation(location);
    return this->data;
}

QString ChinaWeather::getCityCode(const QString & city)
{
    QSqlQuery temp(dbLite);
    bool hh = temp.exec("select * from citycode where city='"+city+"'");
    qDebug() << hh << "  " << temp.size() << "  " << temp.executedQuery() <<" " << temp.first();;
    if (temp.first())
    {
        return temp.value("code").toString();
    }
    else
    {
        return "";
    }
}

void ChinaWeather::dbInit()
{
    QSqlQuery temp(dbLite);
    bool hh = temp.exec("select * from sqlite_master where  type = 'table' and tbl_name='citycode'");
    qDebug() << hh << "  " << temp.size() << "  " << temp.executedQuery() <<" " << temp.first();
    if (temp.first())
    {
        return;
    }

    temp.exec("CREATE TABLE [citycode] ([code] [NVARCHAR(36)] NOT NULL, [city] [NVARCHAR(36)] NOT NULL,  CONSTRAINT [] PRIMARY KEY ([code]));");

    QFile f("citycode.txt");

    if(!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug()<<f.errorString();
    }
    f.seek(0);
    QString strtemp;
    QStringList list;
    while(!f.atEnd())
    {
        strtemp = f.readLine().trimmed();
        if(!strtemp.isEmpty())
        {
            list = strtemp.split('=');
            temp.exec("Insert into citycode (code,city) values('" +list.at(0) +"','"+ list.at(1) + "')");
        }
    }
}
