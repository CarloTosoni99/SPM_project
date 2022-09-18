#include <vector>

using time_t = long int;


// Initialize the matrices A and b of the linear system
void initialize_problem(int n, std::vector<std::vector<float>> &a, std::vector<float> &b, float min_value, float max_value);

// Compute the stopping criterion to understand if Jacobi has achieved convergence.
// Executed iff ch_conv = 1
float compute_norm(std::vector<float>& x, std::vector<float>& xo, int n);


// OPTIONAL, print the matrices A and b of the linear system
void print_system(int n, std::vector<std::vector<float>> &a, std::vector<float> &b);

// OPTIONAL, this function checks the error at the end of Jacobi
void check_error(int n, std::vector<std::vector<float>> &a, std::vector<float> &b, std::vector<float> &x);

// This function is used by the program par_jacobi.cpp (barriers) to compute: the time spent to execute subtasks and to
// wait on the barrier by each thread
void barrier_elapsed_time(std::vector<time_t> &wait_time, std::vector<time_t> &tot_wait_time,
                          std::vector<time_t> &tot_ex_time);

// This function is used by the program par_jacobi.cpp (barriers) to compute: elapsed execution time of the fastest
// thread, elapsed execution time of the slowest thread, average elapsed execution time of all the threads, maximum
// waiting time, minimum waiting time, average waiting time, percentage active time.
void barrier_stats(std::vector<time_t> &tot_wait_time, std::vector<time_t> &tot_ex_time);

// This function is used by the program par_jacobi2.cpp (thread pool) to compute: lowest total execution time among all
// threads, highest execution time among all the threads, average execution time of the threads, lowest waiting time
// among all the threads, highest waiting time among all the threads, average waiting time of the threads, average ratio
// execution time/(execution time + waiting time) of the threads, total time needed to refill the queue by the main
// thread
void thr_pool_stats(std::vector<time_t> &wait_time, std::vector<time_t> &ex_time, time_t &rf_queue);