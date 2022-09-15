#include <iostream>
#include <cmath>
#include <random>
#include <vector>
#include <climits>

#include "utils.h"


// This function initiliazes the elements of the matrix A and the vector b
void initialize_problem(int n, std::vector<std::vector<float>> &a, std::vector<float> &b, float min_value, float max_value) {
    
    
    // Random initialization of matrix A
    for(int i = 0; i < n; i++) {
        for(int j = 0; j < n; j++) {
            a[i][j] = min_value + static_cast<float> (rand() / static_cast<float>(RAND_MAX/(max_value-min_value)));
        }
    }
    
    
    
    // Transform matrix A into a strictly diagonally matrix
    float sum;
    for(int i = 0; i <n; i++) {
        // Store the absolute sum of the i-th row
        sum = 0.0;
        
        for(int j = 0; j < n; j++) {
            sum += std::abs(a[i][j]);
        }
        sum -= std::abs(a[i][i]);
        
        //check if the elements on the diagonal has to be modified
        if (std::abs(a[i][i]) <= sum){
            if (a[i][i] < 0) 
                a[i][i] = -sum - 10;
            else
                a[i][i] = sum + 10;
        }
    }
    
    // Random initialization of vector b
    for (int i = 0; i < n; i++){
        b[i] = min_value + static_cast<float> (rand() / static_cast<float>(RAND_MAX/(max_value-min_value)));;
    }
    
}


// Compute the stopping criterion to understand if Jacobi has achieved convergence.
// Executed iff ch_conv = 1
float compute_norm(std::vector<float>& x, std::vector<float>& xo, int n) {
    
    // This function computes the stopping criterion ||x - x_old||/||x|| if ch_conv == 1
    float num = 0.0;
    for(int i = 0; i < n; i++) {
        num = num + ((x[i] - xo[i])*(x[i] - xo[i]));
    }
    num = std::sqrt(num);

    float den = 0.0;
    for(int i = 0; i < n; i++) {
        den = den + (x[i]*x[i]);
    }
    den = std::sqrt(den);

    return num / den;
}


//OPTIONAL you can use this function to print the system created
void print_system(int n, std::vector<std::vector<float>> &a, std::vector<float> &b) {
    
    std::cout << std::endl;
    std::cout << "printing the matrix A" << std::endl;
    for(int i = 0; i < n; i++) {
        for(int j = 0; j < n; j++) {
            std::cout << a[i][j] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
    
    std::cout << "printing the diagonal of A" << std::endl;
    for(int i = 0; i < n; i++) {
        std::cout << a[i][i] << " ";
    }
    std::cout << std::endl;
    std::cout << std::endl;
    
    std::cout << "printing the vector b" << std::endl;
    for(int i = 0; i < n; i++) {
        std::cout << b[i] << " ";
    }
    std::cout << std::endl;
    std::cout << std::endl;

}

// OPTIONAL, this function checks the error at the end of Jacobi
void check_error(int n, std::vector<std::vector<float>> &a, std::vector<float> &b, std::vector<float> &x) {
    
    float err;
    for(int i = 0; i < n; i++) {
        err = 0.0;
        for(int j = 0; j < n; j++){
            err = a[i][j]*x[j] + err;
        }
        err = err - b[i];
        std::cout << "Error at row i " << err << std::endl;
    }
}


// This function is used by the program par_jacobi.cpp (barriers) to compute: elapsed time of the fastest thread,
// elapsed time of the slowest thread, average elapsed time of all the threads, maximum waiting time, average waiting
// time, percentage active time.
void barrier_stats(std::vector<time_t> &wait_time, int iter){

    time_t fs_thr = LONG_MAX; // Fastest thread time
    time_t sl_thr = LONG_MIN; // Slowest thread time
    time_t avg_thr = 0; // Average threads time
    time_t max_wt = LONG_MIN; // Maximum waiting time
    time_t avg_wt = 0; // Average waiting time
    float perc_at = 0.0; // percentage active time

    int n = wait_time.size();

    for (int i = 0; i < n; i++) {

        if (wait_time[i] < fs_thr)
            fs_thr = wait_time[i];
        if (wait_time[i] > sl_thr)
            sl_thr = wait_time[i];

        avg_thr += wait_time[i];
    }
    avg_thr = (long) avg_thr / n;

    for(int i = 0; i < n; i++) {

        if (sl_thr - wait_time[i] > max_wt)
            max_wt = sl_thr - wait_time[i];

        avg_wt += sl_thr - wait_time[i];
    }

    avg_wt = (long) avg_wt / n;
    perc_at = ((float) avg_thr) / ((float) avg_thr + avg_wt);

    std::cout << "Stats of iteration " << iter << std::endl;
    std::cout << "Fastest thread time: " << fs_thr << std::endl;
    std::cout << "Slowest thread time: " << sl_thr << std::endl;
    std::cout << "Average threads time: " << avg_thr << std::endl;
    std::cout << "Maximum waiting time: " << max_wt << std::endl;
    std::cout << "Average waiting time: " << avg_wt << std::endl;
    std::cout << "Percentage active time: " << perc_at << std::endl;

}


// This function is used by the program par_jacobi2.cpp (thread pool) to compute: lowest total execution time among all
// threads, highest total execution time among all the threads, average execution time of the threads, lowest total
// waiting time among all the threads, highest total waiting time among all the threads, average waiting time of the threads,
// average ratio execution time/(execution time + waiting time) of the threads, total time needed to refill the queue by
// the main thread
void thr_pool_stats(std::vector<time_t> &wait_time, std::vector<time_t> &ex_time, time_t &rf_queue) {

    time_t lw_et = LONG_MAX; // lowest total execution time
    time_t hi_et = LONG_MIN; // highest total execution time
    time_t avg_et = 0; // average execution time
    time_t lw_wt = LONG_MAX; // lowest total waiting time
    time_t hi_wt = LONG_MIN; // highest total waiting time
    time_t avg_wt = 0; // average waiting time
    float avg_rt = 0.0; // average ratio execution time/(execution time + waiting time)

    int n = wait_time.size();

    for (int i = 0; i < n; i++) {

        if (ex_time[i] < lw_et)
            lw_et = ex_time[i];
        if (wait_time[i] < lw_wt)
            lw_wt = wait_time[i];

        if (ex_time[i] > hi_et)
            hi_et = ex_time[i];
        if (wait_time[i] > hi_wt)
            hi_wt = wait_time[i];

        avg_et += ex_time[i];
        avg_wt += wait_time[i];

    }

    avg_et = (long) avg_et / n;
    avg_wt = (long) avg_wt / n;
    avg_rt = ((float) avg_et) / ((float) avg_et + avg_wt);


    std::cout << "Lowest total execution time: " << lw_et << std::endl;
    std::cout << "Highest total execution time: " << hi_et << std::endl;
    std::cout << "Average execution time: " << avg_et << std::endl;
    std::cout << "Lowest total waiting time: " << lw_wt << std::endl;
    std::cout << "Highest total waiting time: " << hi_wt << std::endl;
    std::cout << "Average waiting time: " << avg_wt << std::endl;
    std::cout << "Average ratio execution time/(execution time + waiting time): " << avg_rt << std::endl;
    std::cout << "Total time needed to refill the queue: " << rf_queue << std::endl;
}