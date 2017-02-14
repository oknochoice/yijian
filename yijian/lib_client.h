#ifndef LIB_CLIENT_H_
#define LIB_CLIENT_H_

#include "macro.h"
#include "catch.hpp"
#include "spdlog/spdlog.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ev.h>
#include "buffer_yi_util.hpp"
#include <functional>

#include <openssl/ssl.h>

typedef std::shared_ptr<yijian::buffer> Buffer_SP;
typedef std::function<void(Buffer_SP)> Read_CB;

// main thread call, read_cb subthread callback
void create_client(Read_CB && read_cb);
// main thread call
void client_send(Buffer_SP sp_buffer, uint16_t * sessionid);
void clear_client();


#endif
