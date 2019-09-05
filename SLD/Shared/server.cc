#include"httplib.h"
#include<iostream>
#include<boost/filesystem.hpp>
#include<fstream>

using namespace httplib;
namespace bf=boost::filesystem;

#define SHARED_PATH "Shared"

class SLDServer
{
private:
    Server _server;
private:
    static void GetHostPair(const Request &req,Response &rsp)
    {
        rsp.status=200;
    }
    static void GetFileList(const Request &req,Response &rsp)
    {
        bf::directory_iterator item_begin(SHARED_PATH);
        bf::directory_iterator item_end;
        std::stringstream body;
        //body<<"<html><body>";
        for(;item_begin!=item_end;++item_begin)
        {
            if(bf::is_directory(item_begin->status()))
            {
                continue;
            }
            bf::path path=item_begin->path();
            std::string name=path.filename().string();
            //body<<"<h2><a href='/list/"<<name<<"'>";
            //body<<name;
            //body<<"</a></h2>";
            rsp.body+=name+"\n";
        }
        //body<<"</body></html>";
        //rsp.body=body.str();
        rsp.set_header("Content-Type","text/html");//信息渲染在浏览器上
        rsp.status=200;
    }
    static void GetFileData(const Request &req,Response &rsp)
    {
        // /list/a.txt->Download/a.txt
        bf::path path(req.path);
        std::stringstream name;
        name<<SHARED_PATH<<"/"<<path.filename().string();
        if(!bf::exists(name.str()))
        {
            rsp.status=404;
            return;
        }
        if(bf::is_directory(name.str()))
        {
            rsp.status=403;
            return;
        }
        int64_t fsize=bf::file_size(name.str());//应获取文件大小，获取目录大小会出错，需加判断
        if(req.method=="HEAD")
        {
            rsp.status=200;
            std::string len=std::to_string(fsize);
            rsp.set_header("Content-Length",len.c_str());
            return;
        }
        else
        {
            if(!req.has_header("Range"))
            {
                rsp.status=400;
                return;
            }
            std::string range_val;
            range_val=req.get_header_value("Range");
            int64_t start,len;
            //bytes=start-end
            bool ret=RangeParse(range_val,start,len);
            if(ret==false)
            {
                rsp.status=400;
                return;
            }
            std::cerr<<"range:"<<range_val<<std::endl;
            std::cerr<<"body.resize:"<<len<<std::endl;
            rsp.body.resize(len);
            std::ifstream file(name.str(),std::ios::binary);
            if(!file.is_open())
            {
                std::cerr<<"open file"<<name.str()<<"failed\n";
                rsp.status=404;
                return;
            }
            file.seekg(start,std::ios::beg);
            rsp.body.resize(len);
            file.read(&rsp.body[0],len);
            if(!file.good())
            {
                std::cerr<<"read file"<<name.str()<<"body error"<<std::endl;
                rsp.status=500;
                return;
            }
            file.close();
            rsp.status=206;
            rsp.set_header("Content-Type","application/octet-stream");
            std::cerr<<"file range:"<<range_val<<"download success\n";
        }
    }
    static bool RangeParse(std::string &range_val,int64_t &start,int64_t &len)
    {
        size_t pos1=range_val.find("=");
        size_t pos2=range_val.find("-");
        if(pos1==std::string::npos)
        {
            std::cerr<<"range "<<range_val<<" format error\n";
            return false;
        }
        int64_t end;
        std::string rstart;
        std::string rend;
        rstart=range_val.substr(pos1+1,pos2-pos1-1);
        rend=range_val.substr(pos2+1);
        std::stringstream tmp;
        tmp<<rstart;
        tmp>>start;
        tmp.clear();
        tmp<<rend;
        tmp>>end;
        len=end-start+1;
        return true;
    }
public:
    SLDServer()
    {
        //判断共享目录若不存在，则创建
        if(!bf::exists(SHARED_PATH))
        {
            bf::create_directory(SHARED_PATH);
        }
    }
    bool Start(uint16_t port)
    {
        _server.Get("/hostpair",GetHostPair);
        _server.Get("/list",GetFileList);
        _server.Get("/list/(.*)",GetFileData);
        _server.listen("0.0.0.0",port);
    }
};
int main()
{
    SLDServer srv;
    srv.Start(9000);
    return 0;
}
