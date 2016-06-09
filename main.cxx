#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include <iostream>
#include <sstream>
#include <string.h>
#include <vector>
#include <thread>
#include <future>
#include <random>

#include "runLoop.h"


int main(int argc, char * argv[])
{

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
