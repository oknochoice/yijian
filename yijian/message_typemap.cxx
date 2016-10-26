#include "message_typemap.h"

#include <map>
#include <mutex>

#include "chat_mongo.h"
#include "protofiles/chat_message.pb.h"
#include "pinglist.h"
#include "threads_work.h"
#include <string>
#include <queue>
#include "buffer.h"

#ifdef __cpluscplus
extern "C" {
#endif

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
  throw std::system_error(std::error_code(11000, std::generic_category()), 
      "unkonw dispatch type");
}

enum ChatType : uint8_t {
  error,
  registor,
  login,
  logout,
  user,
  userinfo,
  group,
  groupinfo,
  chatmessage,
  notimessage,
  notiunread
};

constexpr uint8_t dispatchType(chat::Error & ) {
  return ChatType::error;
}
constexpr uint8_t dispatchType(chat::Register & ) {
  return ChatType::registor;
}
constexpr uint8_t dispatchType(chat::Login & ) {
  return ChatType::login;
}
constexpr uint8_t dispatchType(chat::Logout & ) {
  return ChatType::logout;
}
constexpr uint8_t dispatchType(chat::User & ) {
  return ChatType::user;
}
constexpr uint8_t dispatchType(chat::UserInfo & ) {
  return ChatType::userinfo;
}
constexpr uint8_t dispatchType(chat::Group & ) {
  return ChatType::group;
}
constexpr uint8_t dispatchType(chat::GroupInfo & ) {
  return ChatType::groupinfo;
}
constexpr uint8_t dispatchType(chat::ChatMessage & ) {
  return ChatType::chatmessage;
}
constexpr uint8_t dispatchType(chat::Noti_Message & ) {
  return ChatType::notimessage;
}
constexpr uint8_t dispatchType(chat::Noti_Unread & ) {
  return ChatType::notiunread;
}
// dispatch 
void dispatch(chat::Error& ) {

  YILOG_TRACE ("func: {}. ", __func__);

  throw std::system_error(std::error_code(11001, std::generic_category()), 
      "chat::Error can not dispatch to server");

}

template <typename Proto> Buffer_SP
encoding(Proto any) {

    auto buf = std::make_shared<yijian::buffer>();
    buf->data_encoding_length(any.ByteSize());
    buf->data_encoding_type(dispatchType(any));
    any.SerializeToArray(buf->data_encoding_current(), buf->remain_size());
    buf->data_encoding_reset_size(any.ByteSize());

    return buf;
}

std::shared_ptr<yijian::buffer>
dispatch(chat::Register & rollin) {
  
  YILOG_TRACE ("func: {}. ", __func__);

  auto client = yijian::threadCurrent::mongoClient();

  bool isRollin = client.isUserRollined(rollin.phoneno(), rollin.countrycode());
  if (isRollin) {

    auto error = chat::Error();
    error.set_errnum(11002);
    error.set_errmsg("user is already in database");

    return encoding(error);

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

    return encoding(error);

  }

}

std::shared_ptr<yijian::buffer>
dispatch(chat::Login & login) {
  YILOG_TRACE ("func: {}. ", __func__);

  auto client = yijian::threadCurrent::mongoClient();

  bool isRollin = client.isUserRollined(login.phoneno(), login.countrycode());
  if (isRollin) {
    
  }else {
    auto error = chat::Error();
    error.set_errnum(11003);
    error.set_errmsg("user has not enrolled");
  }
}


std::function<Buffer_SP(void)>& 
dispatch(int type, char * header, std::size_t length) {
  auto static map_p = new std::map<int, std::function<Buffer_SP(void)>>();
  std::once_flag flag;
  std::call_once(flag, [&]() {
      (*map_p)[ChatType::error] = [=]() -> Buffer_SP {
        auto chat = chat::Error();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::registor] = [=]() -> Buffer_SP {
        auto chat = chat::Register();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::login] = [=]() -> Buffer_SP {
        auto chat = chat::Login();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::logout] = [=]() -> Buffer_SP {
        auto chat = chat::Logout();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::user] = [=]() -> Buffer_SP {
        auto chat = chat::User();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::userinfo] = [=]() -> Buffer_SP {
        auto chat = chat::UserInfo();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::group] = [=]() -> Buffer_SP {
        auto chat = chat::Group();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::groupinfo] = [=]() -> Buffer_SP {
        auto chat = chat::GroupInfo();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::chatmessage] = [=]() -> Buffer_SP {
        auto chat = chat::ChatMessage();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::notimessage] = [=]() -> Buffer_SP {
        auto chat = chat::Noti_Message();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
      (*map_p)[ChatType::notiunread] = [=]() -> Buffer_SP {
        auto chat = chat::Noti_Unread();
        chat.ParseFromArray(header, length);
        dispatch(chat);
      };
  });
  return (*map_p)[type];
}

struct PingNode;

void dispatch(PingNode* node) {
  auto buf = dispatch(node->buffers_p.front()->datatype(), 
          node->buffers_p.front()->data(),
          node->buffers_p.front()->data_size());
  node->contra_io->buffers_p.push(buf());
}


#ifdef __cpluscplus
}
#endif

