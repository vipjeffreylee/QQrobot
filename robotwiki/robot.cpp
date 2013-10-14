/****************************************************************************
**
** Copyright (C) 2013 Jeffrey Lee.
** All rights reserved.
** Contact: vipjeffreylee@gmail.com
** Please keep the author contact information.
** 2013-10-10
**
****************************************************************************/
#include "robot.h"
#include<QTime>
#include<QDebug>
#include<QDateTime>
Robot::Robot(){
    joke<<"离婚以后，李亚鹏黯然神伤。 中秋朋友聚会，签单时把名字写成了“李亚朋”，旁边朋友悄声提醒：“你的鸟没了。” 李亚鹏苦笑道：“老婆都没了，还要鸟有什么用。” 王菲听说这件事后，默默将名字改成了“王非”。"
       <<"校长有天路过学校后门，突然听到一句：“我要考牛津！”校长顿时感动不己，没想到我们学校也有如此有志青年，决定看看是哪位，忽然又听到一句：“再来两串大腰子！！！"
      <<"感冒了，怕去医院太破费，于是对老婆说：“熬点姜汤喝吧。”老婆说：“姜太贵，咱们还是去医院吧……”"
     <<"有个人来面试，说三年工作经验。面试官问他，你才毕业2年，怎么有3年工作经验，那一年是哪来的？答：加班…"
    <<"一个学生收到他父亲的信,信上说:你以后写家信,应该多写一些生活的情况,不要只知道要钱。这次寄10块钱给你,附带告诉你一点小错误,用阿拉伯写十的时候,只能写一个零,不能写两个。"
    <<"前几天一朋友去东莞玩，在某大酒店下榻，夜里正在迷糊的时候，突然听到有人敲门问话。 女：有人吗？ 朋友：有什么事吗？ 女：我想问下帅哥，昆明的昆怎么写啊？ 朋友：哦，上面一个日，下面一个.....! 女：我知道了，那么我们昆一下好吗？ 朋友晕倒，小-姐不可怕，就怕小-姐有文化。"
    <<"期末考试说马上要开始了，望同学们以突击为主，作弊为辅：采取师进我藏，师退我抄，迂回作战方针！送你一幅对联：考试不作弊来年当学弟，宁可没人格不能不及格。横批：死也要过。考试必要技巧：三长一短选最短；三短一长选最长；长短不一要选B；参差不齐就选D。以抄为主，以蒙为辅，蒙抄结合，一定及格！"
    <<"女人如同一本书，大部分的男人只想翻它而不想买它。"
    <<"优秀男女标准：执着但不固执，随和但不随性，出色但不出格，低调但不低俗，痴情但不矫情，自信但不自我，诱人但不缠人，计算但不算计，幽默但不油滑，潮流但不风流，飘逸但不飘荡。"
    <<"情人节“淘宝数据”显示：卖花的始终干不过卖套的。不得不感叹一下，情人节，送花始终是个手段，上床才是最终目的！";
    QTime time;
    time= QTime::currentTime();
    qsrand(time.msec()+time.second()*1000);
}
QString Robot::name(){
    return QString("生活小百科");
}
ReplyMsg Robot::listenandsay(const MsgSender msgSender, const QString &message){
    ReplyMsg replyMsg;
    if(msgSender.groupTXUIN.isEmpty()){
        replyMsg.recTXUIN=msgSender.friendTXUIN;
    }else{
        replyMsg.recTXUIN=msgSender.groupTXUIN;
    }
    if(message.indexOf("#help")>-1){
        replyMsg.content="我是生活小百科，我能告诉你：\n 几点了、#time  ";
    }else if(message.indexOf("几点了")>-1||message.indexOf("#time")>-1){
        replyMsg.content=QString("%1 现在时间是：\n%2")
                .arg(msgSender.friendName)
                .arg(QDateTime::currentDateTime().toString("yyyy年MM月dd日 hh点mm分ss秒"));
    }
    return replyMsg;
}
