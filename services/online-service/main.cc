#include <iostream>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <ThreadPool.h>
#include <glog/logging.h>
#include "decoder-itf.h"
#include "service-impl.h"
#include "service-env.h"

#include <fstream>

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

typedef websocketpp::config::asio::message_type::ptr message_ptr;

static ThreadPool threadpool(1);

void on_message(server* s, websocketpp::connection_hdl hdl, message_ptr msg) {

    LOG(INFO) << "on_message called with hdl: " << hdl.lock().get();
    LOG(INFO)<<"trigger on message";

    char con_addr[20];
    snprintf(con_addr, sizeof(con_addr), "%p", hdl.lock().get());
    std::string cid(con_addr);
    ContextPtr cptr;
    int ret = ServEnv::instance()->GetContext(cid,s,hdl,cptr);
    if(ret!=0){
        LOG(ERROR)<<"get context or create context failed";
        return;
    }

    /*
    std::ofstream outFile("message.pcm",std::ios::out | std::ios::binary | std::ios::app);
    outFile.write(msg->get_payload().c_str(),msg->get_payload().size());
    LOG(INFO)<<"receice length:"<<msg->get_payload().size();
    */

    std::string speech = msg->get_payload();

    //cptr->RunDecode(speech.c_str(), speech.size(), false);
    threadpool.enqueue(
            [](ContextPtr cptr, std::string& speech){
                cptr->RunDecode(speech.c_str(), speech.size(), false);
            },cptr, speech);
}
void on_open(server* s, websocketpp::connection_hdl hdl) {
    LOG(INFO)<<"trigger on open";

}

void on_close(server* s, websocketpp::connection_hdl hdl) {
    LOG(INFO)<<"trigger on close";
    char con_addr[20];
    snprintf(con_addr, sizeof(con_addr), "%p", hdl.lock().get());
    std::string cid(con_addr);
    ServEnv::instance()->DeleteContext(cid);

}

bool on_validate(server* s, websocketpp::connection_hdl hdl){
    LOG(INFO)<<"trigger on validate";
    ServEnv::instance()->DeleteContext();

    char con_addr[20];
    snprintf(con_addr, sizeof(con_addr), "%p", hdl.lock().get());
    std::string cid(con_addr);
    ContextPtr cptr;
    int ret = ServEnv::instance()->GetContext(cid,s,hdl,cptr);
    if(ret!=0){
        LOG(INFO)<<"reject connect";
        return false;
    }else{
        LOG(INFO)<<"accept connect";
        return true;
    }
}

int main(){

    int thread_num = 5;

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


    server asr_server;
    asr_server.init_asio();
    int port = 8080;
    asr_server.set_message_handler(bind(&on_message,&asr_server,::_1,::_2));
    asr_server.set_open_handler(bind(&on_open,&asr_server,::_1));
    asr_server.set_close_handler(bind(&on_close,&asr_server,::_1));
    asr_server.set_validate_handler(bind(&on_validate,&asr_server,::_1));

    asr_server.listen(websocketpp::lib::asio::ip::tcp::v4(), port);
    asr_server.start_accept();
    LOG(INFO)<<"server ready";

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
