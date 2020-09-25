#include <iostream>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <ThreadPool.h>
#include <glog/logging.h>
#include "decoder-itf.h"
#include "service-impl.h"
#include "service-env.h"


typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

typedef websocketpp::config::asio::message_type::ptr message_ptr;

void on_message(server* s, websocketpp::connection_hdl hdl, message_ptr msg) {

    char con_addr[20];
    snprintf(con_addr, sizeof(con_addr), "%p", hdl.lock().get());
    std::string cid(con_addr);

    ContextPtr cptr;
    int ret = 0;
    if(!ServEnv::instance()->DetectContext(cid)){
        LOG(ERROR)<<"create context failed";
        return;
    }
    ret = ServEnv::instance()->GetContext(cid, cptr);
    if(ret!=0){
        LOG(ERROR)<<"get context failed";
        return;
    }
    ThreadPoolPtr pth = ServEnv::instance()->GetThreadPool(cptr->GetHandler());
    pth->enqueue(
            [](ContextPtr cptr, message_ptr msg){
                cptr->HandleMessage(msg);
            },cptr, msg);
}
void on_open(server* s, websocketpp::connection_hdl hdl) {
    char con_addr[20];
    snprintf(con_addr, sizeof(con_addr), "%p", hdl.lock().get());
    std::string cid(con_addr);
}

void on_close(server* s, websocketpp::connection_hdl hdl) {
    char con_addr[20];
    snprintf(con_addr, sizeof(con_addr), "%p", hdl.lock().get());
    std::string cid(con_addr);

    if(ServEnv::instance()->DetectContext(cid)){
        ContextPtr cptr;
        ServEnv::instance()->GetContext(cid, cptr);
        cptr->SetStatus(CONTEXT_CLOSED);
    }
}

bool on_validate(server* s, websocketpp::connection_hdl hdl){

    ServEnv::instance()->DeleteContext();
    char con_addr[20];
    snprintf(con_addr, sizeof(con_addr), "%p", hdl.lock().get());
    std::string cid(con_addr);

    int ret = ServEnv::instance()->CreateContext(cid, s, hdl);
    if(ret == 0) return true;
    else return false;
}

int main(int argc, char **argv){

    //FLAGS_log_dir = "./logs";
    //google::InitGoogleLogging(argv[0]);

    int thread_num = 10;
    int ret = ServEnv::instance()->SetConfig("conf/resource.conf","conf/decoder.conf");
    if(ret!=0){
        LOG(ERROR)<<"set server env config failed";
    }else{
        LOG(INFO)<<"set server env config successfully";
    }
    ret = ServEnv::instance()->CreateResource();
    if(ret!=0){
        LOG(ERROR)<<"create env resource failed";
    }else{
        LOG(INFO)<<"create env resource successfully";
    }

    ret = ServEnv::instance()->CreateHandlers(thread_num);
    if(ret!=0){
        LOG(ERROR)<<"create handlers failed";
    }else{
        LOG(INFO)<<"create handlers successfully";
    }



    server asr_server;
    asr_server.init_asio();
    asr_server.set_reuse_addr(true);
    int port = 8080;
    asr_server.set_message_handler(bind(&on_message,&asr_server,::_1,::_2));
    asr_server.set_open_handler(bind(&on_open,&asr_server,::_1));
    asr_server.set_close_handler(bind(&on_close,&asr_server,::_1));
    asr_server.set_validate_handler(bind(&on_validate,&asr_server,::_1));

    asr_server.listen(websocketpp::lib::asio::ip::tcp::v4(), port);
    asr_server.start_accept();
    LOG(INFO)<<"Thread Number is:"<<thread_num;
    LOG(INFO)<<"Service Ready:";

    if(thread_num == 1){
        asr_server.run();
    }else{
        typedef websocketpp::lib::shared_ptr<websocketpp::lib::thread> thread_ptr;
        std::vector<thread_ptr> ts;
        for (auto i = 0; i < thread_num; i++) {
            ts.push_back(websocketpp::lib::make_shared<websocketpp::lib::thread>(&server::run, &asr_server));
        }
        for (auto i = 0; i < thread_num; i++) {
            ts[i]->join();
        }
    }
    return 0;
}
