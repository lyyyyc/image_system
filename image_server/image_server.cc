#include"httplib.h"
#include<fstream>
#include"db.hpp"
#include<signal.h>
#include<sys/stat.h>

//回调函数
/*void Hello(const httplib::Request& req,httplib::Response& resp)
  {
//HTTP Content-Type
resp.set_content("hello","text/html");
}*/
class FileUtil
{
public:
    static bool Write(const std::string& file_name,const std::string& content)
    {
        std::ofstream file(file_name.c_str());
        if(!file.is_open())
        {
            return false;
        }
        file.write(content.c_str(),content.length());
        file.close();
        return true;
    }

    static bool Read(const std::string& file_name,std::string* content)
    {
        std::ifstream file(file_name.c_str());
        if(!file.is_open())
        {
            return false;
        }
        struct stat st;
        stat(file_name.c_str(),&st);
        content->resize(st.st_size);
        //一次性把整个文件读完
        //需要先知道文件大小
        //char* 缓冲区长度
        //int 读取多长
        file.read((char*)content->c_str(),content->size());
        //std::string line;
        //while(std::getline(file,line))
        //{
        //    *content+=line;
        //}
        file.close();
        return true;
    }
};

MYSQL* mysql=NULL;
int main()
{
    using namespace httplib;

    mysql=image_system::MySQL_Init();
    image_system::ImageTable image_table(mysql);
    signal(SIGINT,[](int){
           image_system::MySQL_Release(mysql);
           exit(0);
           });
    Server server;
    //客户端请求/hello路径时，执行一个特定的函数，指定不同路径对应到不同的函数上，这个过程称为"设置路由"
    //服务器中两个重要概念：1.请求(Request) 2.响应(Response)
    ////[&image_table]:捕获变量，&相当于按引用捕获
    server.Post("/image",[&image_table](const Request& req,Response& resp){
                Json::FastWriter writer;
                Json::Value resp_json;
                printf("上传图片\n");
                //1.对参数进行校验
                auto ret=req.has_file("upload");
                if(!ret)
                {
                printf("文件上传出错！\n");
                resp.status=404;
                resp_json["ok"]=false;
                resp_json["reason"]="上传出错，没有需要的upload字段";
                resp.set_content(writer.write(resp_json),"application/json");
                return;
                }
                //2.根据文件名获取到文件数据file对象
                const auto& file=req.get_file_value("upload");
                //file.filename
                //file.connect_type
                //图片内容
                //printf("size=%lu ret=%d filename=%s content-type=%s\n",size,ret,file.filename.c_str(),
                //       file.content_type.c_str());
                //3.把图片属性信息插入到数据库中
                Json::Value image;
                image["image_name"]=file.filename;
                image["size"]=(int)file.length;
                image["upload_time"]="2019/09/29";//todo
                image["md5"]="sdasd";//todo
                image["type"]=file.content_type;
                image["path"]="./data/"+file.filename;
                ret=image_table.Insert(image);
                if(!ret)
                {
                    printf("image_table Insert failed!\n");
                    resp_json["ok"]=false;
                    resp_json["reason"]="插入失败！";
                    resp.status=500;
                    resp.set_content(writer.write(resp_json),"application/json");
                    return;
                }
                //4.把图片保存到指定的磁盘目录中
                auto body=req.body.substr(file.offset,file.length);
                FileUtil::Write(image["path"].asString(),body);
                //5.构造一个响应数据通知客户端上传成功
                resp_json["ok"]=true;
                resp.status=200;
                resp.set_content(writer.write(resp_json),"application/json");

                return;
    });
    server.Get("/image",[&image_table](const Request& req,Response& resp){
               (void)req;//无任何实际效果
               printf("获取所有图片信息\n");
               Json::Value resp_json;
               Json::FastWriter writer;
               //1.调用数据库接口获取数据
               bool ret=image_table.SelectAll(&resp_json);
               if(!ret)
               {
               printf("查询数据库失败！\n");
               resp_json["ok"]=false;
               resp_json["reason"]="查询数据库失败！";
               resp.status=500;
               resp.set_content(writer.write(resp_json),"application/json");
               return;
               }
               //2.构造相应结果返回给客户端
               resp.status=200;
               resp.set_content(writer.write(resp_json),"application/json");
               });
    //1.正则表达式
    //2.原始字符串(raw string)
    server.Get(R"(/image/(\d+))",[&image_table](const Request& req,Response& resp){
               Json::Value resp_json;
               Json::FastWriter writer;
               //1.先获取到图片id
               int image_id=std::stoi(req.matches[1]);
               //2.根据图片id查询数据库
               bool ret=image_table.SelectOne(image_id,&resp_json);
               if(!ret)
               {
               printf("数据库查询出错！\n");
               resp_json["ok"]=false;
               resp_json["reason"]="数据库查询出错";
               resp.status=500;
               resp.set_content(writer.write(resp_json),"application/json");
               return;
               }
               //3.把查询结果返回给客户端
               resp_json["ok"]=true;
               resp.set_content(writer.write(resp_json),"application/json");
               return;
               });
    server.Get(R"(/show/(\d+))",[&image_table](const Request& req,Response& resp){
               Json::Value resp_json;
               Json::FastWriter writer;
               //1.根据图片id去数据库查到相对应的目录
               int image_id=std::stoi(req.matches[1]);
               printf("获取id为%d的图片内容！\n",image_id);
               Json::Value image;
               bool ret=image_table.SelectOne(image_id,&image);
               if(!ret)
               {
                   printf("数据库查询出错！\n");
                   resp_json["ok"]=false;
                   resp_json["reason"]="数据库查询出错";
                   resp.status=500;
                   resp.set_content(writer.write(resp_json),"application/json");
                   return;
               }
               //2.根据目录找到文件内容并进行读取
               std::string image_body;
               ret=FileUtil::Read(image["path"].asString(),&image_body);
               if(!ret)
               {
                   printf("读取图片文件失败！\n");
                   resp_json["ok"]=false;
                   resp_json["reason"]="读取图片文件出错";
                   resp.status=500;
                   resp.set_content(writer.write(resp_json),"application/json");
                   return;
               }
               //3.把文件内容构成一个响应
               resp.status=200;
               //不同的图片，设置的content type不同
               //png设置为image/png
               //jpg设置为image/jpg
               resp.set_content(image_body,image["type"].asCString());
    });
    server.Delete(R"(/image(\d+))",[&image_table](const Request& req,Response& resp){
                  Json::Value resp_json;
                  Json::FastWriter writer;
                  //1.根据id找到图片对应的目录
                  int image_id=std::stoi(req.matches[1]);
                  printf("删除id为%d的图片！\n",image_id);
                  //2.查找对应文件的路径
                  Json::Value image;
                  bool ret=image_table.SelectOne(image_id,&image);
                  if(!ret)
                  {
                    printf("删除图片文件失败！\n");
                    resp_json["ok"]=false;
                    resp_json["reason"]="删除图片失败";
                    resp.status=404;
                    resp.set_content(writer.write(resp_json),"application/json");
                    return ;
                  }
                  //3.调用数据库操作进行删除
                  ret=image_table.Delete(image_id);
                  if(!ret)
                  {
                    printf("删除图片文件失败！\n");
                    resp_json["ok"]=false;
                    resp_json["reason"]="删除图片失败";
                    resp.status=404;
                    resp.set_content(writer.write(resp_json),"application/json");
                    return ;
                  }
                  //4.删除磁盘上文件
                  unlink(image["path"].asCString());
                  //5.构造响应
                  resp_json["ok"]=true;
                  resp.status=200;
                  resp.set_content(writer.write(resp_json),"application/json");
                  });
    server.set_base_dir("./wwwroot");
    server.listen("0.0.0.0",9000);
    return 0;
}

