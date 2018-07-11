/** @file test_AArray.cpp
 * @author Mark J. Olah (mjo\@cs.unm DOT edu)
 * @date 2018
 * @brief Use googletest to test the AArray class
 */

#include <cstring>
#include "ParallelRngManager/AlignedArray/AArray.h"
#include "gtest/gtest.h"
namespace {

using namespace aligned_array;

template <typename T>
class AArrayTest : public ::testing::Test {
 public:
     std::vector<size_t> alignments = {8,16,32,64,128,256};
     size_t test_capacity = 10;
};

template<uint32_t N>
class TestSize
{
public:
    TestSize() { std::memset(&bytes,0,N); }
    char* front() { return &bytes;} 
    char* back() { return &bytes[N];} 
    char bytes[N];
};


using SizeTypes = ::testing::Types<char, int, double, TestSize<1>, TestSize<65>, TestSize<258>>;
TYPED_TEST_CASE(AArrayTest, SizeTypes);

    
TYPED_TEST(AArrayTest, create_empty)
{
    for(auto align: this->alignments) {
        AArray<TypeParam> aa(0,align);
        EXPECT_EQ(0, aa.size());
        EXPECT_EQ(0, aa.capacity());
        EXPECT_EQ(align, aa.align());
        EXPECT_TRUE(aa.empty());
        EXPECT_THROW(aa.at(0),std::out_of_range);
    }
}

TYPED_TEST(AArrayTest, emplace_back)
{
    size_t capacity = this->test_capacity;
    for(auto align: this->alignments) {
        AArray<TypeParam> aa(capacity,align);
        EXPECT_EQ(capacity, aa.capacity());
        EXPECT_EQ(0, aa.size()); //Initially empty
        EXPECT_TRUE(aa.empty());
        for(size_t n=0; n<capacity; n++){
            auto &last = aa.emplace_back();
            EXPECT_EQ(&last,&aa.back());
            EXPECT_EQ(n+1,aa.size());
        }
        EXPECT_EQ(aa.size(),aa.capacity());
        EXPECT_FALSE(aa.empty());
    }    
}

TYPED_TEST(AArrayTest, fill)
{
    size_t capacity = this->test_capacity;
    for(auto align: this->alignments) {
        AArray<TypeParam> aa(capacity,align);
        aa.fill();
        EXPECT_EQ(aa.max_size(),aa.size());
        EXPECT_EQ(aa.capacity(),aa.size());
        aa.clear();
        EXPECT_EQ(0,aa.size());
        EXPECT_NE(aa.max_size(),aa.size());
        EXPECT_EQ(aa.capacity(),aa.max_size());
    }
}

TYPED_TEST(AArrayTest, front_back_accessors)
{
    size_t capacity = this->test_capacity;
    for(auto align: this->alignments) {
        AArray<TypeParam> aa(capacity,align);
        aa.fill();
        EXPECT_EQ(&aa.front(),&aa[0]);
        EXPECT_EQ(&aa.back(),&aa[capacity-1]);
        EXPECT_LT(&aa.front(),&aa[1]);
        EXPECT_GT(&aa.back(),&aa[1]);
    }
}

TYPED_TEST(AArrayTest, pop_back)
{
    size_t capacity = this->test_capacity;
    for(auto align: this->alignments) {
        AArray<TypeParam> aa(capacity,align);
        aa.fill();
        for(size_t n=0; n<capacity; n++) aa.pop_back();
        EXPECT_EQ(0,aa.size());
    }
}
    
TYPED_TEST(AArrayTest, copy_construct)
{
    size_t capacity = this->test_capacity;
    for(auto align: this->alignments) {
        AArray<TypeParam> aa(capacity,align);
        aa.fill();
        AArray<TypeParam> bb(aa);
        EXPECT_EQ(aa.size(),bb.size());
        EXPECT_EQ(aa.align(),bb.align());
        EXPECT_EQ(aa.capacity(),bb.capacity());
        EXPECT_NE(&aa.front(),&bb.front());
        
        bb.clear();
        EXPECT_EQ(aa.size(),aa.max_size());
        EXPECT_EQ(bb.size(),0);
        aa.clear();
        bb.fill();
        EXPECT_EQ(aa.size(),0);
        EXPECT_EQ(bb.size(),bb.max_size());
    }
}
 
 
TYPED_TEST(AArrayTest, copy_assignment)
{
    size_t capacity = this->test_capacity;
    for(auto align: this->alignments) {
        AArray<TypeParam> aa(capacity,align);
        aa.fill();
        
        //Do something with bb
        AArray<TypeParam> bb(capacity/2,align);
        bb.fill();
        EXPECT_EQ(bb.size(),capacity/2);
        
        bb = aa; //copy assign
        EXPECT_EQ(aa.size(),bb.size());
        EXPECT_EQ(aa.align(),bb.align());
        EXPECT_EQ(aa.capacity(),bb.capacity());
        EXPECT_NE(&aa.front(),&bb.front());
        
        bb.clear();
        EXPECT_EQ(aa.size(),aa.max_size());
        EXPECT_EQ(bb.size(),0);
        aa.clear();
        bb.fill();
        EXPECT_EQ(aa.size(),0);
        EXPECT_EQ(bb.size(),bb.max_size());
    }
}

 
TYPED_TEST(AArrayTest, move_assignment)
{
    size_t capacity = this->test_capacity;
    for(auto align: this->alignments) {
        AArray<TypeParam> aa(capacity,align);
        aa.fill();
        
        //Do something with bb
        AArray<TypeParam> bb(capacity/2,align);
        bb.fill();
        EXPECT_EQ(bb.size(),capacity/2);
        
        auto *front = &aa.front();
        bb = std::move(aa); //copy assign
        EXPECT_EQ(capacity,bb.size());
        EXPECT_EQ(align,bb.align());
        EXPECT_EQ(capacity,bb.capacity());
        EXPECT_EQ(front,&bb.front());        
        bb.clear();
        EXPECT_EQ(bb.size(),0);
    }
}

TYPED_TEST(AArrayTest, move_construction)
{
    size_t capacity = this->test_capacity;
    for(auto align: this->alignments) {
        AArray<TypeParam> aa(capacity,align);
        aa.fill();
        auto *front = &aa.front();
        //Move construct bb
        AArray<TypeParam> bb(std::move(aa));
        EXPECT_EQ(capacity,bb.size());
        EXPECT_EQ(align,bb.align());
        EXPECT_EQ(capacity,bb.capacity());
        EXPECT_EQ(front,&bb.front());        
        bb.clear();
        EXPECT_EQ(bb.size(),0);
    }
}

TYPED_TEST(AArrayTest, bulk_construction)
{
    size_t capacity = this->test_capacity;
    for(auto align: this->alignments) {
        AArray<TypeParam> aa(capacity,align,TypeParam{});
        EXPECT_FALSE(aa.empty());
        EXPECT_EQ(aa.size(),capacity);
        EXPECT_EQ(aa.max_size(),capacity);
        EXPECT_NO_THROW(aa.at(capacity-1));
        EXPECT_EQ(&aa.back(),&aa[capacity-1]);
    }
}


TYPED_TEST(AArrayTest, element_alignments)
{
    size_t capacity = this->test_capacity;
    for(auto align: this->alignments) {
        AArray<TypeParam> aa(capacity,align,TypeParam{});
        for(size_t n=0;n<capacity;n++) {
            EXPECT_GE(&aa[n], &aa.front());
            EXPECT_LE(&aa[n], &aa.back());
            EXPECT_EQ(0, uintptr_t(&aa[n]) % aa.align());
            if(n>0) {
                uintptr_t delta = uintptr_t(&aa[n]) - uintptr_t(&aa[n-1]);
                EXPECT_EQ(0, delta % aa.align());
                EXPECT_LE(sizeof(TypeParam), delta);
            }
        }
    }
}

TYPED_TEST(AArrayTest, swap)
{
    size_t capacity = this->test_capacity;
    for(auto align: this->alignments) {
        AArray<TypeParam> aa(capacity,align, TypeParam{});
        AArray<TypeParam> bb(2*capacity,2*align, TypeParam{});
        aa.pop_back();
        bb.pop_back();
        
        //Before swap
        EXPECT_EQ(aa.size(),capacity-1);
        EXPECT_EQ(aa.max_size(),capacity);
        EXPECT_EQ(aa.align(),align);
        EXPECT_EQ(bb.size(),2*capacity-1);
        EXPECT_EQ(bb.max_size(),2*capacity);        
        EXPECT_EQ(bb.align(),2*align);
        auto *aa_front = &aa.front();
        auto *aa_back = &aa.back();
        auto *bb_front = &bb.front();
        auto *bb_back = &bb.back();
        
        aa.swap(bb);
        
        //Before swap
        EXPECT_EQ(bb.size(),capacity-1);
        EXPECT_EQ(bb.max_size(),capacity);
        EXPECT_EQ(bb.align(),align);
        EXPECT_EQ(aa.size(),2*capacity-1);
        EXPECT_EQ(aa.max_size(),2*capacity);        
        EXPECT_EQ(aa.align(),2*align);
        EXPECT_EQ(&aa.front(), bb_front);
        EXPECT_EQ(&aa.back(), bb_back);
        EXPECT_EQ(&bb.front(), aa_front);
        EXPECT_EQ(&bb.back(), aa_back);
    }
}

TYPED_TEST(AArrayTest, test_iterator_increment_decrement)
{
    size_t capacity = this->test_capacity;
    for(auto align: this->alignments) {
        AArray<TypeParam> aa(capacity,align, TypeParam{});
        auto b=aa.begin();
        auto b2=aa.begin();
        EXPECT_EQ(b,b2);
        ++b;
        b2++;
        EXPECT_EQ(b,b2);
        b--;
        --b2;
        EXPECT_EQ(b,b2);
        EXPECT_EQ(b,aa.begin());
    }
}

TYPED_TEST(AArrayTest, test_const_iterator_increment_decrement)
{
    size_t capacity = this->test_capacity;
    for(auto align: this->alignments) {
        AArray<TypeParam> aa(capacity,align, TypeParam{});
        auto b=aa.cbegin();
        auto b2=aa.cbegin();
        EXPECT_EQ(b,b2);
        ++b;
        b2++;
        EXPECT_EQ(b,b2);
        b--;
        --b2;
        EXPECT_EQ(b,b2);
        EXPECT_EQ(b,aa.cbegin());
    }
}


TYPED_TEST(AArrayTest, test_iterator_begin_end_ordering)
{
    size_t capacity = this->test_capacity;
    for(auto align: this->alignments) {
        AArray<TypeParam> b(capacity,align);
        EXPECT_EQ(b.begin(),b.cbegin());
        EXPECT_EQ(b.end(),b.cend());
        EXPECT_LE(b.begin(),b.end());
        EXPECT_LE(b.cbegin(),b.cend());
        EXPECT_LE(b.cbegin(),b.end());
        EXPECT_LE(b.begin(),b.cend());
        b.fill();
        EXPECT_EQ(b.begin(),b.cbegin());
        EXPECT_EQ(b.end(),b.cend());
        EXPECT_GE(b.end(),b.begin());
        EXPECT_GE(b.cend(),b.cbegin());
        EXPECT_GE(b.end(),b.cbegin());
        EXPECT_GE(b.cend(),b.begin());
    }
}


TYPED_TEST(AArrayTest, test_iterator_arithmetic)
{
    size_t capacity = this->test_capacity;
    for(auto align: this->alignments) {
        AArray<TypeParam> aa(capacity,align,TypeParam{});
        auto it = aa.begin();
        auto e = aa.end();
        for(size_t n=0;n<capacity;n++) {
            EXPECT_EQ(&aa[n], &it[n]);
            EXPECT_EQ(&aa[n], &*(it+n));
//             EXPECT_EQ(&aa[n], &*(operator+<typename AArray<TypeParam>::iterator>(n,it)));
            auto it2 = it; //copy construct
            it2+=n;
            EXPECT_EQ(&aa[n], &*it2);
            EXPECT_EQ(&aa[n], &*(e-(capacity-n)) );
            auto e2 = e; //copy construct
            e2-=(capacity-n);
            EXPECT_EQ(&aa[n], &*e2);            
        }
    }
}
    
        
        

} /* annonymous namespace */
