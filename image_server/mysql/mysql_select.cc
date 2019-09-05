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
    sprintf(sql,"select * from image_table");
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
    //6.获取结果集合
    MYSQL_RES* result=mysql_store_result(mysql);
    //7.遍历结果集合
    int rows=mysql_num_rows(result);
    int cols=mysql_num_fields(result);
    for(int i=0;i<rows;++i)
    {
        MYSQL_ROW row=mysql_fetch_row(result);
        for(int j=0;j<cols;++j)
        {
            printf("%s\t",row[j]);
        }
        printf("\n");
    }
    //7.释放结果集合
    mysql_free_result(result);
    //8.关闭句柄
    //void mysql_close(MYSQL *mysql)
    mysql_close(mysql);
    return 0;
}

