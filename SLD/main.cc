#include"client.hpp"
#include"server.hpp"

void srv_start()
{
    SLDServer server("./cert.pem","./key.pem");
    server.Start(9000);
}

int main()
{
    std::thread thr(srv_start);
    thr.detach();
    SLDClient client(9000);
    client.Start();
    return 0;
}
