#ifndef LIB_CLIENT_H_
#define LIB_CLIENT_H_

#include "macro.h"
#include "catch.hpp"
#include "spdlog/spdlog.h"
#include "pinglist.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ev.h>
#include "buffer.h"

typedef std::function<void(Buffer_SP)> Read_CB;

// main thread
void create_client(Read_CB && read_cb);
// main thread
void client_send(Buffer_SP sp_buffer);



#endif
