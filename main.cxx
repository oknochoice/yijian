#define CATCH_CONFIG_RUNNER
#define YILOG_ON
#include "macro.h"
#include "catch.hpp"

#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>
#include <vector>
#include <thread>
#include <future>
#include <chrono>
#include <iomanip>
#include <list>
#include <deque>
#include <boost/hana.hpp>
#include <ev.h>

void test();
void hana();
void libev();

int main(int argc, char * argv[])
{
//  test();
//	int result = Catch::Session().run();
//	hana();

  libev();
  return 0;
}

ev_io stdin_watcher;
ev_timer timeout_watcher;

static void
stdin_cb(EV_P_ ev_io * w, int revents) {
  puts("stdin ready");
  ev_io_stop(EV_A_ w);
  ev_break(EV_A_ EVBREAK_ALL);
}

static void
timeout_cb(EV_P_ ev_timer * w, int revents) {
  puts("time out");

  ev_break(EV_A_ EVBREAK_ONE);
}

void libev() {
  struct ev_loop * loop = EV_DEFAULT;

  ev_io_init(&stdin_watcher, stdin_cb, 0, EV_READ);
  ev_io_start(loop, &stdin_watcher);

  ev_timer_init(&timeout_watcher, timeout_cb, 5.5, 0.);
  ev_timer_start(loop, &timeout_watcher);

  ev_run(loop, 0);


}

using namespace boost::hana;

template <typename X, typename Y>
struct plus_my {
  using type = integral_constant<
    decltype(X::value + Y::value),
        X::value + Y::value
            >;
};

using three = plus_my<integral_constant<int, 1>, integral_constant<int, 2>>::type;

template <typename V, V v, typename U, U u>
constexpr auto
operator+(integral_constant<V, v>, integral_constant<U, u>) {
  return integral_constant<decltype(u + v), u + v>{};
}

auto three_obj = integral_constant<int, 1>{} + integral_constant<int, 2>{};

template <int i>
constexpr integral_constant<int, i> int_ccc{};

auto three_int_c = int_ccc<1> + int_ccc<2>;

using namespace boost::hana::literals;
auto three_c = 1_c + 2_c;

// error 
template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
  if (std::is_constructible<T, Args...>::value) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
  }else {
    return std::unique_ptr<T>(new T{std::forward<Args>(args)...});
  } 
}

auto one_two_three = boost::hana::if_(boost::hana::true_c, 123, "hello");
auto hello = boost::hana::if_(boost::hana::false_c, 123, "hello");

void hana() {
  std::cout << "hana test begin ..." << std::endl;
  std::cout << "hana test end ..." << std::endl;
}


TEST_CASE("my test", "[test]") {
	REQUIRE( 1 != 1 );
}

TEST_CASE( "vectors can be sized and resized", "[vector]" ) {

    std::vector<int> v( 5 );

    REQUIRE( v.size() == 5 );
    REQUIRE( v.capacity() >= 5 );

    SECTION( "resizing bigger changes size and capacity" ) {
        v.resize( 10 );

        REQUIRE( v.size() == 10 );
        REQUIRE( v.capacity() >= 10 );
    }
    SECTION( "resizing smaller changes size but not capacity" ) {
        v.resize( 0 );

        REQUIRE( v.size() == 0 );
        REQUIRE( v.capacity() >= 5 );
    }
    SECTION( "reserving bigger changes capacity but not size" ) {
        v.reserve( 10 );

        REQUIRE( v.size() == 5 );
        REQUIRE( v.capacity() >= 10 );
    }
    SECTION( "reserving smaller does not change size or capacity" ) {
        v.reserve( 0 );

        REQUIRE( v.size() == 5 );
        REQUIRE( v.capacity() >= 5 );
    }
}

