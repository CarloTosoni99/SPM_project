#include <functional>
#include <deque>
#include <mutex>
#include <iostream>
#include <thread>
#include <vector>
#include <cmath>
#include <condition_variable>


#include "my_timer.cpp"



using namespace std::chrono_literals;

int main(int argc, char *argv[]) {

    int seed = std::stoul(argv[1]); //seed to generate random numbers
    int n = std::stoul(argv[2]); //linear system's dimension
    int n_iter = std::stoul(argv[3]); //maximum number of iterations
    int nw = std::stoul(argv[4]); //parallel degree
    int csize = std::stoul(argv[5]); //chunks' size


    srand(seed);
    std::vector<std::vector<float>> a(n);
    std::vector<float> b(n);
    std::vector<float> x(n, 0);

    float lo = -32.0;
    float hi = 32.0;

    float lo_d = 32.0*((float)(n-1));
    float hi_d = 32.0*((float)(n+1));

    for (int i = 0; i < n; i++){
        std::vector<float> a_row(n);
        for (int j = 0; j < n; j++){
            if (i == j) {
                a_row[j] = lo_d + static_cast<float> (rand() / static_cast<float>(RAND_MAX/(hi_d-lo_d)));
                if (rand() / static_cast<float>(RAND_MAX) < 0.5)
                    a_row[j] = -a_row[j];
            }
            else {
                a_row[j] = lo + static_cast<float> (rand() / static_cast<float>(RAND_MAX/(hi-lo)));
            }
        }
        a[i] = a_row;
    }


    for (int i = 0; i < n; i++){
        b[i] = lo + static_cast<float> (rand() / static_cast<float>(RAND_MAX/(hi-lo)));;
    }



    my_timer timer;
    timer.start_timer();

    std::deque<std::function<void()>> task_queue;
    std::condition_variable cond;
    std::mutex ll;


    int num_chunk =  n % csize == 0  ? (n / csize) : (n / csize) + 1;
    std::vector<std::pair<int, int>> chunks(num_chunk);
    for(int i = 0; i < num_chunk; i++) {
        int start = csize*i;
        int end = i != num_chunk - 1 ? csize*(i+1) : n;
        chunks[i] = std::make_pair(start, end);
    }


    bool queue_filled [nw];
    for(int i = 0; i < nw; i++) {
        queue_filled[i] = false;
    }
    bool is_done = false;

    auto f = [](std::vector<float>& x, std::vector<std::vector<float>>& a, std::vector<float>& b,
                std::vector<float>& xo, std::pair<int, int>& chunk, int n) {
        for (int i = chunk.first; i < chunk.second; i++) {
            float val = 0.0;
            for (int j = 0; j < n; j++) {
                if (i != j)
                    val = val + a[i][j] * xo[j];
            }
            x[i] = (1 / a[i][i]) * (b[i] - val);
        }
    };


    auto extract_tasks = [](std::mutex &ll, std::condition_variable &cond, std::deque<std::function<void()>> &task_queue,
                            bool &queue_filled, bool &is_done, int num_thr) {
        while(true) {
            bool modified = false;
            std::function<void()> t = []() {return;};
            {
                std::unique_lock<std::mutex> locking(ll);
                cond.wait(locking, [&]() {return queue_filled || is_done;});
                if (!task_queue.empty()) {
                    t = task_queue.back();
                    task_queue.pop_back();
                }
                else {
                    if (queue_filled) {
                        queue_filled = false;
                        modified = true;
                    }
                    if(is_done) {
                        return;
                    }
                }
                locking.unlock();
            }
            if (modified)
                cond.notify_all();
            t();
        }
    };


    std::vector<std::thread> tvec(nw);
    for(int i=0; i < nw; i++) {
        tvec[i] = std::thread(extract_tasks, std::ref(ll), std::ref(cond), std::ref(task_queue),
                              std::ref(queue_filled[i]), std::ref(is_done), i);
    }

    int k = 1;
    std::vector<float> xo = x;
    while (k <= n_iter) {
        {
            std::unique_lock<std::mutex> locking(ll);
            for (int i = 0; i < num_chunk; i++) {
                auto fx = std::bind(f, std::ref(x), std::ref(a), std::ref(b), std::ref(xo), std::ref(chunks[i]), n);
                task_queue.push_front(fx);
            }
            for(int i = 0; i < nw; i++)
                queue_filled[i] = true;
            locking.unlock();
        }
        cond.notify_one();


        {
            std::unique_lock<std::mutex> locking(ll);
            cond.wait(locking, [&]() {
                bool restart = true;
                for(int i = 0; i < nw; i++){
                    if (queue_filled[i]){
                        restart = false;
                        break;
                    }
                }
                return restart;
            });
            locking.unlock();
        }
        k++;
        xo = x;
    }

    {
        std::unique_lock<std::mutex> locking(ll);
        is_done = true;
        locking.unlock();
        cond.notify_all();
    }



    // optional to check the error
    /*
    std::this_thread::sleep_for(2000ms);
    for(int i = 0; i < n; i++){
        float v = 0.0;
        for(int j = 0; j < n; j++){
            v = a[i][j]*x[j] + v;
        }
        v = v - b[i];
        std::cout << "Error at row i " << v << std::endl;
    }
    */

    for(int i = 0; i < nw; i++) {
        tvec[i].join();
    }


    time_t elapsed = timer.get_time();
    std::cout << "time elapsed " << elapsed << std::endl;

    return 0;
}
