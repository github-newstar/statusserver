#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <cstdlib>
#include <grpcpp/server_builder.h>
#include <iostream>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include "const.h"
#include "configMgr.hpp"
#include "RedisMgr.hpp"
#include "MysqlMgr.hpp"
#include "AsioIOServicePool.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <boost/asio.hpp>
#include "StatusServiceImp.hpp"

void RunServer(){
    auto &cfg = ConfigMgr::GetInstance();
    
    std::string serverAddress(cfg["StatusServer"]["Host"] + ":"
                              + cfg["StatusServer"]["Port"]);
    StatusServiceImp service;
    
    grpc::ServerBuilder builder;
    
    //监听端口添加服务
    builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    
    //启动服务
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << serverAddress << std::endl;
    
    //ioc
    boost::asio::io_context ioc;
    boost::asio::signal_set sigs(ioc, SIGINT, SIGTERM);
    
    //异步等待
    sigs.async_wait([&server, &ioc](const boost::system::error_code &ec, int signo) {
            if(!ec){
                std::cout << "Shutting down server ..." << signo << std::endl;
                server->Shutdown();
                ioc.stop();
            }
    });
    
    //启动ioc
    std::thread t([&ioc](){
        ioc.run();
    });
    
    server->Wait();

}

int main(){
    try{
        RunServer();
    }
    catch(const std::exception &e){
        std::cerr << "Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}