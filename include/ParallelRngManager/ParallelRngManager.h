/** @file ParallelRngManager.h
 * @author Mark J. Olah (mjo\@cs.unm DOT edu)
 * @date 2016-2017
 * @brief Adapts TRNG parallel RNG to armadillo, maintaining a per-thread RNG.
 */

#ifndef _PARALLEL_RNG_PARALLELRNGMANAGER_H
#define _PARALLEL_RNG_PARALLELRNGMANAGER_H

#include <cstdint>
#include <exception>
#include <omp.h>

#include <armadillo>
#include <trng/lcg64_shift.hpp>

#include "ParallelRngManager/cache_alignment.h"
#include "ParallelRngManager/AnyRng/AnyRng.h"
#include "ParallelRngManager/AlignedArray/AArray.h"


#ifdef PARALLEL_RNG_DEBUG
    #include "ParallelRngManager/debug/debug_assert.h"
    #ifndef PARALLEL_RNG_DEBUG_LEVEL
        #define PARALLEL_RNG_DEBUG_LEVEL 1 // macro to control assertion level
    #endif
    namespace parallel_rng { namespace assert {
        struct handler : debug_assert::default_handler,
                        debug_assert::set_level<PARALLEL_RNG_DEBUG_LEVEL> 
        { };
    } } /*namespace parallel_rng::assert */
    //ASSERT_SETUP can wrap code only necessary for calling assert statements
    #ifndef ASSERT_SETUP
        #define ASSERT_SETUP(x) x
    #endif
#else
    //Expand DEBUG_ASSERT to empty
    #ifndef DEBUG_ASSERT
        #define DEBUG_ASSERT(...)
    #endif

    //Expand ASSERT_SETUP to empty
    #ifndef ASSERT_SETUP
        #define ASSERT_SETUP(...)
    #endif
#endif

namespace parallel_rng {
  

/** @brief Suggested default ParallelRNG type
 * 
 *  lcg64_shift is one of the fastest ParallelRNG types with shifting to correct for poor 
 * lower order bit randomness in regular lcg64
 * 
 */
using DefaultParallelRngT = trng::lcg64_shift;

class ParallelRngManagerError : public std::exception
{
protected:
    std::string _what;
public:
    ParallelRngManagerError(std::string what) : _what(std::string("ParallelRngManager:")+what) {}
    const char* what() const noexcept override  { return _what.c_str(); }
};

/** @brief Use the true random interface to generate a truly random seed
 */
using SeedT = uint64_t;
using IdxT = arma::uword;
SeedT generate_seed();

/** @brief Use openmp to estimate the maximum number of threads that will be generated
 */
IdxT openmp_estimate_max_threads();

template<class RngT=DefaultParallelRngT, class FloatT=double>
class ParallelRngManager
{
public:
    using VecT = arma::Col<FloatT>;
    using MatT = arma::Mat<FloatT>;
    using NormalDistT = std::normal_distribution<FloatT>;
    using UniformDistT = std::uniform_real_distribution<FloatT>;
    
    ParallelRngManager();
    ParallelRngManager(SeedT seed);
    ParallelRngManager(SeedT seed, IdxT max_threads);

    //Allow copying although it will be expensive
    //This can be useful. e.g., testing.
//     ParallelRngManager(const ParallelRngManager<RngT,FloatT> &) = default;
//     ParallelRngManager& operator=(const ParallelRngManager<RngT,FloatT> &) = default;
//     
//     ParallelRngManager(ParallelRngManager<RngT,FloatT> &&) = default;
//     ParallelRngManager& operator=(ParallelRngManager<RngT,FloatT> &&) = default;
    
    void seed(SeedT seed);
    void reset();
    void reset(SeedT seed);
    void reset(SeedT seed, IdxT max_threads);
    SeedT get_init_seed() const;
    SeedT get_num_threads() const;
        
    RngT& generator();
    any_rng::AnyRng<FloatT> generic_generator(); // Make type-erased gernerator-like object with a reference.
    SeedT operator()();
        
