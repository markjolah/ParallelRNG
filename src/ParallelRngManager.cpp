/** @file ParallelRngManager.cpp
 * @author Mark J. Olah (mjo\@cs.unm DOT edu)
 * @date 2016-2017
 * @brief Fast auto rng for parallel openmp code
 */

#include <random>
#include "ParallelRngManager/ParallelRngManager.h"

namespace parallel_rng {

SeedT generate_seed()
{
    std::random_device true_rnd;
    return true_rnd();
}

IdxT openmp_estimate_max_threads()
{
    return std::max(omp_get_num_threads(),omp_get_num_procs());
}

} /* namespace parallel_rng */
