/** @file AArray.h
 * @author Mark J. Olah (mjo\@cs.unm DOT edu)
 * @date 2018
 * @brief 
 * 
 * A type to hold an array of (largeish) objects so that adjacent elements are
 * never on the same cache line.  This prevents false sharing of elements.
 * 
 */
#ifndef _ALIGNED_ARRAY_AARRAY_H
#define _ALIGNED_ARRAY_AARRAY_H

#include <cstddef>
#include <cstdint>
#include <cmath>
#include <exception>
#include <iterator>
#include <utility>

#include "ParallelRngManager/cache_alignment.h"


#ifdef PARALLEL_RNG_DEBUG
    #include "ParallelRngManager/debug/debug_assert.h"
    #ifndef PARALLEL_RNG_DEBUG_LEVEL
        #define PARALLEL_RNG_DEBUG_LEVEL 1 // macro to control assertion level
    #endif
    namespace aligned_array::assert {
        struct handler : debug_assert::default_handler,
                        debug_assert::set_level<PARALLEL_RNG_DEBUG_LEVEL> 
        { };
    } /*namespace parallel_rng::assert */
    //ASSERT_SETUP can wrap code only necessary for calling assert statements
    #ifndef ASSERT_SETUP
        #define ASSERT_SETUP(x) x
    #endif
#elif
    //Expand DEBUG_ASSERT to empty
    #ifndef DEBUG_ASSERT
        #define DEBUG_ASSERT(...)
    #endif

    //Expand ASSERT_SETUP to empty
    #ifndef ASSERT_SETUP
        #define ASSERT_SETUP(...)
    #endif
#endif


namespace aligned_array
{


template<class T>
class AArray 
{
public:
    template<bool> class Iterator;

    using value_type = T;
    using size_type = size_t;
    using small_size_type = uint32_t; //Type for some elements that will not need 64-bits in any sane scenario
    using difference_type = std::ptrdiff_t;
    
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using reference = value_type&;
    using const_reference = const value_type&;
    using iterator = Iterator<false>;
    using const_iterator = Iterator<true>;
    
    
    AArray(size_type max_size, small_size_type align = parallel_rng::cache_alignment::DefaultCacheAlignment)
        : _max_size{max_size},
          _size{0},
          align_bits{ (small_size_type) std::ceil(std::log2(align)) },
          block_size{ (small_size_type) std::ceil(sizeof(T) / (double) align) << align_bits }
    {
        if( align < alignof(T) || align < alignof(void *) || ((align-1) & align) != 0) 
            throw std::invalid_argument("Align must be power of 2, not less than alignof(T) or alignof(void*)");
        alloc_buf();
    }
    
    AArray(const AArray &o)
        : _max_size{o._max_size},
          _size{o._size},
          align_bits{o.align_bits},
          block_size{o.block_size}
    { alloc_buf(); }
    
    AArray(AArray&& o) noexcept
        : _max_size{o._max_size},
          _size{o._size},
          align_bits{o.align_bits},
          block_size{o.block_size},
          first{o.first}
    { o.first = nullptr; }
    
    AArray& operator=(const AArray&o)
    {
        if(&o == this) return *this; //self assignment guard
        _max_size = o._max_size;
        _size = o._size;
        align_bits = o.align_bits;
        block_size = o.block_size;
        free_buf();
        alloc_buf();
        return *this;
    }
    
    AArray& operator=(AArray&&o) noexcept
    {
        if(&o == this) return *this; //self assignment guard
        _max_size = o._max_size;
        _size = o._size;
        align_bits = o.align_bits;
        block_size = o.block_size;
        free_buf();
        first = o.first;
        return *this;
    }
    
    ~AArray() 
    { 
        clear(); //dealloc elements
        free_buf(); //free managed buffer
    }
    
    //Capacity
    size_type align() const noexcept { return size_type(1) << align_bits; }
    size_type size() const noexcept { return _size; }
    size_type max_size() const noexcept { return _max_size; }
    size_type capacity() const noexcept { return _max_size; }
    bool empty() const noexcept { return !_size; }
    
    void clear() noexcept
    { 
        for(;_size>0; _size--) reinterpret_cast<pointer>(first+(_size-1)*block_size)->~T();
    }
    
    template<class... Ts>
    void fill(const Ts&... vs)
    { for(;_size<_max_size; _size++) new(first+_size*block_size) T{vs...}; }
    
    //Exchanges the contents of the container with those of other. Does not invoke any move, copy, or swap operations on individual elements
    void swap(AArray &o) noexcept
    {
        std::swap(_max_size, o._max_size);
        std::swap(_size, o._size);
        std::swap(align_bits, o.align_bits);
        std::swap(block_size, o.block_size);
        std::swap(first, o.first);
    }
    
    reference operator[](size_type n) { return *reinterpret_cast<pointer>(first+n*block_size); }
    const_reference operator[](size_type n) const { return *reinterpret_cast<const_pointer>(first+n*block_size); }

    reference at(size_type n) 
    { 
        if(n>=_size) throw std::out_of_range("Index of of range."); 
        return *reinterpret_cast<pointer>(first+n*block_size); 
    }
    
