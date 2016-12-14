#ifndef LIB_CLIENT_H_
#define LIB_CLIENT_H_

#include "macro.h"
#include "catch.hpp"
#include "spdlog/spdlog.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ev.h>
#include "buffer.h"


using yijian::Buffer_SP;
typedef std::function<void(Buffer_SP)> Read_CB;

// main thread call, read_cb subthread callback
void create_client(Read_CB && read_cb);
// main thread call
void client_send(Buffer_SP sp_buffer, int16_t * sessionid);



#endif
