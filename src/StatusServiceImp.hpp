#pragma once
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include <mutex>
#include <string>
#include <unordered_map>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using message::GetChatServerReq;
using message::GetChatServerRsp;
using message::LoginReq;
using message::LoginRsp;
using message::StatusService;

struct ChatServer{
    std::string host;
    std::string port;
    std::string name;
    int conCnt;
};

class StatusServiceImp final : public StatusService::Service{
public:
    StatusServiceImp();
    Status  GetChatServer(ServerContext* context, const GetChatServerReq* request,
         GetChatServerRsp* response) override;
    Status  Login(ServerContext* context, const LoginReq* request,
         LoginRsp* response) override;
private:
    void insertToken(const int uid, const std::string token);
    
    ChatServer getChatServer();
    std::unordered_map<std::string, ChatServer> servers_;
    std::unordered_map<int, std::string> tokens_;
    std::mutex serverMtx_;
    std::mutex tokenMtx_;


};