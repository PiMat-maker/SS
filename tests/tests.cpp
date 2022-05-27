#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <producer_consumer.h>

TEST_CASE("just_example") { CHECK(4 == 4); }

TEST_CASE("tests") {
  system(
      "clang++ *.cpp -g -std=c++17 -O3 -Wall -Wextra -pthread -pedantic -o "
      "posix");
  CHECK(system("echo '1 2 3' | timeout 3s ./posix 3 300") == 6);
  CHECK(system("echo '1 2' | timeout 3s ./posix 6 30") == 3);
}

