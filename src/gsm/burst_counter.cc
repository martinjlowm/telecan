#include "gsm/burst_counter.h"

#include <math.h>

#include "gsm/constants.h"

GSM::BurstCounter::burst_type
GSM::BurstCounter::frame_lut_[51][kTimeslotsPerFrame] = {{}};

void GSM::BurstCounter::InitializeFrameLUT(uint8_t timeslot,
                                           const uint8_t *allocation,
                                           uint32_t size,
                                           burst_type burst_type) {
  uint32_t i;
  for (i = 0; i < size; i++) {
    frame_lut_[allocation[i]][timeslot] = burst_type;
  }
}

GSM::BurstCounter::BurstCounter() {
  t1_ = 0;
  t2_ = 0;
  t3_ = 0;
  timeslot_num_ = 0;
  offset_fractional_ = 0.0;
  offset_integer_ = 0.0;

  // Assuming just timeslot 0 is used for common and broadcast, 51
  // frame and the remaining timeslots are dedicated and use 26 frame.
  InitializeFrameLUT(0, kFCCHFrames, sizeof(kFCCHFrames), FREQUENCY_CORRECTION);
  InitializeFrameLUT(0, kSCHFrames, sizeof(kSCHFrames), SYNCHRONIZATION);
  InitializeFrameLUT(0, kCCCHFrames, sizeof(kCCCHFrames), NORMAL_OR_DUMMY);

  uint32_t i;
  for (i = 1; i < kTimeslotsPerFrame; i++) {
    InitializeFrameLUT(i, kDCCHFrames, sizeof(kDCCHFrames), NORMAL_OR_DUMMY);
  }

  for (i = 1; i < kTimeslotsPerFrame; i++) {
    num_stored_bursts_[i] = 0;
  }
}

GSM::BurstCounter::~BurstCounter() {}

GSM::BurstCounter & GSM::BurstCounter::operator++(int32_t /* term */) {
  timeslot_num_++;

  if (timeslot_num_ == kTimeslotsPerFrame) {
    timeslot_num_ = 0;

    if ((t2_ == 25) && (t3_ == 50)) {
      t1_ = (t1_ + 1) % (1 << 11);
    }

    t2_ = (t2_ + 1) % 26;
    t3_ = (t3_ + 1) % 51;
  }

  // Update offset - this is integer for OSR_ which is multiple of four
  offset_fractional_ += kGuardFractional * kOSR;
  offset_integer_ = floor(offset_fractional_);
  offset_fractional_ = offset_fractional_ - offset_integer_;

  return *this;
}

void GSM::BurstCounter::Set(uint32_t t1, uint32_t t2, uint32_t t3,
                            uint32_t timeslot_num) {
  t1_ = t1;
  t2_ = t2;
  t3_ = t3;

  timeslot_num_ = timeslot_num;

  double first_sample_position = (FrameNumber() * 8 + timeslot_num_) * kTimeslotBits;
  offset_fractional_ = first_sample_position - floor(first_sample_position);
  offset_integer_ = 0;
}

uint32_t GSM::BurstCounter::T1() {
  return t1_;
}
uint32_t GSM::BurstCounter::T2() {
  return t2_;
}
uint32_t GSM::BurstCounter::T3() {
  return t3_;
}

uint32_t GSM::BurstCounter::offset_integer() {
  return (uint32_t) offset_integer_;
}

uint32_t GSM::BurstCounter::timeslot_num() {
  return timeslot_num_;
}

uint32_t GSM::BurstCounter::FrameNumber() {
  return (51 * 26 * t1_) + (51 * (((t3_ + 26) - t2_) % 26)) + t3_;
}

uint32_t GSM::BurstCounter::FrameNumberMod() {
  return (t1_ << 11) + (t3_ << 5) + t2_;
}

GSM::BurstCounter::burst_type GSM::BurstCounter::GetType() {
  uint32_t frame_num;
  uint32_t timeslot = timeslot_num();

  if (timeslot > 0) {
    frame_num = T2();
  } else {
    frame_num = T3();
  }

  return frame_lut_[frame_num][timeslot];
}

uint8_t *GSM::BurstCounter::get_buffer() {
  return buffer_[timeslot_num_];
}

uint32_t GSM::BurstCounter::num_stored_bursts() {
  return num_stored_bursts_[timeslot_num_];
}
void GSM::BurstCounter::set_num_stored_bursts(uint32_t num_stored_bursts) {
  num_stored_bursts_[timeslot_num_] = num_stored_bursts;
}
