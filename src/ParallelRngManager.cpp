/** @file ParallelRngManager.cpp
 * @author Mark J. Olah (mjo\@cs.unm DOT edu)
 * @date 2016-2017
 * @brief Fast auto rng for parallel openmp code
 */

#include <random>
#include <thread>
#include "omp.h"
#include "ParallelRngManager/ParallelRngManager.h"

namespace parallel_rng {

SeedT generate_seed()
{
    std::random_device true_rnd;
    return true_rnd();
}

IdxT openmp_estimate_max_threads()
{
    IdxT num_threads = omp_get_num_threads();
    IdxT num_procs = omp_get_num_procs();
    IdxT hw_conc = std::thread::hardware_concurrency();
    return std::max(num_threads,std::max(num_procs, hw_conc));
}

} /* namespace parallel_rng */
