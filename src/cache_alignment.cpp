/** @file cache_alignment.cpp
 * @author Mark J. Olah (mjo\@cs.unm DOT edu)
 * @date 2018
 * @brief 
 * 
 * Get the cache alignment.  Working for linux only at the moment.
 * 
 */
#include <fstream>
#include <cstddef>
#include <algorithm>

namespace parallel_rng::cache_alignment {

static const std::size_t DefaultCacheAlignment = 64;
static const std::size_t MinimumCacheAlignment = 16;

size_t estimate_cache_alignment()
{
#if defined(__linux__)
    std::ifstream coherency_line_size;
    coherency_line_size.open("/sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size");
    if(!coherency_line_size) return DefaultCacheAlignment;
    size_t cache_line_size;
    coherency_line_size >> cache_line_size;
    coherency_line_size.close();
    if(cache_line_size < MinimumCacheAlignment) return DefaultCacheAlignment; //Something went wrong;
    return cache_line_size;
#else
    return DefaultCacheAlignment
#endif
}

} /* namespace parallel_rng::cache_alignment */
