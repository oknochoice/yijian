#define CATCH_CONFIG_RUNNER
#define YILOG_ON
#include "macro.h"
#include "catch.hpp"
#include "spdlog/spdlog.h"

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

int main(int argc, char * argv[])
{
  boost::any a = 'x';
  boost::any b = 1;
  boost::any d = 2.5;
  dispatch(a);
  dispatch(b);
  dispatch(d);
  std::vector<int> v = std::vector<int>();

  return 0;
}




