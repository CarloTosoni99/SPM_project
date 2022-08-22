#include <functional>
#include <deque>
#include <mutex>
#include <iostream>
#include <thread>
#include <vector>
#include <condition_variable>

#include "utils.h"
#include "my_timer.cpp"

#define MAX_VALUE 32
#define MIN_VALUE -32



int main(int argc, char *argv[]) {

    int seed = std::stoul(argv[1]); //seed to generate random numbers
    int n = std::stoul(argv[2]); //linear system's dimension
    int n_iter = std::stoul(argv[3]); //maximum number of iterations
    int ch_conv = std::stoul(argv[4]); //if it's 1 the programm will check the convergence of jacobi at each iteration, if it's 0 it will not
    float tol = std::atof(argv[5]); //maximum tolerance for convergence, the program will use this value only if ch_conv == 1
    int nw = std::stoul(argv[6]); //parallel degree
    int csize = std::stoul(argv[7]); //chunks' size

    srand(seed);

    // Creation of matrix A
    std::vector<std::vector<float>> a(n);
    for (int i = 0; i < n; i++) {
        a[i] = std::vector<float>(n);
    }
    // Creation of vector b
    std::vector<float> b(n);
    // Creation of vector x
    std::vector<float> x(n, 0);

    // Initialize the matrices A and b
    initialize_problem(n, std::ref(a), std::ref(b), MIN_VALUE, MAX_VALUE);

    // OPTIONAL, print the system created
    //print_system(n, std::ref(a), std::ref(b));


    // Start to measure the elapsed time
    my_timer timer;
    timer.start_timer();


    std::deque<std::function<void()>> task_queue;
    std::condition_variable cond;
    std::mutex ll;

    // Compute every chunk, each chunk states which, and how many iterations each threads has to compute
    int num_chunk =  n % csize == 0  ? (n / csize) : (n / csize) + 1;
    std::vector<std::pair<int, int>> chunks(num_chunk);
    for(int i = 0; i < num_chunk; i++) {
        int start = csize*i;
        int end = i != num_chunk - 1 ? csize*(i+1) : n;
        chunks[i] = std::make_pair(start, end);
    }


    // This array of bool variables are used by the threads to understand when the queue has been refilled with new tasks to complete
    bool queue_filled [nw];
    for(int i = 0; i < nw; i++) {
        queue_filled[i] = false;
    }
    // This bool variable is used by the threads to understand when they have to terminate their execution
    bool is_done = false;
    // This bool variable is set to true iff the system achieves convergence
    bool conv = false;

    // Task to execute
    auto f = [](std::vector<float>& x, std::vector<std::vector<float>>& a, std::vector<float>& b,
                std::vector<float>& xo, std::pair<int, int>& chunk, int n) {
        float val;
        for (int i = chunk.first; i < chunk.second; i++) {
            val = 0.0;
            for (int j = 0; j < n; j++) {
                val += a[i][j] * xo[j];
            }
            val -= a[i][i]*xo[i];
            x[i] =  (b[i] - val)/(a[i][i]);
        }
    };

    // This function is used by the threads to extract tasks from the queue and to execute them
    auto extract_tasks = [](std::mutex &ll, std::condition_variable &cond, std::deque<std::function<void()>> &task_queue,
                            bool &queue_filled, bool &is_done, int num_thr) {
        while(true) {
            bool modified = false;
            std::function<void()> t = []() {return;};
            {
                // The thread will wait on the condition variable until the queue has been refilled
                // with new tasks or the Jacobi method is done
                std::unique_lock<std::mutex> locking(ll);
                cond.wait(locking, [&]() {return queue_filled || is_done;});
                
                // If the queue is not empty, extract a task...
                if (!task_queue.empty()) {
                    t = task_queue.back();
                    task_queue.pop_back();
                }
                // ...Otherwise wait on the condition variable
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
            // If the thread has finished to execute the tasks of an iteration, notify it to the main thread
            if (modified)
                cond.notify_all();
            t();
        }
    };

    // Initialise the threads
    std::vector<std::thread> tvec(nw);
    for(int i=0; i < nw; i++) {
        tvec[i] = std::thread(extract_tasks, std::ref(ll), std::ref(cond), std::ref(task_queue),
                              std::ref(queue_filled[i]), std::ref(is_done), i);
    }

    int k = 1;
    std::vector<float> xo = x;
    while (k <= n_iter && !is_done) {
        {
            // Acquire the lock to fill the queue with new tasks to be executed
            std::unique_lock<std::mutex> locking(ll);
            for (int i = 0; i < num_chunk; i++) {
                auto fx = std::bind(f, std::ref(x), std::ref(a), std::ref(b), std::ref(xo), std::ref(chunks[i]), n);
                task_queue.push_front(fx);
            }
            for(int i = 0; i < nw; i++)
                queue_filled[i] = true;
            locking.unlock();
        }
        // Notify a waiting thread
        cond.notify_one();


        {
            // Wait every thread before starting a new iteration and refilling the queue
            std::unique_lock<std::mutex> locking(ll);
            cond.wait(locking, [&]() {
                bool restart = true;
                for(int i = 0; i < nw; i++){
                    if (queue_filled[i]){
                        restart = false;
                        break;
                    }
                }
                //check if the method has reached the convergene, in case stop the iterations
                if (ch_conv != 0)
                    if (restart)
                        if (compute_norm(std::ref(x), std::ref(xo), n) < tol) {
                            std::cout << "condition for convergence is satisfied" << std::endl;
                            is_done = true;
                            conv = true;
                            cond.notify_all();
                        }

                return restart;
            });
            locking.unlock();
        }
        k++;
        xo = x;
    }

    
    // Jacobi method is done
    if (!conv) {
        std::unique_lock<std::mutex> locking(ll);
        is_done = true;
        locking.unlock();
        cond.notify_all();
    }



    // Waiting the termination of the threads
    for(int i = 0; i < nw; i++) {
        tvec[i].join();
    }

    // Measure the elapsed time and print it
    time_t elapsed = timer.get_time();
    std::cout << "time elapsed " << elapsed << std::endl;


    // OPTIONAL to check the error
    //check_error(n, std::ref(a), std::ref(b), std::ref(x));

    return 0;
}
