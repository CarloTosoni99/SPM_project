#include <iostream>
#include <vector>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <chrono>


#include "my_timer.cpp"

using namespace std::chrono_literals;

auto dummyFunction = []() {
  return;
};

auto blockThreads = [](std::condition_variable &cond, std::mutex &ll, bool &awake_thr) {
  std::unique_lock<std::mutex> locking(ll);

  cond.wait(locking, [&]() {return awake_thr;});

  locking.unlock();
  return;
};


int main(int argc, char *argv[]) {

  int nw = std::stoul(argv[1]); //parallel degree
  int dummy_mode = std::stoul(argv[2]); //if it's 1 activates the dummy mode, otw pass 0

  bool awake_thr = false;
  std::condition_variable cond;
  std::mutex ll;


  if (dummy_mode == 0) {

    my_timer thr_timer;
    thr_timer.start_timer();

    std::vector <std::thread> tvec(nw);
    for (int i = 0; i < nw; i++) {
      tvec[i] = std::thread(blockThreads, std::ref(cond), std::ref(ll), std::ref(awake_thr));
    }

    thr_timer.stop_time();

    std::this_thread::sleep_for(2000ms);


    my_timer cond_timer;
    cond_timer.start_timer();
    {
      std::unique_lock <std::mutex> locking(ll);
      awake_thr = true;
      cond.notify_all();
      locking.unlock();
    }
    time_t cond_elapsed = cond_timer.get_time();


    thr_timer.restart_time();

    for (int i = 0; i < nw; i++) {
      tvec[i].join();
    }

    time_t thr_elapsed = thr_timer.final_time();
    std::cout << "elapsed time to fork and join threads " << thr_elapsed << " ... ";
    std::cout << "elapsed time per thread " << (thr_elapsed / nw) << std::endl;

    std::cout << "elapsed time to notify all the threads " << cond_elapsed << " ... ";
    std::cout << "elapsed time per waiting thread " << (cond_elapsed / nw) << std::endl;
  }
  else {

    my_timer thr_timer;
    thr_timer.start_timer();

    std::vector <std::thread> tvec(nw);
    for (int i = 0; i < nw; i++) {
      tvec[i] = std::thread(dummyFunction);
    }

    for (int i = 0; i < nw; i++) {
      tvec[i].join();
    }

    time_t thr_elapsed = thr_timer.get_time();
    std::cout << "elapsed time to fork and join threads " << thr_elapsed << " ... ";
    std::cout << "elapsed time per thread " << (thr_elapsed / nw) << std::endl;
  }
  return 0;
}