    const_reference at(size_type n) const
    { 
        if(n>=_size) throw std::out_of_range("Index of of range."); 
        return *reinterpret_cast<const_pointer>(first+n*block_size); 
    }

    const_reference front() const noexcept { return *reinterpret_cast<const_pointer>(first); }
    reference front() noexcept { return *reinterpret_cast<pointer>(first); }
    const_reference back() const noexcept { return *reinterpret_cast<const_pointer>(first+(_size-1)*block_size); }
    reference back() noexcept { return *reinterpret_cast<pointer>(first+(_size-1)*block_size); }
    
    template<class... Args>
    reference emplace_back(Args&&... args)
    {
        auto ref = new(first + _size++ * block_size) T{std::forward<Args>(args)...};
        return *ref; 
    }
    
    reference push_back(const T& t) { return emplace_back(t); }
    reference push_back(T&& t) { return emplace_back(std::move(t)); }
    
    void pop_back() noexcept { reinterpret_cast<pointer>(first + --_size * block_size)->~T(); }
    
    iterator begin() noexcept { return {*this}; }
    const_iterator cbegin() noexcept { return {*this}; }
    iterator end() noexcept { return {*this,size()}; }
    const_iterator cend() noexcept { return {*this,size()}; }

private:
    size_type _max_size;
    size_type _size;
    small_size_type align_bits;
    small_size_type block_size;
    char *first;

    void alloc_buf()
    {
        if(!_max_size) {
            first = nullptr;
            return;
        }
        size_type buf_size = align() + _max_size*block_size; //Allow at least align extra bytes which is enough to store a pointer
        void *buf = new char[buf_size];
        first = reinterpret_cast<char*>(((reinterpret_cast<size_t>(buf) >> align_bits) + 1) << align_bits); //First aligned position within buf after head
        void **hidden_buf = reinterpret_cast<void**>(first) - 1;
        *hidden_buf = buf; //Store buf pointer in the extra align sized space before first;       
    }
    
    void free_buf() noexcept
    {
        if(!first) return;
        char **hidden_buf = reinterpret_cast<char**>(first) - 1;
        char *buf = *hidden_buf; //Retrieve buf pointer from the extra align sized space before first;
        DEBUG_ASSERT( first-buf <= (difference_type) align() , assert::handler{},"Buf is busted!");
        delete[](buf);
        first = nullptr;
    }
        
    template<bool is_const, class Type, class ConstType> 
    struct type_for_const;
    
    template<class Type, class ConstType> 
    struct type_for_const<true,Type,ConstType> 
    { using type = ConstType; };
    
    template<class Type, class ConstType> 
    struct type_for_const<false,Type,ConstType> 
    { using type = Type; };
    
    
public: 
    template<bool IsConst>
    class Iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = AArray::value_type;
        using difference_type = AArray::difference_type;
        using pointer = typename type_for_const<IsConst,AArray::pointer,AArray::const_pointer>::type;
        using reference = typename type_for_const<IsConst,AArray::reference,AArray::const_reference>::type;
        
        explicit Iterator(const AArray &arr, size_t idx=0) noexcept : arr(&arr), idx(idx) {}
        
        //Functions as copy constructor for non-const_iterator and as converting constructor for const_iterator
        Iterator(const AArray<T>::Iterator<false> &o) noexcept
            : arr(o.arr), idx(o.idx) 
        { }
        
        Iterator& operator++() noexcept//prefix increment
        { idx++; return *this; }
        
        Iterator operator++(int) noexcept//postfix increment
        { 
            Iterator copy = *this; 
            idx++; 
            return copy; 
        }
        
        Iterator& operator--() noexcept//prefix decrement
        { idx--; return *this; }
            
        Iterator operator--(int) noexcept//postfix decrement
        {
            Iterator copy = *this; 
            idx--; 
            return copy;             
        }
        
        reference operator*() const noexcept { return arr->operator[](idx); }
        pointer operator->() const noexcept { return &arr->operator[](idx); }
        bool operator==(const Iterator<IsConst> &o) noexcept { return idx == o.idx; } 
        bool operator!=(const Iterator<IsConst> &o) noexcept { return idx != o.idx; } 
        bool operator<(const Iterator<IsConst> &o) noexcept { return idx < o.idx; } 
        bool operator<=(const Iterator<IsConst> &o) noexcept { return idx <= o.idx; } 
        bool operator>(const Iterator<IsConst> &o) noexcept { return idx > o.idx; } 
        bool operator>=(const Iterator<IsConst> &o) noexcept { return idx >= o.idx; } 
        
        template<bool _Const>
        friend void swap(Iterator<_Const>& lhs, Iterator<_Const>& rhs) noexcept;
        
    private:
        using AArrayPrtT = typename type_for_const<IsConst, AArray*, const AArray*>::type;
        AArrayPrtT arr;
        size_type idx;
    };

};

// template<class T, bool IsConst>
// void swap(typename AArray<T>::template Iterator<IsConst>& lhs, typename AArray<T>::template Iterator<IsConst>& rhs) noexcept
// {
//     std::swap(lhs.arr,rhs.arr);
//     std::swap(lhs.idx,rhs.idx);
// }

    
} /* namespace parallel_rng */

#endif /* _PARALLEL_RNG_CACHEALIGNEDARRAY_H */
