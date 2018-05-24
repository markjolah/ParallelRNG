/** @file test_parallel_rng.cpp
 * @author Mark J. Olah (mjo\@cs.unm DOT edu)
 * @date 2016-2017
 * @brief Use googletest to test the ParallelRngManager class
 */


#include "ParallelRngManager/ParallelRngManager.h"
#include "gtest/gtest.h"
#include <trng/yarn5s.hpp>
#include <trng/yarn5.hpp>
#include <trng/yarn3.hpp>
#include <trng/yarn3s.hpp>
#include <trng/yarn2.hpp>
namespace {

template<class ParallelRngT=parallel_rng::DefaultParallelRngT>
class ParallelRngManagerTest : public ::testing::Test {
public:
    using ParallelRngManagerT = parallel_rng::ParallelRngManager<ParallelRngT>;
    using SeedT = parallel_rng::SeedT;
    int Nsample = 100;
    SeedT seed = 42;
    ParallelRngManagerT M;
    virtual void SetUp() {
        M = parallel_rng::make_parallel_rng_manager<ParallelRngT>(seed);
    }
};

/* Delcare the model types to test on */
using ParallelRngTypes = ::testing::Types<trng::lcg64_shift, 
                                          trng::yarn5s,
                                          trng::yarn3,trng::yarn3s,
                                          trng::yarn2>;

TYPED_TEST_CASE(ParallelRngManagerTest, ParallelRngTypes);


    
TYPED_TEST( ParallelRngManagerTest, TestGetInitSeed)
{
    EXPECT_EQ(this->seed, this->M.get_init_seed());
    this->M(); //Sample
    EXPECT_EQ(this->seed, this->M.get_init_seed());
}

TYPED_TEST( ParallelRngManagerTest, TestSetInitSeed)
{
    EXPECT_EQ(this->seed, this->M.get_init_seed());
    auto new_seed = this->seed-1;
    this->M.seed(new_seed);
    EXPECT_EQ(new_seed, this->M.get_init_seed());
}

TYPED_TEST( ParallelRngManagerTest, TestSeed)
{
    auto r = this->M();
    this->M.seed(this->seed);
    EXPECT_EQ(this->seed, this->M.get_init_seed()) << "init_seed not reset on call to seed()";
    auto r2 = this->M();
    EXPECT_EQ(r, r2) << "Seeding is not repeatable.";

    this->M.seed(this->seed >> 1);    
    auto r3 = this->M();
    EXPECT_NE(r, r3) << "Reseeding does not change rng.";    
}

TYPED_TEST( ParallelRngManagerTest, TestReset)
{
    auto r = this->M();
    this->M.reset();
    EXPECT_EQ(this->seed, this->M.get_init_seed()) << "init_seed not reset on call to seed()";
    auto r2 = this->M();
    EXPECT_EQ(r, r2) << "Seeding is not repeatable.";

    this->M.reset(this->seed >> 1);    
    auto r3 = this->M();
    EXPECT_NE(r, r3) << "Reseeding does not change rng.";    
}

void check_sample_uniform(const arma::vec &sample)
{
    double old_r;
    for(arma::uword i=0; i < sample.n_elem; i++){
        double r = sample[i];
        //Domain [0,1)
        EXPECT_TRUE(std::isfinite(r));
        EXPECT_LE(0,r);
        EXPECT_GT(1,r);
        if(i>0) { EXPECT_NE(r,old_r) << "Successive samples equal"; }
        old_r = r;
    }    
}

void check_sample_normal(const arma::vec &sample)
{
    double old_r;
    for(arma::uword i=0; i < sample.n_elem; i++){
        double r = sample[i];
        //Domain [0,1)
        EXPECT_TRUE(std::isfinite(r));
        if(i>0) { EXPECT_NE(r,old_r) << "Successive samples equal"; }
        old_r = r;
    }    
}

void check_sample_category(const arma::uvec &sample, const arma::vec &weights)
{
    for(arma::uword i=9; i<weights.n_elem; i++) 
        EXPECT_LE(0,weights[sample[i]]) << "Bin: "<<i<<" -- Weights should be non-negative."; 
    for(arma::uword i=0; i < sample.n_elem; i++){
        //Domain 0 ... Nbins-1
        EXPECT_LE(0,sample[i]);
        EXPECT_GT(weights.n_elem, sample[i]);
        EXPECT_LT(0,weights[sample[i]]) << "Sample: "<<i<<" Bin: "<<sample[i]<<" -- Weight should be non-negative."; 
    }    
}


TYPED_TEST( ParallelRngManagerTest, RandUScalarBounds)
{
    arma::vec sample(this->Nsample);
    for(int i=0; i < this->Nsample; i++)
        sample[i] = this->M.randu();
    check_sample_uniform(sample);
}

TYPED_TEST( ParallelRngManagerTest, RandUVectorBounds)
{
    auto sample = this->M.randu(this->Nsample);
    check_sample_uniform(sample);
}

TYPED_TEST( ParallelRngManagerTest, RandUMatrixBounds)
{
    auto sample = this->M.randu(this->Nsample,this->Nsample);
    for(int j=0; j<this->Nsample; j++)
        check_sample_uniform(sample.col(j));
}

TYPED_TEST( ParallelRngManagerTest, RandNScalarBounds)
{
    arma::vec sample(this->Nsample);
    for(int i=0; i < this->Nsample; i++)
        sample[i] = this->M.randn();
    check_sample_normal(sample);
}

TYPED_TEST( ParallelRngManagerTest, RandNVectorBounds)
{
    auto sample = this->M.randn(this->Nsample);
    check_sample_normal(sample);
}

TYPED_TEST( ParallelRngManagerTest, RandNMatrixBounds)
{
    auto sample = this->M.randn(this->Nsample,this->Nsample);
    for(int j=0; j<this->Nsample; j++)
        check_sample_normal(sample.col(j));
}

TYPED_TEST( ParallelRngManagerTest, ResampleDistScalarTest)
{
    auto weights = this->M.randu(10);
    arma::uvec sample(this->Nsample);
    for(int j=0; j<this->Nsample; j++) sample[j]=this->M.resample_dist(weights);
    check_sample_category(sample,weights);        
}


TYPED_TEST( ParallelRngManagerTest, ResampleDistVectorTest)
{
    auto weights = this->M.randu(10);
    auto sample=this->M.resample_dist(weights, this->Nsample);
    check_sample_category(sample,weights);        
}

}  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
