/** @file test_AArray.cpp
 * @author Mark J. Olah (mjo\@cs.unm DOT edu)
 * @date 2018
 * @brief Use googletest to test the AArray class
 */

#include <cstring>
#include "ParallelRngManager/AArray.h"
#include "gtest/gtest.h"
namespace {

using aligned_array::AArray;

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
    
} /* annonymous namespace */
