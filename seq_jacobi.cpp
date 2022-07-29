#include <vector>
#include <iostream>
#include <functional>
#include <cmath>


#include "my_timer.cpp"

//using namespace std::chrono_literals;


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

std::vector<float> seq_jacobi(std::vector<float>& a, std::vector<float>& b, std::vector<float>& x, int n, int n_iter, float tol, int ch_conv) {

    std::cout << "entering the jacobi method... " << std::endl;

    int k = 1;
    std::vector<float> xo = x;
    while (k <= n_iter) {
        for(int i = 0; i < n; i++) {
            x[i] = 0;
            for(int j = 0; j < n; j++) {
                if (i != j)
                    x[i] = x[i] + a[i*n + j]*xo[j];
            }
            x[i] = (1/a[(n+1)*i])*(b[i]-x[i]);
        }
        if (ch_conv != 0)
            if (compute_magnitude(std::ref(x), std::ref(xo), n) < tol) {
                std::cout << "condition for convergence is satisfied" << std::endl;
                return x;
            }
        k++;
        xo = x;
    }

    std::cout << "jacobi is done" << std::endl;
    return x;
}

int main(int argc, char *argv[]) {

    int seed = std::stoul(argv[1]); //seed to generate random numbers
    int n = std::stoul(argv[2]); //linear system's dimension
    int n_iter = std::stoul(argv[3]); //maximum number of iterations
    int ch_conv = std::stoul(argv[4]); //if it's 1 the programm will check the convergence of jacobi at each iteration, if it's 0 it will not
    float tol = std::atof(argv[5]); //maximum tolerance for convergence, the program will use this value only if ch_conv == 1

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
    std::vector<float> sol =  seq_jacobi(std::ref(a), std::ref(b), std::ref(x), n, n_iter, tol, ch_conv);
    time_t elapsed = timer.get_time();
    std::cout << "Elapsed time: " << elapsed << std::endl;

    /* optional to check the error
    for(int i = 0; i < n; i++){
        float val = 0.0;
        for(int j = 0; j < n; j++){
            val = a[i*n + j]*x[j] + val;
        }
        val = val - b[i];
        std::cout << "Error at row i " << val << std::endl;
    }
    return 0;
    */
}
