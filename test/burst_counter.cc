#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gsm/burst_counter.h"
#include "gsm/constants.h"

class BurstCounterTest : public testing::Test {
 protected:
  GSM::BurstCounter *burst_counter_;

  BurstCounterTest() {
  }

  virtual ~BurstCounterTest() {
  }

  virtual void SetUp() {
    burst_counter_ = new GSM::BurstCounter();
  }

  virtual void TearDown() {
    delete burst_counter_;
  }
};

TEST_F(BurstCounterTest, GetBurstTypeOfTimeslotFollowingTimeslot0) {
  burst_counter_->Set(434, 1, 31, 0);

  (*burst_counter_)++;

  ASSERT_EQ(GSM::BurstCounter::NORMAL_OR_DUMMY, burst_counter_->GetType());
}

TEST_F(BurstCounterTest, GetFrequencyBurstTypeOfTimeslotFollowingSCHTimeslot) {
  burst_counter_->Set(434, 1, 31, 0);

  uint32_t i = 0;
  while (i < 8*9) {
    (*burst_counter_)++;
    i++;
  }

  ASSERT_EQ(GSM::BurstCounter::FREQUENCY_CORRECTION, burst_counter_->GetType());
}
