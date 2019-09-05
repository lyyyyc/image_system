#include<cstdio>
#include<cstdlib>
#include<mysql/mysql.h>

using namespace std;

int main()
{
    //使用mysqlAPI操作数据库
    //1.创建一个mysql的句柄
    //MYSQL *mysql_init(MYSQL *mysql)
    MYSQL* mysql=mysql_init(NULL);
    //2.用句柄和数据库建立连接
    //MYSQL *mysql_real_connect(MYSQL *mysql, const char *host, const char *user, const char *passwd, 
    //const char *db, unsigned int port, const char *unix_socket, unsigned long client_flag)
    if(mysql_real_connect(mysql,"127.0.0.1","root","","image_system",3306,NULL,0)==NULL)
        //数据库连接失败
    {
        printf("connect error\n",mysql_error(mysql));
        return 1;//进程退出码
    }
    //3.设置编码格式
    //int mysql_set_character_set(MYSQL *mysql, const char *csname)
    mysql_set_character_set(mysql,"utf8");
    //4.拼接sql语句
    char sql[4096]={0};
    sprintf(sql,"insert into image_table value(null,'test.png',1024,'2019/08/26','abcdef','png','data/test.png')");
    //5.执行sql语句，负责客户端给服务器发送数据的过程
    //int mysql_query(MYSQL *mysql, const char *stmt_str)
    int ret=mysql_query(mysql,sql);
    {
        if(ret!=0)
        {
            printf("query error\n");
            return 1;
        }
    }
    //6.关闭句柄
    //void mysql_close(MYSQL *mysql)
    mysql_close(mysql);
    return 0;
}

