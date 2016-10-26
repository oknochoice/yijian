#ifndef MESSAGE_TYPEMAP_H
#define MESSAGE_TYPEMAP_H

#include <typeindex>
#include <boost/hana.hpp>
#include <boost/any.hpp>
#include <functional>
#include "pinglist.h"

#ifdef __cpluscplus
extern "C" {
#endif


template <typename Any>
void dispatch(Any & a);

constexpr uint8_t dispatchType(chat::Error & error);

void dispatch(chat::Error& error);

std::shared_ptr<yijian::buffer>
dispatch(chat::Register & rollin);

std::shared_ptr<yijian::buffer>
dispatch(chat::Login & login);

std::function<Buffer_SP(void)>& 
dispatch(int type, char * header, std::size_t length);

void dispatch(PingNode* node);

void dispatch(std::shared_ptr<yijian::buffer> buffer_sp);

#ifdef __cpluscplus
}
#endif

#endif
