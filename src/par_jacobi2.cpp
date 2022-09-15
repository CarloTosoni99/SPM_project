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

// This function represents the tasks that have to be executed by the threads
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


class TaskQueue {
private:
    // Shared queue
    std::deque<std::function<void()>> task_queue;

    std::condition_variable cond;
    std::mutex ll;

    std::vector<std::pair<int, int>> chunks;
    int num_chunk;

    bool is_done;
    std::vector<bool> queue_filled;
    bool conv;


public:
    // Initialize a data structure for the tasks
    TaskQueue(int n, int nw, int csize);

    // This function is used by the threads to extract tasks from the queue and to execute them
    void extract_tasks(int num_thr);

    // This function prints some stats about the execution time
    void extract_tasks_stats(int num_thr, std::vector<time_t> &wait_time, std::vector<time_t> &ex_time);

    // This function is used by the main thread to insert new tasks in the shared queue
    void insert_tasks(std::vector<std::vector<float>> &a, std::vector<float> &b, std::vector<float> &x, int n,
                     int n_iter, int ch_conv, float tol, int nw);

    // This function prints some stats about the execution time
    void insert_tasks_stats(std::vector<std::vector<float>> &a, std::vector<float> &b, std::vector<float> &x, int n,
                      int n_iter, int ch_conv, float tol, int nw, time_t &rf_queue);

    // Terminate the execution of Jacobi
    void terminate_jacobi();
};

// Initialize a data structure for the tasks
TaskQueue::TaskQueue(int n, int nw, int csize) {

    // Compute every chunk, each chunk states which, and how many iterations each threads has to compute
    num_chunk =  n % csize == 0  ? (n / csize) : (n / csize) + 1;
    std::vector<std::pair<int, int>> ch_vec(num_chunk);
    for(int i = 0; i < num_chunk; i++) {
        int start = csize*i;
        int end = i != num_chunk - 1 ? csize*(i+1) : n;
        ch_vec[i] = std::make_pair(start, end);
    }
    chunks = ch_vec;

    // This array of bool variables are used by the threads to understand when the queue has been refilled with new tasks to complete
    std::vector<bool> qf(nw);
    for(int i = 0; i < nw; i++)
        qf[i] = false;
    queue_filled = qf;

    // This bool variable is used by the threads to understand when they have to terminate their execution
    is_done = false;
    // This bool variable is set to true iff the system achieves convergence
    conv = false;
}

