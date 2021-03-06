#ifndef SERVER_MSG_TYPEMAP_H
#define SERVER_MSG_TYPEMAP_H

#include "macro.h"
#include <typeindex>
#include <boost/hana.hpp>
#include <boost/any.hpp>
#include <tuple>

#include <functional>
#include "protofiles/chat_message.pb.h"
#include "typemap.h"
#include "typemapre.h"
#include "buffer_yi_util.hpp"
#include "threads_work.h"

#ifdef __cpluscplus
extern "C" {
#endif

struct Read_IO;

typedef std::shared_ptr<yijian::buffer> Buffer_SP;

namespace yijian {
  namespace threadCurrent {
    static thread_local std::shared_ptr<Read_IO> currentNode_;
    static thread_local chat::ConnectInfo connectInfo_;
    static thread_local chat::ConnectInfoLittle infolittle_;
    static thread_local chat::NodeSelfDevice node_self_;
    static thread_local chat::NodePeerServer node_peer_;
    static thread_local chat::NodeUser node_user_;
    static thread_local chat::NodeSpecifiy node_specifiy_;
    static thread_local uint16_t session_id_;

    std::vector<Buffer_SP> errorBuffer(uint_fast32_t err_num, std::string && err_msg);
  }
}

// gcc6.2 bug
void dispatch(const std::shared_ptr<Read_IO> node, 
    const std::vector<std::shared_ptr<yijian::buffer> > & spvec,
    const uint16_t session_id);
/*
void dispatch(const std::tuple<std::shared_ptr<Read_IO>, 
    std::shared_ptr<yijian::buffer>,
    uint16_t> & t);
    */


template <typename Any>
void mountBuffer2Node(Any &) {
  YILOG_TRACE ("func: {}. ", __func__);
  throw std::system_error(std::error_code(11007, std::generic_category()), 
      "unkonw node type");
}

/*
template <typename Any>
void dispatch(Any & ) {
  YILOG_TRACE ("func: {}. ", __func__);
  throw std::system_error(std::error_code(11000, std::generic_category()), 
      "unkonw dispatch type");
}
*/

#ifdef __cpluscplus
}
#endif

#endif
