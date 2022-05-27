#include <iostream>
#include <string>
#include <vector>
#include "producer_consumer.h"

int main(int argc, char** argv) {
  if (argc < 3) {
    std::cout << "Too few parameters\n";
    return -1;
  }

  int num_threads = std::stoi(argv[1]);
  if (num_threads <= 0) {
    std::cout << "Number of threads should be positive\n";
    return -1;
  }

  int sleep_limit_time = std::stoi(argv[2]);
  if (sleep_limit_time <= 0) {
    sleep_limit_time = 500;
  }
  bool debug_flag = false;
  if (argc == 4) {
    std::string debug_option = argv[3];
    if (debug_option == "-debug") debug_flag = true;
  }

  std::cout << run_threads(num_threads, sleep_limit_time, debug_flag)
            << std::endl;
  return 0;
}
