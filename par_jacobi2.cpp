#include <iostream>
#include <cmath>
#include <thread>
#include <functional>
#include <barrier>
#include <vector>

#include "my_timer.cpp"


void jacobi(std::vector<float>& a, std::vector<float>& b, std::vector<float>& x, int n, int n_iter, float tol, int ch_conv, int nw) {

    int k = 1;
    int h = 1;
    bool stop = false;

    std::vector<float> xo = x;

    // Verify if Jacobi converged
    std::function<bool(float, float, float)> convergence = [](float num, float den, float tol) {
        num = std::sqrt(num);
        den = std::sqrt(den);

        if ((num / den) < tol)
            return true;
        return false;
    };


    std::vector<float> num_vec(n, 0);
    std::vector<float> den_vec(n, 0);

    // This barrier is required to wait all the threads at the end of each Jacobi iteration
    std::barrier bar(nw, [&]() {return;});
    
    // These two barriers are required to parallelise the computations of the stopping criterion
    std::barrier bar2(nw, [&]() {h = h * 2;});
    std::barrier bar3(nw, [&]() {
        stop = convergence(num_vec[0], den_vec[0], tol);
        k = k + 1;
        xo = x;
        h = 1;
    });
    
    
    // Parallel Jacobi method
    std::function<void(int)> parjac = [&](int thr_n){

        while (k <= n_iter) {
            for (int i = thr_n; i < n; i += nw) {
                float val = 0.0;
                for (int j = 0; j < n; j++) {
                    if (i != j)
                        val = val + a[i*n + j]*xo[j];
                }
                x[i] = (1/a[(n+1)*i])*(b[i]-val);
            }

            // Waiting the other threads...
            bar.arrive_and_wait();

            // Compute the stopping criterion in parallel if ch_conv == 1
            if (ch_conv != 0) {
                for (int i = thr_n; i < n; i += nw) {
                    num_vec[i] = (x[i] - xo[i])*(x[i] - xo[i]);
                    den_vec[i] = (x[i]*x[i]);
                }
                bar.arrive_and_wait();

                while (h < n) {
                    for (int i = thr_n * (h*2); i + h < n; i += nw * (h*2)) {
                        num_vec[i] = num_vec[i] + num_vec[i + h];
                        den_vec[i] = den_vec[i] + den_vec[i + h];
                    }
                    bar2.arrive_and_wait();
                }
            }
            
            bar3.arrive_and_wait();
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
    int ch_conv = std::stoul(argv[4]); //if it's 1 the program will check the convergence of jacobi at each iteration, if it's 0 it will not
    float tol = std::atof(argv[5]); //maximum tolerance for convergence, the program will use this value only if ch_conv ==
    int nw = std::stoul(argv[6]); //parallel degree

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
    jacobi(std::ref(a), std::ref(b), std::ref(x), n, n_iter, tol, ch_conv, nw);
    
    // Measure the elapsed time and print the result.
    time_t elapsed = timer.get_time();
    std::cout << "Elapsed time: " << elapsed << std::endl;


    // optional to check the error
    /*std::cout << "error check enabled" << std::endl;
    for(int i = 0; i < n; i++){
        float v = 0.0;
        for(int j = 0; j < n; j++){
            v = a[i*n + j]*x[j] + v;
        }
        v = v - b[i];
        if ( v > 0.001 )
            std::cout << "Error at row i " << v << std::endl;
    }*/


    return 0;
}
