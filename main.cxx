#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include <iostream>
#include <sstream>
#include <string.h>
#include <vector>
#include <thread>
#include <future>
#include <random>
#include <chrono>
#include <iomanip>
#include <list>
#include <deque>

#include "runLoop.h"

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


int main(int argc, char * argv[])
{
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

	int result = Catch::Session().run(argc, argv);

	return result;

	/*
	std::thread t1([]{
			std::cout << "t1 begin..." << std::endl;
			yijian::runLoop::currentRunloop(1)->run();
			std::cout << "t1 end..." << std::endl;
			});
	t1.detach();

	std::thread t2([]{
			std::cout << "t2 begin..." << std::endl;
			yijian::runLoop::currentRunloop(2)->run();
			std::cout << "t2 end..." << std::endl;
			});
	t2.detach();

	unsigned int index = 1;
	std::default_random_engine e;
	std::uniform_int_distribution<unsigned> u(0,100);

	while (1) {
		unsigned int randomNum = u(e);
		index = randomNum % 2 + 1;
		std::cout << "random number = " << randomNum<< ", index = "<< index << std::endl;
		yijian::runLoop::addFunctionAndWakeup([=] () mutable{
					std::cout << "t" << yijian::currentThread::runloopNum << ": running...  " << std::endl;
					},index);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	
	return 0;
	*/
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
