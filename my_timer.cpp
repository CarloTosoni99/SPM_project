
#include <chrono>

using time_t = long int;

class my_timer {

private:
  std::chrono::steady_clock::time_point start;

public:
  my_timer() {}

  void start_timer() {
    start = std::chrono::steady_clock::now();
  }

  time_t get_time() {  //return the number of microseconds elapsed since the execution of the function start_timer()
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto delta = end - start;
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(delta).count();
    return elapsed;
  }
};
