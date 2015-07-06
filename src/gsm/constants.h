/* -*- c++ -*- */
/*
 * Copyright (c) 2015, Martin Jesper Low Madsen
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     *  Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *
 *     *  Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef GSM_CONSTANTS_H_
#define GSM_CONSTANTS_H_

#include <stdint.h>

const uint32_t kFCCHFailureThreshold = 10;

const float kSampleRate = 2000000.0;
const float kBurstLength = 156.25;
const float kFrameLength = (8 * kBurstLength);
const float k26FrameLength = (26 * kFrameLength);
const float k51FrameLength = (51 * kFrameLength);
const float kGSMBitRate = (1625000.0 / 6.0);

const uint32_t kOSR = 4;

// const float kSampleRate = kGSMBitRate * kOSR;

// GSM Burst structure
const uint32_t kTailBits = 3;
const uint32_t kPayloadBits = 57;
const uint32_t kStealingBit = 1;
const uint32_t kTrainingBits = 26;
const uint32_t kGuardBits = 8;
const float kGuardFractional = 0.25;
const float kGuardPeriod = kGuardFractional + kGuardBits;

const uint32_t kUsefulBits =
    2 * (kPayloadBits + kStealingBit) + kTrainingBits;
const uint32_t kFCCHBits = kUsefulBits;
const uint32_t kBurstSize = kUsefulBits + 2 * kTailBits;

const uint32_t kTimeslotBits = kTailBits + kUsefulBits + kTailBits + kGuardBits;
const uint32_t kTimeslotsPerFrame = 8;
const uint32_t kFrameBits = kTimeslotsPerFrame * kTimeslotBits + 2; // 156.25 * 8

const uint32_t kFCCHHitsNeeded = (kUsefulBits - 4);
const uint32_t kFCCHMaxMisses = 1;

const uint32_t kMaxErrors = 3;

// Training sequences
const uint32_t kSyncPosition = 39;
const uint32_t kSyncSearchRange = 30;

// 5 is offset of the inner training sequence. Change this eventually to mimic SCH.
const uint32_t kTrainPosition = kTailBits + (kPayloadBits + kStealingBit) + 5;
const uint32_t kNumTrainBits = 26;
const uint32_t kNumSyncBits = 64;
const uint32_t kNumTrainSeq = 9;

const uint32_t kSafetyMargin = 6;

const uint8_t kSyncTrainingSequence[] = {
  1, 0, 1, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 0, 1, 0,
  0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
  0, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1,
  0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1
};

const uint8_t kTrainingSequence[kNumTrainSeq][kNumTrainBits] = {
  {0, 0, 1, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0,  // [0]
   0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1},
  {0, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1,  // [1]
   1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1},
  {0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1,  // [2]
   0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0},
  {0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0,  // [3]
   1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0},
  {0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 0, 0,  // [4]
   1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1},
  {0, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0,  // [5]
   0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 0},
  {1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1,  // [6]
   0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1},
  {1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 1, 0,  // [7]
   0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0},
  {0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0,  // [8] Dummy
   0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 0, 1}
};
const uint32_t kDummySequenceNum = 8;

// Channel Impulse Response
const uint32_t kCIRLength = 5;

// Format A is used on DCCHs for frames where there is no information
// field.
// Formats B, Bter and B4 are used on DCCHs for frames containing an
// information field:
//   - format Bter is used on request of higher layers if and only if
//   short L2 header type 1 is supported and a UI command is to be
//   transmitted on SAPI 0;
//   - format B4 is used for UI frames transmitted by the network on
//   SACCH;
//   - format B is applied in all other cases.
// Format Bbis is used only on BCCH, PCH, NCH, and AGCH.

// frames of format A and B:
//   for the SACCH: N201 = 18;
//   for the FACCH and SDCCH: N201 = 20.
// frames of format Bbis:
//   for BCCH, AGCH, NCH and PCH: N201 = 23;
// frames of format Bter
//   for the SACCH: N201 = 21;
//   for the FACCH and SDCCH: N201 = 23;
// frames of format B4
//   for the SACCH: N201 = 19.

// Synchronization
const uint32_t kSyncDataLen = 39;
const uint32_t kSyncDataBlockSize = 25;
const uint32_t kSyncParitySize = 10;
const uint32_t kSyncFillBits = 4;
const uint32_t kSyncParityOutputSize = kSyncDataBlockSize + kSyncParitySize +
    kSyncFillBits;
const uint32_t kSyncConvInputSize = kSyncParityOutputSize;
const uint32_t kSyncConvSize = 2 * kSyncConvInputSize;
const uint32_t kSyncMaxError = kSyncConvSize + 1;

// Normal
const uint32_t kNormalDataBlockSize = 184;
const uint32_t kNormalParitySize = 40;
const uint32_t kNormalFillBitsSize = 4;
const uint32_t kNormalParityOutputSize = kNormalDataBlockSize +
    kNormalParitySize + kNormalFillBitsSize;
const uint32_t kNormalConvInputSize = kNormalParityOutputSize;
const uint32_t kNormalConvSize = 2 * kNormalConvInputSize;
const uint32_t kNormalMaxError = kNormalConvSize + 1;

const uint32_t kNormalBlocks = 4;
const uint32_t kNormalBlockSize = kNormalConvSize / kNormalBlocks;

const uint32_t kPolynomialK = 1 << (5 - 1);

static const uint32_t kNextState[kPolynomialK][2] = {
  {0,  8}, {0,  8}, {1,  9}, {1,  9},
  {2, 10}, {2, 10}, {3, 11}, {3, 11},
  {4, 12}, {4, 12}, {5, 13}, {5, 13},
  {6, 14}, {6, 14}, {7, 15}, {7, 15}
};

// 16x16
static const uint32_t kPrevNextState[kPolynomialK][kPolynomialK] = {
  { 0,  2,  2,  2,  2,  2,  2,  2,  1,  2,  2,  2,  2,  2,  2,  2},
  { 0,  2,  2,  2,  2,  2,  2,  2,  1,  2,  2,  2,  2,  2,  2,  2},
  { 2,  0,  2,  2,  2,  2,  2,  2,  2,  1,  2,  2,  2,  2,  2,  2},
  { 2,  0,  2,  2,  2,  2,  2,  2,  2,  1,  2,  2,  2,  2,  2,  2},
  { 2,  2,  0,  2,  2,  2,  2,  2,  2,  2,  1,  2,  2,  2,  2,  2},
  { 2,  2,  0,  2,  2,  2,  2,  2,  2,  2,  1,  2,  2,  2,  2,  2},
  { 2,  2,  2,  0,  2,  2,  2,  2,  2,  2,  2,  1,  2,  2,  2,  2},
  { 2,  2,  2,  0,  2,  2,  2,  2,  2,  2,  2,  1,  2,  2,  2,  2},
  { 2,  2,  2,  2,  0,  2,  2,  2,  2,  2,  2,  2,  1,  2,  2,  2},
  { 2,  2,  2,  2,  0,  2,  2,  2,  2,  2,  2,  2,  1,  2,  2,  2},
  { 2,  2,  2,  2,  2,  0,  2,  2,  2,  2,  2,  2,  2,  1,  2,  2},
  { 2,  2,  2,  2,  2,  0,  2,  2,  2,  2,  2,  2,  2,  1,  2,  2},
  { 2,  2,  2,  2,  2,  2,  0,  2,  2,  2,  2,  2,  2,  2,  1,  2},
  { 2,  2,  2,  2,  2,  2,  0,  2,  2,  2,  2,  2,  2,  2,  1,  2},
  { 2,  2,  2,  2,  2,  2,  2,  0,  2,  2,  2,  2,  2,  2,  2,  1},
  { 2,  2,  2,  2,  2,  2,  2,  0,  2,  2,  2,  2,  2,  2,  2,  1}
};

static const unsigned int kEncode[kPolynomialK][2] = {
  {0, 3}, {3, 0}, {3, 0}, {0, 3},
  {0, 3}, {3, 0}, {3, 0}, {0, 3},
  {1, 2}, {2, 1}, {2, 1}, {1, 2},
  {1, 2}, {2, 1}, {2, 1}, {1, 2}
};


// Frame allocations -- Timeslot 0
const uint8_t kFCCHFrames[] = {0, 10, 20, 30, 40};
const uint8_t kSCHFrames[] = {1, 11, 21, 31, 41};
const uint8_t kCCCHFrames[] = {
  2, 3, 4, 5, 6, 7, 8, 9, 12, 13, 14, 15, 16, 17, 18, 19, 22, 23, 24, 25, 26,
  27, 28, 29, 32, 33, 34, 35, 36, 37, 38, 39, 42, 43, 44, 45, 46, 47, 48, 49
};

const uint8_t kDCCHFrames[] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
  22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50
};

#endif  // GSM_CONSTANTS_H_
