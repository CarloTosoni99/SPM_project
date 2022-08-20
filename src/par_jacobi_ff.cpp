#include <vector>
#include <iostream>
#include <functional>
#include <thread>

#include <ff/ff.hpp>
#include <ff/parallel_for.hpp>

#include "my_timer.cpp"
#include "utils.h"

#define MAX_VALUE 32
#define MIN_VALUE -32

using namespace ff;

void par_jacobi_ff(std::vector<std::vector<float>> &a, std::vector<float> &b, std::vector<float> &x, int n, int n_iter, int nw, int chunk_size, int ch_conv, float tol) {

    // Execute the Jacobi method
    int k = 1;
    bool stop = false;

    std::vector<float> xo = x;
    
    // FastFlow's class to implement a map
    ParallelFor pf(nw);

    // This function has to be executed by the ParallelFor object at each Jacobi iteration
    std::function<void(int)> f = [&](const int i) {
        float val = 0.0;
        for (int j = 0; j < n; j++) {
            val += a[i][j]*xo[j];
        }
        val -= a[i][i]*xo[i];
        x[i] = (b[i]-val)/(a[i][i]);
    };

    std::function<void(void)> parjac_ff = [&](){

        while (k <= n_iter) {
            pf.parallel_for(0, n, 1, chunk_size, f);

            k = k + 1;
            xo = x;

            //check if the method has reached the convergene, in case stop the iterations
            if (ch_conv != 0)
                if (compute_norm(std::ref(x), std::ref(xo), n) < tol) {
                    std::cout << "condition for convergence is satisfied" << std::endl;
                    return;
                }
        }

    };

    parjac_ff();
}

int main(int argc, char *argv[]) {

    int seed = std::stoul(argv[1]); //seed to generate random numbers
    int n = std::stoul(argv[2]); //linear system's dimension
    int n_iter = std::stoul(argv[3]); //maximum number of iterations
    int ch_conv = std::stoul(argv[4]); //if it's 1 the programm will check the convergence of jacobi at each iteration, if it's 0 it will not
    float tol = std::atof(argv[5]); //maximum tolerance for convergence, the program will use this value only if ch_conv == 1
    int nw = std::stoul(argv[6]); //parallel degree
    int chunk_size = std::stoul(argv[7]); //chunks' size for the ParallelFor

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
    
    // Compute Jacobi
    par_jacobi_ff(std::ref(a), std::ref(b), std::ref(x), n, n_iter, nw, chunk_size, ch_conv, tol);
    
    // Measure the elapsed time and print the result.
    time_t elapsed = timer.get_time();
    std::cout << "Elapsed time: " << elapsed << std::endl;


    // OPTIONAL to check the error
    //check_error(n, std::ref(a), std::ref(b), std::ref(x));

    return 0;
}
