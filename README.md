# proxy

使用流套接字实现的一个http代理程序，同时利用gtk实现了一个简单的界面。

#### 功能

1：支持ip地址过滤<br/>
2：支持url过滤<br/>
3：支持自定义的代理服务器<br/>

#### 使用方式

环境：Linux Ubuntu （gtk2.0+pthread）<br/>
编译：gcc -o ui ui.c csapp.c \`pkg-config --cflags --libs gtk+-2.0\` -g -Wall -lpthread<br/>

#### 效果

<div align="center">
<img width="260" height="450" src="https://github.com/wenboi/proxy/raw/master/image/1.png"/>
<img width="260" height="450" src="https://github.com/wenboi/proxy/raw/master/image/2.png"/>
</div>

