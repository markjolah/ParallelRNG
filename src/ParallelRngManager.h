/** @file ParallelRngManager.h
 * @author Mark J. Olah (mjo\@cs.unm DOT edu)
 * @date 2016-2017
 * @brief Adapts TRNG parallel RNG to armadillo, maintaining a per-thread RNG.
 */

#ifndef _PARALLEL_RNG_PARALLELRNGMANAGER_H
#define _PARALLEL_RNG_PARALLELRNGMANAGER_H

#include <cstdint>
#include <exception>

#include <armadillo>
#include <boost/thread/shared_mutex.hpp>
#include <trng/lcg64_shift.hpp>
#include <trng/yarn5s.hpp>
#include <trng/yarn5.hpp>
#include <trng/yarn3s.hpp>
#include <trng/yarn3.hpp>
#include <boost/variant.hpp>
#include <omp.h>


namespace parallel_rng {
    

using SeedT = uint64_t; /**< Type for 64-bit seeds */
using IdxT = arma::uword; /**< Type for indexes */

/** @brief Suggested default ParallelRNG type
 * 
 *  lcg64_shift is one of the fastest ParallelRNG types with shifting to correct for poor 
 * lower order bit randomness in regular lcg64
 * 
 */
using DefaultParallelRngT = trng::lcg64_shift;

/** @brief Variant type containing one of a list of ParallelRNG implementations.
 * 
 * The list of distributions was chosen for a mix of good RNG quality metrics and speed.
 */
// using ParallelRNGVariantT = boost::variant< trng::lcg64_shift, trng::yarn3, trng::yarn3s, 
//                                             trng::yarn5, trng::yarn5s>;

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
SeedT generate_seed();

/** @brief Use openmp to estimate the maximum number of threads that will be generated
 */
IdxT openmp_estimate_max_threads();


template<typename RngT>
class ParallelRngManager
{
public:
    using NormalDistT = std::normal_distribution<double>;
    using UniformDistT = std::uniform_real_distribution<double>;
    
    
    ParallelRngManager();
    ParallelRngManager(SeedT seed);
    ParallelRngManager(SeedT seed, IdxT max_threads);

    // Move only
    ParallelRngManager(const ParallelRngManager<RngT> &) = delete;
    ParallelRngManager& operator=(const ParallelRngManager<RngT> &) = delete;
    ParallelRngManager(ParallelRngManager<RngT> &&) = default;
    ParallelRngManager& operator=(ParallelRngManager<RngT> &&) = default;
    
    void seed(SeedT seed);
    void reset(SeedT seed, IdxT max_threads);
    SeedT get_init_seed() const;
    SeedT get_num_threads() const;
        
    RngT& generator();
    SeedT operator()();
        
    double randu();
    double randn();
    arma::vec randu(IdxT N);
    arma::vec randn(IdxT N);
    arma::mat randu(IdxT rows, IdxT cols);
    arma::mat randn(IdxT rows, IdxT cols);
    IdxT resample_dist(const arma::vec &weights);
    arma::uvec resample_dist(const arma::vec &weights, IdxT N);
    
protected:
    void split_rngs();
    
