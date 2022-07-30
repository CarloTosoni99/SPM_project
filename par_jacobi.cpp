#include <iostream>
#include <cmath>
#include <thread>
#include <functional>
#include <barrier>
#include <vector>

#include "my_timer.cpp"


float compute_magnitude(std::vector<float>& x, std::vector<float>& xo, int n) {

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

    //std::cout << "Current value for convergence " << (num / den) << std::endl;
    return num / den;
}


void jacobi(std::vector<float>& a, std::vector<float>& b, std::vector<float>& x, int n, int n_iter, float tol, int ch_conv, int nw) {

    std::cout << "entering the Jacobi method... " << std::endl;

    int k = 1;
    bool stop = false;

    std::vector<float> xo = x;


    std::barrier bar(nw, [&]() {
        if (ch_conv != 0) {
            stop = compute_magnitude(std::ref(x), std::ref(xo), n) < tol;
        }
        k = k + 1;
        xo = x;
    });

    std::function<void(int)> parjac = [&](int thr_n){

        while (k <= n_iter) {
            //std::cout << "this is thread " << thr_n << " i'm starting iter number " << k << std::endl;
            for (int i = thr_n; i < n; i += nw) {
                float val = 0.0;
                for (int j = 0; j < n; j++) {
                    if (i != j)
                        val = val + a[i*n + j]*xo[j];
                }
                x[i] = (1/a[(n+1)*i])*(b[i]-val);
            }

            bar.arrive_and_wait();
            if (stop)
                return;
        }

        return;
    };

    std::vector<std::thread> tvec(nw);
    for (int i = 0; i < nw; i++) {
        tvec[i] = std::thread(parjac, i);
    }

    for(std::thread &thr : tvec) {
        thr.join();
    }

    std::cout << "Jacobi is done" << std::endl;
    return;
}


int main(int argc, char *argv[]){

    int seed = std::stoul(argv[1]); //seed to generate random numbers
    int n = std::stoul(argv[2]); //linear system's dimension
    int n_iter = std::stoul(argv[3]); //maximum number of iterations
    int ch_conv = std::stoul(argv[4]); //if it's 1 the programm will check the convergence of jacobi at each iteration, if it's 0 it will not
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


    for (int i = 0; i < n; i++){
        b[i] = lo + static_cast<float> (rand() / static_cast<float>(RAND_MAX/(hi-lo)));;
    }

    my_timer timer;
    timer.start_timer();
    jacobi(std::ref(a), std::ref(b), std::ref(x), n, n_iter, tol, ch_conv, nw);
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