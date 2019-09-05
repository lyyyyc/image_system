//使用httplib实现一个最简单的http服务器
//了解httplib的最基本使用

#include"httplib.h"

using namespace httplib;

void HelloWorld(const Request &req,Response &rsp)
{
    rsp.status=302;
    rsp.set_header("Location","http://www.baidu.com");
    rsp.body="<html><h1>Hello World</h4></html>";
    return;
}
int main()
{
    Server server;
    server.Get("/",HelloWorld);
    server.listen("0.0.0.0",9000); 
    return 0;
}

