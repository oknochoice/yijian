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

typedef void (*IM_CB)(const char * header, uint32_t length, 
                      uint16_t session, int type);

// main thread
void create_client(IM_CB callback);
// main thread
void client_send(Buffer_SP sp_buffer);

#endif
