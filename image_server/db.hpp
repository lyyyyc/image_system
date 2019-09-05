#pragma once

#include<cstdio>
#include<cstdlib>
#include<mysql/mysql.h>
#include<jsoncpp/json/json.h>

namespace image_system
{
static MYSQL* MySQL_Init()
{
    MYSQL* mysql=mysql_init(NULL);
    if(mysql_real_connect(mysql,"127.0.0.1","root","","image_system",3306,NULL,0)==NULL)
    {
        printf("连接失败！%s\n",mysql_error(mysql));
        return NULL;
    }
    mysql_set_character_set(mysql,"utf8");
    return mysql; 
}

static void MySQL_Release(MYSQL* mysql)
{
    mysql_close(mysql);
}

//操作数据库中image_table
//insert等操作，函数依赖的输入信息较多，为了防止参数过多，可以使用JSON来封装参数
class ImageTable
{
public:
    ImageTable(MYSQL* mysql)
        :_mysql(mysql)
    {}
    //image形如
    //{
    //  image_name:"test.png",
    //  size:1024,
    //  upload_time:"2019/08/28",
    //  md5:"abcdefgh",
    //  type:"png",
    //  path:"data/test.png"
    //}
    //使用json原因：1.扩展更方便 2.方便和服务器收到的数据打通
    bool Insert(const Json::Value& image)
    {
        char sql[4096]={0};
        sprintf(sql,"insert into image_table values(null,'%s',%d,'%s','%s','%s','%s')",
                image["image_name"].asCString(),image["size"].asInt(),image["upload_time"].asCString(),
                image["md5"].asCString(),image["type"].asCString(),image["path"].asCString());
        printf("[Insert sql]%s\n",sql);
        int ret=mysql_query(_mysql,sql);
        if(ret!=0)
        {
            printf("Insert 执行 sql 失败！%s\n",mysql_error(_mysql));
            return false;
        }
        return true;
    }
    bool SelectAll(Json::Value* images)
    {
        char sql[4096]={0};
        sprintf(sql,"select * from image_table");
        int ret=mysql_query(_mysql,sql);
        if(ret!=0)
        {
            printf("SelectAll 执行 sql 失败！%s\n",mysql_error(_mysql));
            return false;
        }
        //遍历结果集合，并把结果集合写到images参数中
        MYSQL_RES* result=mysql_store_result(_mysql);
        int rows=mysql_num_rows(result);
        for(int i=0;i<rows;++i)
        {
            MYSQL_ROW row=mysql_fetch_row(result);
            //数据库查出的每条记录都相当于是一个图片信息，需要把这个信息转换成JSON格式
            Json::Value image;
            image["image_id"]=atoi(row[0]);
            image["image_name"]=row[1];
            image["size"]=atoi(row[2]);
            image["upload_time"]=row[3];
            image["md5"]=row[4];
            image["type"]=row[5];
            image["path"]=row[6];
            images->append(image);
        }
        //释放结果集合
        //忘记释放会导致内存泄漏
        mysql_free_result(result);
        return true;
    }
    bool SelectOne(int image_id,Json::Value* image_ptr)
    {
        char sql[4096]={0};
        sprintf(sql,"select * from image_table where image_id=%d",image_id);
        int ret=mysql_query(_mysql,sql);
        if(ret!=0)
        {
            printf("SelectOne 执行 sql 失败！%s\n",mysql_error(_mysql));
            return false;
        }
        MYSQL_RES* result=mysql_store_result(_mysql);
        int rows=mysql_num_rows(result);
        if(rows!=1)
        {
            printf("SelectOne 查询结果不是一条记录！实际查到%d条！\n",rows);
            return false;
        }
        MYSQL_ROW row=mysql_fetch_row(result);
        Json::Value image;
        image["image_id"]=atoi(row[0]);
        image["image_name"]=row[1];
        image["size"]=atoi(row[2]);
        image["upload_time"]=row[3];
        image["md5"]=row[4];
        image["type"]=row[5];
        image["path"]=row[6];
        *image_ptr=image;
        mysql_free_result(result);
        return true;
    }
    bool Delete(int image_id)
    {
        char sql[4096]={0};
        sprintf(sql,"delete from image_table where image_id=%d",image_id);
        int ret=mysql_query(_mysql,sql);
        if(ret!=0)
        {
            printf("Delete 执行 sql 失败！%s\n",mysql_error(_mysql));
            return false;
        }
        return true;
    }
private:
    MYSQL* _mysql;
};
}
//end image_system