template <typename C>
void printClockData () {

	typedef typename C::period P;
	std::cout << std::fixed << P::num << "/" << P::den << " seconds" << std::endl;
	std::cout << "- precision: ";
	if (std::ratio_less_equal<P, std::milli>::value) {
		typedef typename std::ratio_multiply<P, std::kilo>::type TT;
		std::cout << std::fixed << double(TT::num) / TT::den << " milliseconds" << std::endl;
	}else {
		std::cout << std::fixed << double(P::num) / P::den << " seconds" << std::endl;
	}
	std::cout << "- is_steady: " << std::boolalpha << C::is_steady << std::endl;
}

void test() {
  {
    std::cout << "logger" << std::endl;
    YILOG_CRITICAL("log");
    YILOG_ERROR("log");
  }

	{
		std::cout << "system_clock: " << std::endl;
		printClockData<std::chrono::system_clock>();
		std::cout << "\nhigh_resolution_clock: " << std::endl;
		printClockData<std::chrono::high_resolution_clock>();
		std::cout << "\nsteady_clock: " << std::endl;
		printClockData<std::chrono::steady_clock>();
	}

	{
		auto lamHeap = new std::function<void()>([](){
			printf("lambda heap\n");
		});
		(*lamHeap)();
	}

	{
		std::vector<int> coll1;
		std::list<int> coll2;
		std::deque<int> coll3;

		coll1 = {1, 2, 3};
		coll2 = {3, 2, 1};
		if( std::is_permutation(coll1.cbegin(), coll1.cend(),
								coll2.cbegin())) {
			std::cout << "coll1 and coll2 have equal elements" << std::endl;
		}
	}

	{
		std::vector<std::string> v1 {"a", "bc", "c", "d"};
		std::vector<std::string> v2 {"a", "b", "c", "d"};
 
		std::srand(std::time(0));
		while (!std::lexicographical_compare(v1.begin(), v1.end(),
                                         v2.begin(), v2.end())) {
			for (auto c : v1) std::cout << c << ' ';
			std::cout << ">= ";
			for (auto c : v2) std::cout << c << ' ';
			std::cout << '\n';
	 
			std::random_shuffle(v1.begin(), v1.end());
			std::random_shuffle(v2.begin(), v2.end());
		}
 
		for (auto c : v1) std::cout << c << ' ';
		std::cout << "< ";
		for (auto c : v2) std::cout << c << ' ';
		std::cout << '\n';
	}

	{
		
       std::vector<int> coll = { 5, 3, 9, 1, 3, 4, 8, 2, 6 };
       auto isOdd = [](int elem) {
                        return elem%2 == 1;
					};
		if (is_partitioned (coll.cbegin(), coll.cend(),
                           isOdd)) {
        	std::cout << "coll is partitioned" << std::endl;
            auto pos = partition_point (coll.cbegin(),coll.cend(),
                                       isOdd);
			std::cout << "first even element: " << *pos << std::endl; }
		else {
			std::cout << "coll is not partitioned" << std::endl;
		}
	}

	{
		std::vector<int> v1 = {1,2,2,4,6,7,7,9};
		std::vector<int> v2 = {2,2,2,3,6,6,8,9};
		std::vector<int> v3;
		std::set_difference(v1.begin(), v1.end(),
							v2.begin(), v2.end(),
							std::back_inserter(v3));
		for (const auto i: v3) {
			std::cout << i << " ";
		}
		std::cout << std::endl;
		std::vector<int> v4;
		std::set_difference(v2.begin(), v2.end(),
							v1.begin(), v1.end(),
							std::back_inserter(v4));
		for (const auto i: v4) {
			std::cout << i << " ";
		}
		std::cout << std::endl;
	}
	{
		std::string s1("大地");
		std::cout << s1 << std::endl;
	}

	{
		std::ostream hexout(std::cout.rdbuf());
		hexout.setf(std::ios::hex, std::ios::basefield);
		hexout.setf(std::ios::showpos);

		hexout << "hexout: " << 177 << " ";
		std::cout << "cout: " << 177 << " ";
		hexout << "hexout: " << -49 << " ";
		std::cout << "cout: " << -49 << " ";
		hexout << std::endl;
	}

}

