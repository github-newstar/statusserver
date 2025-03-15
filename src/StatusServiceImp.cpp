#include "StatusServiceImp.hpp"
#include "configMgr.hpp"
#include "const.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iostream>
#include <mutex>

//生成uuid
std::string generate_unique_string() {
    boost::uuids::uuid uuid = boost::uuids::random_generator()();

	std::string unique_string = boost::uuids::to_string(uuid);
    std::cerr<<"gen uuid" <<std::endl;
	return unique_string;
}
Status StatusServiceImp::GetChatServer(ServerContext *context,
                                       const GetChatServerReq *request,
                                       GetChatServerRsp *response){
    std::cerr<< "status has received" << std::endl;
    const auto& serve = getChatServer();
    response->set_host(serve.host);
    response->set_port(serve.port);
    response->set_error(ErrorCodes::Success);
    response->set_token(generate_unique_string());
    insertToken(request->uid(), response->token());
    return Status::OK;
                                       }
Status StatusServiceImp::Login(ServerContext *context, const LoginReq *request,
                               LoginRsp *response){
    std::cerr << "login has received" << std::endl;
    auto uid = request->uid();
    auto token = request->token();
    std::lock_guard<std::mutex> lock(tokenMtx_);
    auto iter = tokens_.find(uid);

    if (iter == tokens_.end()) {
        response->set_error(ErrorCodes::UidInvalid);
        return Status::OK;
    }
    if (iter->second != token) {
        response->set_error(ErrorCodes::TokenInvalid);
        return Status::OK;
    }

    response->set_error(ErrorCodes::Success);
    response->set_uid(uid);
    response->set_token(token);
    return Status::OK;
}

ChatServer StatusServiceImp::getChatServer() {
    std::lock_guard<std::mutex> lock(serverMtx_);
    auto minServer = servers_.begin()->second;
    for (const auto &server : servers_) {
        if (server.second.conCnt < minServer.conCnt) {
            minServer = server.second;
        }
    }
    return minServer;
}
StatusServiceImp::StatusServiceImp() {
    auto &cfg = ConfigMgr::GetInstance();
    ChatServer server;
    server.name = cfg["ChatServer1"]["Name"];
    server.host = cfg["ChatServer1"]["Host"];
    server.port = cfg["ChatServer1"]["Port"];
    server.conCnt = 0;
    servers_[server.name] = server;
    
    // server.name = cfg["ChatServer2"]["Name"];
    // server.host = cfg["ChatServer2"]["Host"];
    // server.port = cfg["ChatServer2"]["Port"];
    // server.conCnt = 0;
    // servers_[server.name] = server;
}

void StatusServiceImp::insertToken(const int uid, const std::string token) {
    std::lock_guard<std::mutex> lock(tokenMtx_);
    tokens_[uid] = token;
}