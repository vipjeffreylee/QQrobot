QQrobot
=======

<p>
    QQrobot是使用Qt5开发的，基于腾讯公司WebQQ协议实现，可以向QQ群或者是个人自动发送信息的程序，分为两个部分，QQ主体和robot插件。QQ主体负责实现WebQQ协议部分，实现了QQ号码登录，信息接收和发送功能。robot使用Qt插件机制，负责分析聊天内容，智能做出回应。
</p>
<p>
    QQ主体窗口内，可监控显示聊天信息、好友列表、群列表和机器人列表。可为QQ群或者个人指定随意多个机器人为之提供服务。也提供了信息发送功能，可随时向QQ群或者个人发送信息。
</p>
<p>
    robot插件，使用Qt5的plugin技术，完成接口RobotInterface内的name和listenandsay方法就ok，name返回robot的名子，listenandsay的参数是robot收到的聊天信息和发送者信息，listenandsay返回值是robot回应信息。
</p>
