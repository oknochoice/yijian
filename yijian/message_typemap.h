#ifndef MESSAGE_TYPEMAP_H
#define MESSAGE_TYPEMAP_H

#include <typeindex>
#include <boost/hana.hpp>
#include <boost/any.hpp>
#include <functional>

// .cc
#include <map>
#include <mutex>

#include "chat_mongo.h"
#include "protofiles/chat_message.pb.h"
#include "pinglist.h"
#include "threads_work.h"
#include <string>

#ifdef __cpluscplus
extern "C" {
#endif

std::function<void(void)> &
dispatch(int type, char * header, std::size_t length);


//.cc
/* //hana library test
namespace hana = boost::hana;

template <typename T>
auto case_ = [](auto f) {
  return hana::make_pair(hana::type_c<T>, f);
};

struct default_t;
auto default_ = case_<default_t>;

template <typename Any, typename Default>
auto process(Any &, std::type_index const &, Default & default_) {
  return default_();
}
template <typename Any, typename Default, typename Case, typename ...Rest>
auto process(Any & a, std::type_index const & t, Default & default_,
            Case & case_, Rest & ...rest) {
  using T = typename decltype(+hana::first(case_))::type;
  if (t == typeid(T)) {
    return hana::second(case_)(*boost::unsafe_any_cast<T>(&a));
  }else {
    return process(a, t, default_, rest...);
  }
}

template <typename Any>
auto switch_(Any & a) {
  return [&a](auto ...cases_) {
    auto cases = hana::make_tuple(cases_...);

    auto default_ = hana::find_if(cases, [](auto const & c) {
        return hana::first(c) == hana::type_c<default_t>;
        });
    static_assert(default_ != hana::nothing, "switch is missing a default_ case");

    auto rest = hana::filter(cases, [](auto & c){
        return hana::first(c) != hana::type_c<default_t>;
        });
    return hana::unpack(rest, [&] (auto & ...rest) {
        return process(a, a.type(), hana::second(*default_), rest...);
        });
  };
}

template <typename Any>
auto dispatch(Any & a) {
  auto r = switch_(a)(
      case_<chat::Error>([](auto m) {
          std::cout << "chat error" << std::endl;
        }), 
      default_([]() {throw std::system("messge type is not find");})
  );
  return r;
}
*/

template <typename Any>
void dispatch(Any & a) {
  YILOG_TRACE ("func: {}. ", __func__);
  throw std::system_error(std::error_code(11000, std::generic_category()), "unkonw dispatch type");
}

void dispatch(chat::Error& error) {

  YILOG_TRACE ("func: {}. ", __func__);

  throw std::system_error(std::error_code(11001, std::generic_category()), "chat::Error can not dispatch to server");

}

void dispatch(chat::Register & rollin) {
  
  YILOG_TRACE ("func: {}. ", __func__);

  auto client = yijian::threadCurrent::mongoClient();

  bool isRollin = client.isUserRollined(rollin.phoneno(), rollin.countrycode());
  if (isRollin) {
//    throw std::system_error(std::error_code(11002, std::generic_category()), "user is already in database");
    auto error = chat::Error();
    error.set_errnum(10002);
    error.set_errmsg("user is already in database");
  }else {
    auto user = chat::User();
    user.set_phoneno(rollin.phoneno());
    user.set_countrycode(rollin.countrycode());
    user.set_password(rollin.password());
    user.set_nickname(rollin.nickname());
    client.enrollUser(user);

    auto error = chat::Error();
    error.set_errnum(0);
    error.set_errmsg("success");
  }

}

void dispatch(int type, char * header, std::size_t length) {
  auto static map_p = new std::map<int, std::function<void(void)>>();
  std::once_flag flag;
  std::call_once(flag, [&]() {
      (*map_p)[0] = []() {
        auto chat = chat::Error();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[1] = []() {
        auto chat = chat::Register();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[2] = []() {
        auto chat = chat::Login();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[3] = []() {
        auto chat = chat::Logout();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[4] = []() {
        auto chat = chat::User();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[5] = []() {
        auto chat = chat::UserInfo();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[6] = []() {
        auto chat = chat::Group();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[7] = []() {
        auto chat = chat::GroupInfo();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[8] = []() {
        auto chat = chat::ChatMessage();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[9] = []() {
        auto chat = chat::Noti_Message();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[10] = []() {
        auto chat = chat::Noti_Unread();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
  });
  (*map_p)[type]();
}

void dispatch(PingNode* node) {
  dispatch(node->buffer->datatype(), 
          node->buffer->data(),
          node->buffer->data_size());
}

#ifdef __cpluscplus
}
#endif

#endif