    FloatT randu();
    FloatT randn();
    VecT randu(IdxT N);
    VecT randn(IdxT N);
    MatT randu(IdxT rows, IdxT cols);
    MatT randn(IdxT rows, IdxT cols);

    template<class Weights=VecT,class IdxT=IdxT>
    IdxT resample_dist(const Weights &weights);
    
    template<class Weights=VecT,class IdxT=IdxT>
    arma::Col<IdxT> resample_dist(const Weights &weights, IdxT N);
    
protected:
    void split_rngs();
    
    SeedT init_seed;
    IdxT num_threads;
    aligned_array::AArray<RngT> rngs;
    //std::normal_distribution implementations are not thread safe.  Use per-thread distributions
    aligned_array::AArray<NormalDistT> norm_dist;
    //Note.  Most implementations of std::uniform_real_distribution should be thread safe.
    //But without a cross-platform guarantee we use per-thread uniform_real_distribution
    aligned_array::AArray<UniformDistT> uni_dist;
};

/* Factory functions */

template<class RngT=DefaultParallelRngT, class FloatT=double>
ParallelRngManager<RngT,FloatT> make_parallel_rng_manager()
{
    return ParallelRngManager<RngT,FloatT>();
}

template<class RngT=DefaultParallelRngT, class FloatT=double>
ParallelRngManager<RngT,FloatT> make_parallel_rng_manager(SeedT seed)
{
    return ParallelRngManager<RngT,FloatT>(seed);
}

/* Template class methods */

template<class RngT, class FloatT>
ParallelRngManager<RngT,FloatT>::ParallelRngManager() : 
    ParallelRngManager(generate_seed(), openmp_estimate_max_threads())
{}

template<class RngT, class FloatT>
ParallelRngManager<RngT,FloatT>::ParallelRngManager(SeedT seed_) : 
    ParallelRngManager(seed_, openmp_estimate_max_threads())
{}

template<class RngT, class FloatT>
ParallelRngManager<RngT,FloatT>::ParallelRngManager(SeedT seed_, IdxT num_threads_) : 
    init_seed(seed_),
    num_threads(num_threads_),
    rngs{num_threads,cache_alignment::CacheAlignment, RngT{seed_}},
    norm_dist{num_threads,cache_alignment::CacheAlignment, NormalDistT{}},
    uni_dist{num_threads,cache_alignment::CacheAlignment, UniformDistT{}}
{
    split_rngs();
}

template<class RngT, class FloatT>
void ParallelRngManager<RngT,FloatT>::split_rngs()
{
    for(IdxT n=0; n<rngs.size(); n++) rngs[n].split(num_threads,n);
}

template<class RngT, class FloatT>
void ParallelRngManager<RngT,FloatT>::seed(SeedT seed_)
{
    //trng does not easily allow reseeding of split parallel_rng streams.  The
    // .split() function in general changes internal state and repeated calls are invalid.
    // .split() also modifies the seed values.  Therefore the only what to re-seed a parallel_rng
    // is to completely reconstruct with the desired seed, and then call .split()
    reset(seed_, num_threads);
}

template<class RngT, class FloatT>
void ParallelRngManager<RngT,FloatT>::reset()
{
    reset(init_seed, num_threads);
}

template<class RngT, class FloatT>
void ParallelRngManager<RngT,FloatT>::reset(SeedT seed_)
{
    reset(seed_, num_threads);
}

template<class RngT, class FloatT>
void ParallelRngManager<RngT,FloatT>::reset(SeedT seed_, IdxT num_threads_)
{
    num_threads = num_threads_;
    rngs.clear();
    uni_dist.clear();
    norm_dist.clear();
    rngs.fill(seed_);
    uni_dist.fill();
    norm_dist.fill();
    split_rngs();
    init_seed = seed_;
}

template<class RngT, class FloatT>
SeedT ParallelRngManager<RngT,FloatT>::get_init_seed() const 
{
    return init_seed;
}

template<class RngT, class FloatT>
SeedT ParallelRngManager<RngT,FloatT>::get_num_threads() const 
{
    return num_threads;
}

template<class RngT, class FloatT>
RngT& ParallelRngManager<RngT,FloatT>::generator()
{
    auto id = omp_get_thread_num();
    return rngs[id];
}

template<class RngT, class FloatT>
any_rng::AnyRng<FloatT> ParallelRngManager<RngT,FloatT>::generic_generator()
{
    auto id = omp_get_thread_num();
    return {rngs[id]};
}

/**Random 64-bit integer */
template<class RngT, class FloatT>
SeedT ParallelRngManager<RngT,FloatT>::operator()() 
{
    return generator()();    
}

/**Random FloatT uniform on [0,1) */
template<class RngT, class FloatT>
FloatT ParallelRngManager<RngT,FloatT>::randu()
{
    auto id = omp_get_thread_num();
    return uni_dist[id](rngs[id]);
}

/**Random standard normal variate */
template<class RngT, class FloatT>
inline
FloatT ParallelRngManager<RngT,FloatT>::randn()
{
    auto id = omp_get_thread_num();
    return norm_dist[id](rngs[id]);
}

/**Vector of Random FloatT uniform on [0,1) */
template<class RngT, class FloatT>
typename ParallelRngManager<RngT,FloatT>::VecT 
ParallelRngManager<RngT,FloatT>::randu(IdxT N)
{
    VecT samp(N);
    auto id = omp_get_thread_num();
    auto &gen = rngs[id] ;//get per-thread parallel generator
    auto &uniform = uni_dist[id];  //get per-thread normal distribution
    for(IdxT n=0;n<N;n++) samp(n) = uniform(gen);
    return samp;
}

/**Vector of standard normal variate */
template<class RngT, class FloatT>
typename ParallelRngManager<RngT,FloatT>::VecT 
ParallelRngManager<RngT,FloatT>::randn(IdxT N)
{
    VecT samp(N);
    auto id = omp_get_thread_num();
    auto &gen = rngs[id]; //get per-thread parallel generator
    auto &normal = norm_dist[id]; //get per-thread normal distribution
    for(IdxT n=0;n<N;n++) samp(n) = normal(gen);
    return samp;
}

/**Matrix of Random FloatT uniform on [0,1) */
template<class RngT, class FloatT>
typename ParallelRngManager<RngT,FloatT>::MatT 
ParallelRngManager<RngT,FloatT>::randu(IdxT rows, IdxT cols)
{
    MatT samp(rows, cols);
    auto id = omp_get_thread_num();
    auto &gen = rngs[id]; //get per-thread parallel generator
    auto &uniform = uni_dist[id];  //get per-thread normal distribution
    for(IdxT r=0;r<rows;r++) for(IdxT c=0;c<cols;c++) samp(r,c) = uniform(gen);
    return samp;
}

/**Matrix of standard normal variate */
template<class RngT, class FloatT>
typename ParallelRngManager<RngT,FloatT>::MatT 
ParallelRngManager<RngT,FloatT>::randn(IdxT rows, IdxT cols)
{
    MatT samp(rows, cols);
    auto id = omp_get_thread_num();
    auto &gen = rngs[id]; //get per-thread parallel generator
    auto &normal = norm_dist[id]; //get per-thread normal distribution
    for(IdxT r=0;r<rows;r++) for(IdxT c=0;c<cols;c++) samp(r,c) = normal(gen);
    return samp;
}

template<class RngT, class FloatT>
template<class Weights,class IdxT>
IdxT 
ParallelRngManager<RngT,FloatT>::resample_dist(const Weights &weights)
{
    std::discrete_distribution<IdxT> dist(weights.begin(),weights.end());
    return dist(generator());
}

template<class RngT, class FloatT>
template<class Weights,class IdxT>
arma::Col<IdxT>
ParallelRngManager<RngT,FloatT>::resample_dist(const Weights &weights, IdxT N)
{
    std::discrete_distribution<IdxT> dist(weights.begin(),weights.end());
    arma::Col<IdxT> samp(N);
    auto &gen = generator();
    for(IdxT n=0; n<N; n++) samp(n) = dist(gen);
    return samp;
}

} /* namespace parallel_rng */

#endif /* _PARALLEL_RNG_PARALLELRNGMANAGER_H */
