QQrobot
=======

<p>
    QQrobot是用C++语言开发设计，程序界面、网络通迅等模块使用了Qt，Qt是开源的，跨平台C++程序图形界面框架库,QQrobot具备跨平台特性，可在linux、MAC OS以及Windows等操作系统中运行，集成开发编译环境请到 http://qt-project.org/ 下载。
</p>
<p>
    QQrobot基于腾讯公司WebQQ协议实现，可以向QQ群或者是个人自动发送信息。分为两个部分，QQ主体和robot插件。QQ主体解析WebQQ协议，负责QQ号码登录，信息接收和发送功能。robot分析聊天内容，跟据聊天内容智能做出回应。
</p>
<p>
    QQ主体窗口内，可监控显示聊天信息、好友列表、群列表和机器人列表。为QQ群或者个人指定随意多个机器人为之提供服务。也提供了信息发送功能，可随时向QQ群或者个人发送信息。
</p>
<p>
    robot插件，使用Qt5的plugin技术，可单独开发，编译后拷贝到plugins目录中，QQ主体自动识别安装运行。robot插件只要完成接口RobotInterface内的name和listenandsay方法就ok，name返回robot的名子，listenandsay的参数是收到的聊天内容和发送者信息，返回值是robot回应信息。
</p>
