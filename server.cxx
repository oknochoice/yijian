#define CATCH_CONFIG_RUNNER
#define YILOG_ON
#include "macro.h"
#include "catch.hpp"
#include "spdlog/spdlog.h"

#include <boost/hana.hpp>
#include <boost/hana/any.hpp>
#include <boost/hana/integral_constant.hpp>
#include <boost/hana/type.hpp>
#include <boost/hana/map.hpp>
#include <boost/hana/pair.hpp>
#include <boost/hana/assert.hpp>
#include <tuple>
#include "yijian/protofiles/chat_message.pb.h"

#include <iostream>
#include <string>


template <typename Any>
static void 
insertNumtpyeMap(int i) {
//  boost::hana::insert(num_type(), 
      //boost::hana::make_pair(i, boost::hana::type_c<Any>));
  boost::hana::insert(type_num(), 
      boost::hana::make_pair(boost::hana::type_c<Any>, std::to_string(i)));
}

int main(int argc, char * argv[])
{
  auto tuple = std::make_tuple("12", 1, 3.5, '1', "123");
  for (int i = 0; i < 5; i++) {
  }

  insertNumtpyeMap<chat::Error>(0);
  insertNumtpyeMap<chat::User>(1);
  insertNumtpyeMap<chat::UserInfo>(2);
  insertNumtpyeMap<chat::Group>(3);
  insertNumtpyeMap<chat::GroupInfo>(4);
  insertNumtpyeMap<chat::Register>(5);
  insertNumtpyeMap<chat::Login>(6);
  insertNumtpyeMap<chat::Logout>(7);
  insertNumtpyeMap<chat::ChatMessage>(8);
  insertNumtpyeMap<chat::Noti_Unread>(9);
  insertNumtpyeMap<chat::Noti_Message>(10);

  //BOOST_HANA_CONSTANT_CHECK(num_type()[0] == boost::hana::type_c<chat::Error>);
  BOOST_HANA_RUNTIME_CHECK(type_num()[boost::hana::type_c<chat::Error>] == "0");
	namespace	 hana = boost::hana;
  
	auto m = hana::make_map(
        hana::make_pair(hana::type_c<int>, std::string{"int"}),
        hana::make_pair(hana::int_c<3>, std::string{"3"})
    );
    BOOST_HANA_RUNTIME_CHECK(hana::at_key(m, hana::type_c<int>) == "int");
    // usage as operator[]
    BOOST_HANA_RUNTIME_CHECK(m[hana::type_c<int>] == "int");
    BOOST_HANA_RUNTIME_CHECK(m[hana::int_c<3>] == "3");

  return 0;
}
