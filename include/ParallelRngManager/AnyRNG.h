/** @file AnyRNG.h
 * @author Mark J. Olah (mjo\@cs.unm DOT edu)
 * @date 2018
 * @brief A type-erased random number generator interface that takes the RNG by non-const reference
 * 
 */
#ifndef _ANY_RNG_ANYRNG_H
#define _ANY_RNG_ANYRNG_H

namespace any_rng
{

/**
 * 
 * Stores a reference to rng.  This is by design.  This will become invalid 
 * if this object outlives the RNG it refers too.  RNGs are expected to be global or
 * at least have lifetimes that span the entire lifetime of the code.  Therefore it
 * is normally safe to use this type erased object as if it were.
 * 
 * In and of itself this class does not check for reference validity or threaded useage.
 * This is intended as a single-threaded data structure.
 * 
 */
template<typename T>
class AnyRNG
{
public:
    template<typename RNG>
    explicit AnyRNG(RNG &rng) 
        : min(RNG::min()),
          max(RNG::max()),
          _generate( [&rng](){return rng();} )
    { }
    
    using result_type = T;
    T operator()() { return _generate(); }

    const T min;
    const T max;
private:
    std::function<T()> _generate;
};

} /* namespace any_rng */

#endif /* _ANY_RNG_ANYRNG_H */
