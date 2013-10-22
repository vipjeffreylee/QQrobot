#include "baiduweather.h"

BaiduWeather::BaiduWeather(QObject *parent) :
    QObject(parent)
{

}

BaiduWeather::BaiduWeather(const QString &location, QObject *parent):
    QObject(parent)
{
    this->setLocation(location);
}

void BaiduWeather::setLocation(const QString &location)
{
    data = "对不起，我获取信息失败了、、、";
//    location=location.trimmed();
    cityname=location;
    QString strSent = "http://api.map.baidu.com/telematics/v3/weather?location=" + location +" &output=json&ak=6632266da0d7c5839bf5dcc440d15c87";
    request.setUrl(strSent);
    this->reply = this->manger.get(this->request);
//    connect(reply,&QNetworkReply::finished,this,&BaiduWeather::analysisJson,Qt::DirectConnection);
    QEventLoop evenloop;
    connect(&this->manger, SIGNAL(finished(QNetworkReply*)), &evenloop, SLOT(quit()));
    evenloop.exec();
    analysisJson();
}

void BaiduWeather::analysisJson()//解析json
{

    QJsonObject jsonObj;
    QJsonArray jsonArray;
    jsonDocument = QJsonDocument::fromJson(QString(this->reply->readAll()).toUtf8());

    if (!(jsonDocument.isNull() || jsonDocument.isEmpty()))
    {
        if (jsonDocument.isObject())
        {
            jsonObj = jsonDocument.object();
            if(jsonObj.value("status").toString() == "success")
            {
                jsonArray = jsonObj.value("results").toArray();
                jsonObj = jsonArray.at(0).toObject();
                this->data = jsonObj.value("currentCity").toString()+"天气预报：";
                this->data += "\n";
                jsonArray = jsonObj.value("weather_data").toArray();
                for (int i = 0; i< jsonArray.size();++i)
                {
                    jsonObj = jsonArray.at(i).toObject();
                    this->data += jsonObj.value("date").toString();
                    this->data += " ";
                    this->data += jsonObj.value("weather").toString();
                    this->data += " ";
                    this->data += jsonObj.value("wind").toString();
                    this->data += " ";
                    this->data += jsonObj.value("temperature").toString();
                    this->data += "\n";
                }
            }
            else
            {
              this->data = "对不起，找不到您输入的城市:"+cityname;
            }
        }

    }
    else
    {
        this->data = "对不起，找不到您输入的城市:"+cityname;
    }

    this->reply->deleteLater();
    emit sentWeather(data);
}

QString BaiduWeather::getWeather()
{
    return data;
}

QString BaiduWeather::getWeather(const QString & location)
{
    static BaiduWeather *baiduweather=nullptr;
    if(baiduweather==nullptr){
        baiduweather=new BaiduWeather();
    }
    baiduweather->setLocation(location);
    return baiduweather->data;
}
