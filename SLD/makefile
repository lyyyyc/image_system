.PYHON:main
main:main.cc
	g++ -std=c++0x -DCPPHTTPLIB_OPENSSL_SUPPORT $^ -o $@ -lboost_filesystem -lboost_system -lpthread -lboost_thread -lssl -lcrypto -g
client:client.cc client.hpp
	g++ -std=c++0x $^ -o $@ -lboost_filesystem -lboost_system -lpthread -lboost_thread
server:server.cc server.hpp
	g++ -std=c++0x $^ -o $@ -lboost_filesystem -lboost_system -lpthread
.PHONY:clean
clean:	
	rm main
