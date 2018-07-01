/** @file cache_alignment.h
 * @author Mark J. Olah (mjo\@cs.unm DOT edu)
 * @date 2018
 * @brief 
 * 
 * Get the cache alignment.  Working for linux only at the moment.
 * 
 */

#ifndef _PARALLEL_RNG_CACHE_ALIGNMENT_H
#define _PARALLEL_RNG_CACHE_ALIGNMENT_H

namespace parallel_rng::cache_alignment {
    
extern const std::size_t DefaultCacheAlignment;
extern const std::size_t MinimumCacheAlignment;

size_t estimate_cache_alignment();

} /* namespace parallel_rng::cache_alignment */

#endif /* _PARALLEL_RNG_CACHE_ALIGNMENT_H */
