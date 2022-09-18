#include <iostream>
#include <cmath>
#include <thread>
#include <functional>
#include <barrier>
#include <vector>

#include "utils.h"
#include "my_timer.cpp"

#define MAX_VALUE 32
#define MIN_VALUE -32

// Standard version of the parallel jacobi algorithm implemented using barriers, this function is executed iff
// it is passed the argument stats == 0 to the program
void par_jacobi(std::vector<std::vector<float>> &a, std::vector<float> &b, std::vector<float> &x, int n, int n_iter,
                float tol, int ch_conv, int nw) {

    int k = 1;
    bool stop = false;

    std::vector<float> xo = x;

    // This barrier is required to wait all the threads at the end of each Jacobi iteration
    std::barrier bar(nw, [&]() {
        if (ch_conv != 0) {
            stop = compute_norm(std::ref(x), std::ref(xo), n) < tol;
            if (stop)
                std::cout << "condition for convergence is satisfied" << std::endl;
        }
        k = k + 1;
        xo = x;
    });

    // start the parallel Jacobi method
    std::function<void(int)> parjac = [&](int thr_n){

        float val;
        while (k <= n_iter) {
            for (int i = thr_n; i < n; i += nw) {
                val = 0.0;
                for (int j = 0; j < n; j++) {
                    val += a[i][j]*xo[j];
                }
                val -= a[i][i]*xo[i];
                x[i] = (b[i]-val)/a[i][i];
            }
            // Waiting the other threads...
            bar.arrive_and_wait();
            if (stop)
                return;
        }

        return;
    };
    
    // Initialisation of the threads
    std::vector<std::thread> tvec(nw);
    for (int i = 0; i < nw; i++) {
        tvec[i] = std::thread(parjac, i);
    }
    
    // Waiting the threads
    for(std::thread &thr : tvec) {
        thr.join();
    }

    return;
}


// Second version of the parallel Jacobi algorithm implemented using barriers, this version prints some stats about the
// execution of the Jacobi method. This version has been separated from the standard one due to its slight higher
// overhead. This version will be executed iff it is passed the argument stats == 1 to the program
void par_jacobi_stats(std::vector<std::vector<float>> &a, std::vector<float> &b, std::vector<float> &x, int n, int n_iter,
                float tol, int ch_conv, int nw, std::vector<time_t> &wait_time, std::vector<time_t> &tot_wait_time,
                std::vector<time_t> &tot_ex_time) {

    int k = 1;
    bool stop = false;

    std::vector<float> xo = x;


    // This barrier is required to wait all the threads at the end of each Jacobi iteration, the time required to
    // initialize the barrier will be measured by the object "btimer"

    my_timer btimer;
    btimer.start_timer();

    std::barrier bar(nw, [&]() {

        // At the end of each iteration, update the total waiting time and the total execution time of each thread
        barrier_elapsed_time(std::ref(wait_time), std::ref(tot_wait_time), std::ref(tot_ex_time));

        if (ch_conv != 0) {
            stop = compute_norm(std::ref(x), std::ref(xo), n) < tol;
            if (stop)
                std::cout << "condition for convergence is satisfied" << std::endl;
        }
        k = k + 1;
        xo = x;
    });

    // Stop the timer and print the elapsed time to initialise the barrier
    time_t btime = btimer.get_time();
    std::cout << "Time to initialize the barrier: " << btime << std::endl;

    // start the parallel Jacobi method
    std::function<void(int)> parjac = [&](int thr_n){

        // This timer measures the time needed by the thread to execute all its subtasks of a single Jacobi iteration
        my_timer iter_timer;

        float val;
        while (k <= n_iter) {

            // Start the timer
            iter_timer.start_timer();

            for (int i = thr_n; i < n; i += nw) {
                val = 0.0;
                for (int j = 0; j < n; j++) {
                    val += a[i][j]*xo[j];
                }
                val -= a[i][i]*xo[i];
                x[i] = (b[i]-val)/a[i][i];
            }

            // stop the timer and save the result on the vector wait_time
            wait_time[thr_n] = iter_timer.get_time();

            // wait the other threads...
            bar.arrive_and_wait();
            if (stop)
                return;
        }

        return;
    };

    // Initialisation of the threads
    std::vector<std::thread> tvec(nw);
    for (int i = 0; i < nw; i++) {
        tvec[i] = std::thread(parjac, i);
    }

    // Waiting the threads
    for(std::thread &thr : tvec) {
        thr.join();
    }

    return;
}


int main(int argc, char *argv[]){

    int seed = std::stoul(argv[1]); //seed to generate random numbers
    int n = std::stoul(argv[2]); //linear system's dimension
    int n_iter = std::stoul(argv[3]); //maximum number of iterations
    int ch_conv = std::stoul(argv[4]); //if it's 1 the programm will check the convergence of jacobi at each iteration, if it's 0 it will not
    float tol = std::atof(argv[5]); //maximum tolerance for convergence, the program will use this value only if ch_conv == 1
    int nw = std::stoul(argv[6]); //parallel degree
    int stats = std::stoul(argv[7]); //if it's 1 the programm will print some stats about the program execution, if it's 0 it will not

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

    // vector of the average waiting time of each thread, this vector is used to understand how much a static
    // load balancing policy affects the performance of par_jacobi.cpp. It will be filled iff stats == 1
    std::vector<time_t> wait_time(nw, 0);
    // total waiting time of each thread
    std::vector<time_t> tot_wait_time(nw, 0);
    // total execution time of each thread
    std::vector<time_t> tot_ex_time(nw, 0);

    // Initialize the matrices A and b
    initialize_problem(n, std::ref(a), std::ref(b), MIN_VALUE, MAX_VALUE);
    
    // OPTIONAL, print the system created
    //print_system(n, std::ref(a), std::ref(b));
    

    // Start to measure the elapsed time
    my_timer timer;
    timer.start_timer();
    
    // Compute Jacobi1
    // if stats is equal to 0, the program will execute the standard version of the parallel Jacobi algorithm
    if (stats == 0)
        par_jacobi(std::ref(a), std::ref(b), std::ref(x), n, n_iter, tol, ch_conv, nw);
    // Otherwise, if stats is equal to 1, the program will execute the version of the parallel Jacobi algorithm that
    // prints some data about its execution time
    else
        par_jacobi_stats(std::ref(a), std::ref(b), std::ref(x), n, n_iter, tol, ch_conv, nw, std::ref(wait_time),
                         std::ref(tot_wait_time), std::ref(tot_ex_time));


    // This function is used to find: elapsed time of the fastest thread, elapsed time of the slowest thread,
    // average elapsed time of all the threads, maximum waiting time, minimum waiting time, average waiting time. Called
    // only if stats == 1
    if (stats != 0)
        barrier_stats(std::ref(tot_wait_time), std::ref(tot_ex_time));

    // Measure the elapsed time and print the result.
    time_t elapsed = timer.get_time();
    std::cout << "Elapsed time: " << elapsed << std::endl;

    
    // OPTIONAL to check the error
    //check_error(n, std::ref(a), std::ref(b), std::ref(x));

    return 0;
}
