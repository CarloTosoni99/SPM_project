#include <vector>
#include <iostream>
#include <functional>
#include <thread>

#include <ff/ff.hpp>
#include <ff/parallel_for.hpp>

#include "my_timer.cpp"


using namespace ff;

void jacobi(std::vector<float>& a, std::vector<float>& b, std::vector<float>& x, int n, int n_iter, int nw, int chunk_size) {

    // Execute the Jacobi method
    int k = 1;
    bool stop = false;

    std::vector<float> xo = x;
    
    // FastFlow's class to implement a map
    ParallelFor pf;

    // Function that has to be executed by the ParallelFor object at each Jacobi iteration
    std::function<void(int)> f = [&](const int i) {
        float val = 0.0;
        for (int j = 0; j < n; j++) {
            if (i != j)
                val = val + a[i*n + j]*xo[j];
        }
        x[i] = (1/a[(n+1)*i])*(b[i]-val);
    };

    std::function<void(void)> parjac_ff = [&](){

        while (k <= n_iter) {
            pf.parallel_for(0, n, 1, chunk_size, f, nw);

            k = k + 1;
            xo = x;
        }

    };

    parjac_ff();
}

int main(int argc, char *argv[]) {

    int seed = std::stoul(argv[1]); //seed to generate random numbers
    int n = std::stoul(argv[2]); //linear system's dimension
    int n_iter = std::stoul(argv[3]); //maximum number of iterations
    int nw = std::stoul(argv[4]); //parallel degree
    int chunk_size = std::stoul(argv[5]); //chunks' size for the ParallelFor

    srand(seed);
    std::vector<float> a(n*n);
    std::vector<float> b(n);
    std::vector<float> x(n, 0);

    float lo = -32.0;
    float hi = 32.0;

    float lo_d = 32.0*((float)(n-1));
    float hi_d = 32.0*((float)(n+1));

    
    // Generate the matrix A for the linear system Ax = b
    for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++){
            if (i == j) {
                a[i*n + j] = lo_d + static_cast<float> (rand() / static_cast<float>(RAND_MAX/(hi_d-lo_d)));
                if (rand() / static_cast<float>(RAND_MAX) < 0.5)
                    a[i*n + j] = -a[i*n + j];
            }
            else {
                a[i*n + j] = lo + static_cast<float> (rand() / static_cast<float>(RAND_MAX/(hi-lo)));
            }
        }
    }

    // Generate the matrix b for the linear system Ax = b
    for (int i = 0; i < n; i++){
        b[i] = lo + static_cast<float> (rand() / static_cast<float>(RAND_MAX/(hi-lo)));;
    }


    
    // Start to measure the elapsed time
    my_timer timer;
    timer.start_timer();
    
    // Compute Jacobi
    jacobi(std::ref(a), std::ref(b), std::ref(x), n, n_iter, nw, chunk_size);
    
    // Measure the elapsed time and print the result.
    time_t elapsed = timer.get_time();
    std::cout << "Elapsed time: " << elapsed << std::endl;

    // optional to check the error
    /*for(int i = 0; i < n; i++){
        float v = 0.0;
        for(int j = 0; j < n; j++){
            v = a[i*n + j]*x[j] + v;
        }
        v = v - b[i];
        std::cout << "Error at row i " << v << std::endl;
    }*/

    return 0;
}
