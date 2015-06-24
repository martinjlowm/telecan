

#ifndef GSM_BURST_COUNTER_H_
#define GSM_BURST_COUNTER_H_

#include <stdint.h>

#include "gsm/constants.h"

namespace GSM {

class BurstCounter {
 public:
  enum burst_type {
    NORMAL_OR_DUMMY,  // These two share channels and dummy is used
                      // whenever normal is not (when the channel is
                      // not occupied).
    FREQUENCY_CORRECTION,
    SYNCHRONIZATION,
    ACCESS
  } burst_type_;

  BurstCounter();
  ~BurstCounter();

  BurstCounter & operator++(int32_t term);

  void Set(uint32_t t1, uint32_t t2, uint32_t t3, uint32_t timeslot_num);

  uint32_t T1();
  uint32_t T2();
  uint32_t T3();

  uint32_t offset_integer();

  uint32_t timeslot_num();

  uint32_t FrameNumber();
  uint32_t FrameNumberMod();

  burst_type GetType();

  uint8_t *get_buffer();
  uint32_t num_stored_bursts();
  void set_num_stored_bursts(uint32_t num_stored_bursts);

 private:
  uint32_t t1_, t2_, t3_, timeslot_num_;
  double offset_fractional_;
  double offset_integer_;

  uint8_t buffer_[kTimeslotsPerFrame][kNormalBlocks * kBurstSize];
  uint32_t num_stored_bursts_[kTimeslotsPerFrame];

  static void InitializeFrameLUT(uint8_t timeslot, const uint8_t *allocation,
                                 uint32_t size, burst_type burst_type);
  static burst_type frame_lut_[51][kTimeslotsPerFrame];
};

}  // namespace GSM

#endif  // GSM_BURST_COUNTER_H_
