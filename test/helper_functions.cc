#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "helper_functions.h"
#include "gsm/constants.h"

TEST(PhaseDifference, PhaseDifferenceCalculatesTheRightResult) {
  gr_complex a = gr_complex(2.0, 3.0);
  gr_complex b = gr_complex(1.0, 3.0);
  float c = ComputePhaseDifference(a, b);
  EXPECT_LT(c, 0);
  EXPECT_GT(c, -1);
}

TEST(RotateSCHSequence, AssureRotatedSequenceIsRotatedCorrectly) {
  gr_complex SCH_training_seq[kNumSyncBits];
  gr_complex start_point = gr_complex(0.0, -1.0);

  RotateSequence(kSyncTrainingSequence,
                 kNumSyncBits,
                 SCH_training_seq,
                 start_point);
  const gr_complex expected_rotation[] = {
    gr_complex(0,-1), gr_complex(-1,0), gr_complex(0,1),
    gr_complex(-1,0), gr_complex(0, -1), gr_complex(-1,0),
    gr_complex(0,-1), gr_complex(-1,0), gr_complex(0,1),
    gr_complex(1, 0), gr_complex(0, 1), gr_complex(1, 0),
    gr_complex(0, 1), gr_complex(-1, 0), gr_complex(0, 1),
    gr_complex(1, 0), gr_complex(0, 1),gr_complex(-1, 0),
    gr_complex(0, -1), gr_complex(1, 0), gr_complex(0, 1),
    gr_complex(1, 0), gr_complex(0, -1),  gr_complex(1, 0),
    gr_complex(0, 1), gr_complex(-1, 0), gr_complex(0, -1),
    gr_complex(1, 0), gr_complex(0, -1),  gr_complex(1, 0),
    gr_complex(0, 1), gr_complex(-1, 0), gr_complex(0, 1),
    gr_complex(-1, 0), gr_complex(0, 1),  gr_complex(1, 0),
    gr_complex(0, -1), gr_complex(1, 0), gr_complex(0, -1),
    gr_complex(-1,  0), gr_complex(0,  1),  gr_complex(1, 0),
    gr_complex(0, -1), gr_complex(1, 0), gr_complex(0,  1),
    gr_complex(1, 0), gr_complex(0, -1), gr_complex(-1, 0),
    gr_complex(0, 1), gr_complex(1, 0), gr_complex(0, 1),
    gr_complex(-1, 0), gr_complex(0,  1), gr_complex(1, 0),
    gr_complex(0, 1), gr_complex(1, 0), gr_complex(0, 1),
    gr_complex(-1, 0), gr_complex(0, -1), gr_complex(-1, 0),
    gr_complex(0, -1), gr_complex(-1, 0), gr_complex(0,  1),
    gr_complex(-1, 0)};

  ASSERT_THAT(SCH_training_seq, testing::ElementsAreArray(expected_rotation));
}

TEST(Correlation, CheckToSeeIfCorrelationIsWhatWeExpect) {
  gr_complex SCH_training_seq[kNumSyncBits];
  gr_complex start_point = gr_complex(0.0, -1.0);
  gr_complex correlation;

  RotateSequence(kSyncTrainingSequence,
                 kNumSyncBits,
                 SCH_training_seq,
                 start_point);

  // const gr_complex in_samples[]
  #include "correlation_data.h"

  correlation = CrossCorrelate(&SCH_training_seq[5], in_samples,
                               kNumSyncBits - 10, 0);

  ASSERT_NEAR(correlation.real(), -0.001314, 0.001);
  ASSERT_NEAR(correlation.imag(), -0.002469, 0.001);
}
