#include "macro.h"
#include "spdlog/spdlog.h"
#include "yijian/pinglist.h"
#include "yijian/lib_client.h"
#include "yijian/mongo.h"
#include "yijian/protofiles/chat_message.pb.h"
#include "yijian/message_typemap.h"
#include "buffer.h"
#include <iostream>
#include <unistd.h>

/*
#include <boost/hana.hpp>
#include <boost/any.hpp>
#include <boost/hana/integral_constant.hpp>
#include <boost/hana/type.hpp>
#include <boost/hana/map.hpp>
#include <boost/hana/pair.hpp>
#include <boost/hana/assert.hpp>
#include <boost/any.hpp>
#include <tuple>
#include "yijian/protofiles/chat_message.pb.h"

#include <iostream>
#include <string>
#include <typeindex>
#include <string>

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
  switch_(a)(
      case_<int>([](auto i) { std::cout << i << std::endl;}), 
      case_<char>([](auto c) { std::cout << c << std::endl;}), 
      default_([]() {std::cout << "default" << std::endl;})
      );
}
*/

/*
using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::array;
*/

int main(int argc, char * argv[])
{
  initConsoleLog();
  std::cout << "main" << std::endl;
  YILOG_DEBUG("start");

  std::mutex mutex;
  std::unique_lock<std::mutex> ul(mutex);
  ul.unlock();

  /*
  mongocxx::uri uri("mongodb://localhost:27017");
	mongocxx::client client(uri);

	auto db = client["test"];
	auto user = db["user"];

  auto opt = mongocxx::options::update{};
  opt.upsert(true);

  user.update_one(
      document{} 
      << "name" << "yijian"
      << "age" << 777
      << finalize,
      document{} << "$set" << open_document
      << "name" << "yijian"
      << "age" << 777
      << "status" << "very good" 
      << "friend" << open_array << close_array  << close_document
      << finalize,
      opt
      );

  auto arrayBuild = array{};
  for (int i = 0; i < 9; ++i) {
    arrayBuild << i;
  }
  auto array = arrayBuild << finalize;

  auto build = bsoncxx::builder::stream::document{};
  build
    << "name" << "jiwei" 
    << "age" << 1999
    << "status" << "good"
    << "friend" << array;
  auto doc = build
    << bsoncxx::builder::stream::finalize;
  */
//  user.insert_one(doc.view());

//  std::string userid = "5827e5de4b99d9495f68d141";
  /*
  {
    std::vector<void *> vector;
    vector.push_back(nullptr);
    vector.push_back(nullptr);
    vector.push_back(nullptr);
    vector.push_back(nullptr);
    std::cout << "vector size: " << vector.size() << std::endl;
    for (auto & p: vector) {
      if (p == nullptr) {
        std::cout << "nullptr" << std::endl;
      }
    }
  }
  {
    std::vector<std::pair<std::shared_ptr<int64_t>, std::string>> vector;
    vector.push_back(std::make_pair(nullptr, ""));
    std::cout << "vector size: " << vector.size() << std::endl;
  }
  */


  return 0;
}