// This function is used by the threads to extract tasks from the queue and to execute them
void TaskQueue::extract_tasks(int num_thr) {

    while(true) {
        bool modified = false;
        std::function<void()> t = []() {return;};
        {
            // The thread will wait on the condition variable until the queue has been refilled
            // with new tasks or the Jacobi method is done
            std::unique_lock<std::mutex> locking(ll);
            cond.wait(locking, [&]() {return queue_filled[num_thr] || is_done;});

            // If the queue is not empty, extract a task...
            if (!task_queue.empty()) {
                t = task_queue.back();
                task_queue.pop_back();
            }
                // ...Otherwise wait on the condition variable
            else {
                if (queue_filled[num_thr]) {
                    queue_filled[num_thr] = false;
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

// This function prints some stats about the execution time
void TaskQueue::extract_tasks_stats(int num_thr, std::vector<time_t> &wait_time, std::vector<time_t> &ex_time) {

    // Timer to measure the time spent by the thread to compute activities
    my_timer ex_timer;

    // TImer to measure the time spent by the thread to wait for the shared queue
    my_timer wait_timer;

    while(true) {
        wait_timer.restart_time();

        bool modified = false;
        std::function<void()> t = []() {return;};
        {
            // The thread will wait on the condition variable until the queue has been refilled
            // with new tasks or the Jacobi method is done
            std::unique_lock<std::mutex> locking(ll);
            cond.wait(locking, [&]() {return queue_filled[num_thr] || is_done;});

            // If the queue is not empty, extract a task...
            if (!task_queue.empty()) {
                t = task_queue.back();
                task_queue.pop_back();
            }
            // ...Otherwise wait on the condition variable
            else {
                if (queue_filled[num_thr]) {
                    queue_filled[num_thr] = false;
                    modified = true;
                }
                if(is_done) {
                    break;
                }
            }
            locking.unlock();
        }
        // If the thread has finished to execute the tasks of an iteration, notify it to the main thread
        if (modified)
            cond.notify_all();

        wait_timer.stop_time();
        ex_timer.restart_time();
        t();
        ex_timer.stop_time();
    }

    wait_time[num_thr] = wait_timer.final_time();
    ex_time[num_thr] = ex_timer.saved_time();
};

// This function is used by the main thread to insert new tasks in the shared queue
void TaskQueue::insert_tasks(std::vector<std::vector<float>> &a, std::vector<float> &b, std::vector<float> &x, int n,
                            int n_iter, int ch_conv, float tol, int nw){

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
        // Notify the waiting threads
        cond.notify_all();


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
                //check if the method has reached the convergence, in case stop the iterations
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
};

// This function prints some stats about the execution time
void TaskQueue::insert_tasks_stats(std::vector<std::vector<float>> &a, std::vector<float> &b, std::vector<float> &x, int n,
                             int n_iter, int ch_conv, float tol, int nw, time_t &rf_queue){

    int k = 1;
    std::vector<float> xo = x;

    my_timer qu_timer;

    while (k <= n_iter && !is_done) {
        // Measure the elapsed time to refill the queue
        qu_timer.restart_time();

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
        // Notify the waiting threads
        cond.notify_all();

        // Pause the timer
        qu_timer.stop_time();

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
                //check if the method has reached the convergence, in case stop the iterations
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

    rf_queue = qu_timer.saved_time();
};


//Terminate the execution of the threads
void TaskQueue::terminate_jacobi() {

    if (!conv) {
        std::unique_lock<std::mutex> locking(ll);
        is_done = true;
        locking.unlock();
        cond.notify_all();
    }
}

int main(int argc, char *argv[]) {

    int seed = std::stoul(argv[1]); //seed to generate random numbers
    int n = std::stoul(argv[2]); //linear system's dimension
    int n_iter = std::stoul(argv[3]); //maximum number of iterations
    int ch_conv = std::stoul(argv[4]); //if it's 1 the programm will check the convergence of jacobi at each iteration, if it's 0 it will not
    float tol = std::atof(argv[5]); //maximum tolerance for convergence, the program will use this value only if ch_conv == 1
    int nw = std::stoul(argv[6]); //parallel degree
    int csize = std::stoul(argv[7]); //chunks' size
    int stats = std::stoul(argv[8]); //if it's 1 the programm will print some stats about the program execution, if it's 0 it will not

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

    // vectors of total waiting time (on the shared queue) and total execution time of each thread, these vectors is
    // used to understand how much a shared queue can affect the performance of par_jacobi2.cpp. It will be filled iff
    // stats == 1
    std::vector<time_t> wait_time(nw, 0);
    std::vector<time_t> ex_time(nw, 0);

    // variable to measure the total time required by the main to refill the queue (this time is plain overhead)
    time_t rf_queue = 0;

    // OPTIONAL, print the system created
    //print_system(n, std::ref(a), std::ref(b));


    // Start to measure the elapsed time
    my_timer timer;
    timer.start_timer();


    TaskQueue my_taskQueue(n, nw, csize);
    std::vector <std::thread> tvec(nw);

    // If stats == 0, the program executes the standard version of the thread_pool (does not measure anything) ...
    if (stats == 0) {
       // Initialise the threads
       for (int i = 0; i < nw; i++) {
           tvec[i] = std::thread(&TaskQueue::extract_tasks, std::ref(my_taskQueue), i);
       }


       my_taskQueue.insert_tasks(std::ref(a), std::ref(b), std::ref(x), n, n_iter, ch_conv, tol, nw);
       my_taskQueue.terminate_jacobi();
    }

    // ... Otherwise the program measures also the time to executes activities and to wait for new activities
    else {
        // Initialise the threads
        for (int i = 0; i < nw; i++) {
            tvec[i] = std::thread(&TaskQueue::extract_tasks_stats, std::ref(my_taskQueue), i, std::ref(wait_time),
                                  std::ref(ex_time));
        }


        my_taskQueue.insert_tasks_stats(std::ref(a), std::ref(b), std::ref(x), n, n_iter, ch_conv, tol, nw,
                                        std::ref(rf_queue));
        my_taskQueue.terminate_jacobi();
    }

    for(int i = 0; i < nw; i++) {
        tvec[i].join();
    }

    // Measure the elapsed time and print it
    time_t elapsed = timer.get_time();
    //std::cout << "elapsed time " << elapsed << std::endl;

    // OPTIONAL to check the error
    //check_error(n, std::ref(a), std::ref(b), std::ref(x));

    if (stats != 0) {
        thr_pool_stats(std::ref(wait_time), std::ref(ex_time), std::ref(rf_queue));
    }

    return 0;
}