    SeedT init_seed;
    IdxT num_threads;
    std::vector<RngT> rngs;
    //std::normal_distribution implementations are not thread safe.  Use per-thread distributions
    std::vector<NormalDistT> norm_dist;
    //Note.  Most implementations of std::uniform_real_distribution should be thread safe.
    //But without a cross-platform guarantee we use per-thread uniform_real_distribution
    std::vector<UniformDistT> uni_dist;
};


/* Factory functions */

template<typename RngT=DefaultParallelRngT>
ParallelRngManager<RngT> make_parallel_rng()
{
    return ParallelRngManager<RngT>();
}

template<typename RngT=DefaultParallelRngT>
ParallelRngManager<RngT> make_parallel_rng(SeedT seed)
{
    return ParallelRngManager<RngT>(seed);
}

/* Template class methods */

template<typename RngT>
ParallelRngManager<RngT>::ParallelRngManager() : 
    ParallelRngManager(generate_seed(), openmp_estimate_max_threads())
{}

template<typename RngT>
ParallelRngManager<RngT>::ParallelRngManager(SeedT seed_) : 
    ParallelRngManager(seed_, openmp_estimate_max_threads())
{}

template<typename RngT>
ParallelRngManager<RngT>::ParallelRngManager(SeedT seed_, IdxT num_threads_) : 
    init_seed(seed_),
    num_threads(num_threads_),
    rngs(num_threads,RngT(seed_)),
    norm_dist(num_threads,NormalDistT()),
    uni_dist(num_threads,UniformDistT())
{
    split_rngs();
}

template<typename RngT>
void ParallelRngManager<RngT>::split_rngs()
{
    for(IdxT i=0;i<num_threads;i++) rngs[i].split(num_threads,i);
}

template<typename RngT>
void ParallelRngManager<RngT>::seed(SeedT seed_)
{
    init_seed = seed_;
    for(IdxT n=0; n<num_threads; n++) {
        rngs[n].seed(init_seed);
        uni_dist[n].reset();
        norm_dist[n].reset();  //Important to reset normal distributions on seeding for repeatability
    }
    split_rngs();
}

template<typename RngT>
void ParallelRngManager<RngT>::reset(SeedT seed_, IdxT num_threads_)
{
    num_threads = num_threads_;
    init_seed = seed_;
    rngs = std::vector<RngT>(num_threads,RngT(init_seed));
    uni_dist = std::vector<UniformDistT>(num_threads,UniformDistT());
    norm_dist = std::vector<NormalDistT>(num_threads,NormalDistT());
    split_rngs();
}

template<typename RngT>
SeedT ParallelRngManager<RngT>::get_init_seed() const 
{
    return init_seed;
}

template<typename RngT>
SeedT ParallelRngManager<RngT>::get_num_threads() const 
{
    return num_threads;
}

template<typename RngT>
RngT& ParallelRngManager<RngT>::generator()
{
    auto id = omp_get_thread_num();
    return rngs[id];
}
  
/**Random 64-bit integer */
template<typename RngT>
SeedT ParallelRngManager<RngT>::operator()() 
{
    return generator()();    
}

/**Random double uniform on [0,1) */
template<typename RngT>
double ParallelRngManager<RngT>::randu()
{
    auto id = omp_get_thread_num();
    return uni_dist[id](rngs[id]);
}

/**Random standard normal variate */
template<typename RngT>
inline
double ParallelRngManager<RngT>::randn()
{
    auto id = omp_get_thread_num();
    return norm_dist[id](rngs[id]);
}

/**Vector of Random double uniform on [0,1) */
template<typename RngT>
arma::vec ParallelRngManager<RngT>::randu(IdxT N)
{
    arma::vec samp(N);
    auto id = omp_get_thread_num();
    auto &gen = rngs[id] ;//get per-thread parallel generator
    auto &uniform = uni_dist[id];  //get per-thread normal distribution
    for(IdxT n=0;n<N;n++) samp(n) = uniform(gen);
    return samp;
}

/**Vector of standard normal variate */
template<typename RngT>
arma::vec ParallelRngManager<RngT>::randn(IdxT N)
{
    arma::vec samp(N);
    auto id = omp_get_thread_num();
    auto &gen = rngs[id]; //get per-thread parallel generator
    auto &normal = norm_dist[id]; //get per-thread normal distribution
    for(IdxT n=0;n<N;n++) samp(n) = normal(gen);
    return samp;
}

/**Matrix of Random double uniform on [0,1) */
template<typename RngT>
arma::mat ParallelRngManager<RngT>::randu(IdxT rows, IdxT cols)
{
    arma::mat samp(rows, cols);
    auto id = omp_get_thread_num();
    auto &gen = rngs[id]; //get per-thread parallel generator
    auto &uniform = uni_dist[id];  //get per-thread normal distribution
    for(IdxT r=0;r<rows;r++) for(IdxT c=0;c<cols;c++) samp(r,c) = uniform(gen);
    return samp;
}

/**Matrix of standard normal variate */
template<typename RngT>
arma::mat ParallelRngManager<RngT>::randn(IdxT rows, IdxT cols)
{
    arma::mat samp(rows, cols);
    auto id = omp_get_thread_num();
    auto &gen = rngs[id]; //get per-thread parallel generator
    auto &normal = norm_dist[id]; //get per-thread normal distribution
    for(IdxT r=0;r<rows;r++) for(IdxT c=0;c<cols;c++) samp(r,c) = normal(gen);
    return samp;
}

template<typename RngT>
arma::uword
ParallelRngManager<RngT>::resample_dist(const arma::vec &weights)
{
    std::discrete_distribution<arma::uword> dist(weights.begin(),weights.end());
    return dist(generator());
}

template<typename RngT>
arma::uvec
ParallelRngManager<RngT>::resample_dist(const arma::vec &weights, arma::uword N)
{
    std::discrete_distribution<arma::uword> dist(weights.begin(),weights.end());
    arma::uvec samp(N);
    auto &gen = generator();
    for(IdxT n=0; n<N; n++) samp(n) = dist(gen);
    return samp;
}

} /* namespace parallel_rng */

#endif /* _PARALLEL_RNG_PARALLELRNGMANAGER_H */
