#include <functional>
#include <deque>
#include <mutex>
#include <iostream>
#include <thread>
#include <vector>
#include <cmath>
#include <condition_variable>
#include <atomic>


#include "my_timer.cpp"

using namespace std::chrono_literals;


int main(int argc, char *argv[]) {

    int seed = std::stoul(argv[1]); //seed to generate random numbers
    int n = std::stoul(argv[2]); //linear system's dimension
    int n_iter = std::stoul(argv[3]); //maximum number of iterations
    //int ch_conv = std::stoul(argv[4]); //if it's 1 the programm will check the convergence of jacobi at each iteration, if it's 0 it will not
    //float tol = std::atof(argv[5]); //maximum tolerance for convergence, the program will use this value only if ch_conv ==
    int nw = std::stoul(argv[4]); //parallel degree //CAMBIA IL NUMERO SE RIMETTI LA CONVERGENZA
    int csize = std::stoul(argv[5]); //chunks' size //CAMBIA IL NUMERO SE RIMETTI LA CONVERGENZA


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


    std::vector<std::atomic<bool>> thr_fin (nw);
    for(int i = 0; i < nw; i++) {
        thr_fin[i] = false;
    }
    std::vector<std::atomic<bool>> queue_filled (nw);
    for(int i = 0; i < nw; i++) {
        queue_filled[i] = false;
    }
    std::atomic<bool> is_done;
    is_done = false;

    auto f = [](std::vector<float>& x, std::vector<std::vector<float>>& a, std::vector<float>& b, std::vector<float>& xo, std::pair<int, int>& chunk, int n) {
        for (int i = chunk.first; i < chunk.second; i++) {
            float val = 0.0;
            for (int j = 0; j < n; j++) {
                if (i != j)
                    val = val + a[i][j] * xo[j];
            }
            x[i] = (1 / a[i][i]) * (b[i] - val);
        }
    };


    auto extract_tasks = [](std::mutex &ll, std::condition_variable &cond, std::deque<std::function<void()>> &task_queue, std::atomic<bool> &thr_fin,
                            std::atomic<bool> &queue_filled, std::atomic<bool> &is_done, int num_thr) {
        while(!is_done || !thr_fin) {
            std::function<void()> t = []() {return;};
            {
                std::unique_lock<std::mutex> locking(ll);
                //cond.wait(locking);
                if (!task_queue.empty()) {
                    //std::cout << "worker here number " << num_thr << ", taking a task" << std::endl;
                    t = task_queue.back();
                    task_queue.pop_back();
                }
                else {
                    //std::cout << "yoyo" << std::endl;
                    if (queue_filled) {
                        queue_filled = false;
                        thr_fin = true;
                    }
                }
                locking.unlock();
            }
            t();
        }
    };


    std::vector<std::thread> tvec(nw);
    for(int i=0; i < nw; i++) {
        tvec[i] = std::thread(extract_tasks, std::ref(ll), std::ref(cond), std::ref(task_queue),
                              std::ref(thr_fin[i]), std::ref(queue_filled[i]), std::ref(is_done), i);
    }

    int k = 1;
    std::vector<float> xo = x;
    while (k <= n_iter) {
        {
            std::unique_lock<std::mutex> locking(ll);
            for (int i = 0; i < num_chunk; i++) {
                auto fx = std::bind(f, std::ref(x), std::ref(a), std::ref(b), std::ref(xo), std::ref(chunks[i]), n);
                //std::cout << "main thread here, I'm pushing a task..." << std::endl;
                task_queue.push_front(fx);
            }
            for(int i = 0; i < nw; i++)
                queue_filled[i] = true;
            locking.unlock();
        }
        cond.notify_one();

        bool restart = false;
        while (!restart) {
            restart = true;
            for(int i = 0; i < nw; i++) {
                if (!thr_fin[i])
                    restart = false;
            }
        }

        for(int i = 0; i < nw; i++){
            thr_fin[i] = false;
        }

        //std::cout << "iteration number "<< k << " computed, ready for next iteration" << std::endl;
        k++;
        xo = x;
    }
    for(int i = 0; i < nw; i++){
        thr_fin[i] = true;
    }
    is_done = true;

    std::cout << "exited from main while" << std::endl;

    //std::this_thread::sleep_for(2000ms);

    // optional to check the error
    /*
    for(int i = 0; i < n; i++){
        float v = 0.0;
        for(int j = 0; j < n; j++){
            v = a[i][j]*x[j] + v;
        }
        v = v - b[i];
        std::cout << "Error at row i " << v << std::endl;
    }*/

    for(int i = 0; i < nw; i++) {
        tvec[i].join();
    }


    time_t elapsed = timer.get_time();
    std::cout << "time elapsed " << elapsed << std::endl;

    return 0;
}
