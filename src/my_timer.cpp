
#include <chrono>

using time_t = long int;

// class to measure the elapsed time, it measures in microseconds
class my_timer {

private:
  std::chrono::steady_clock::time_point start;
  time_t par_time;

public:
  my_timer() {
    par_time = 0;
  }

  // Start to measure the elapsed time
  void start_timer() {
    par_time = 0;
    start = std::chrono::steady_clock::now();
  }

  // pause the timer, the time measured up to its invocation will be saved in par_time
  void stop_time() {
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto delta = end - start;
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(delta).count();
    par_time += elapsed;

    start = std::chrono::steady_clock::now();
  }

  // restart the timer previously paused, the value in par_time will be maintained
  void restart_time() {
    start = std::chrono::steady_clock::now();
  }

  // return the elapsed time, and restart the state of the timer
  time_t final_time() {
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto delta = end - start;
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(delta).count();

    time_t prv_time = par_time;
    par_time = 0;

    return prv_time + elapsed;
  }

  // return only the state of the timer (i.e. par_time)
  time_t saved_time() {
      return par_time;
  }

  // return the elapsed time, without considering the state of the timer (i.e. par_time)
  time_t get_time() {
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto delta = end - start;
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(delta).count();
    return elapsed;
  }
};
