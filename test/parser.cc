#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "helper_functions.h"
#include "gsm/constants.h"
#include "gsm/parser.h"

class ParserTest : public testing::Test {
 protected:
  GSM::Parser *parser_;

  ParserTest() {
  }

  virtual ~ParserTest() {
  }

  virtual void SetUp() {
    parser_ = new GSM::Parser();
  }

  virtual void TearDown() {
    delete parser_;
  }
};

TEST_F(ParserTest, AttemptToParseSynchronizationBurst) {
  // SCH does not follow the basic format, thus N201 def. is ignored.
  const uint8_t input[] = {
    // 25 bits
    1, 0, 0, 1, 0, 1, 0, 0,
    0, 1, 0, 1, 1, 0, 0, 1,
    0, 1, 1, 0, 0, 0, 0, 0,
    0,
    // 10 parity bits
    1, 0, 1, 0, 0, 0, 1, 1, 0, 1,
    // 4 fill
    0, 0, 0, 0};

  parser_->Parse(&input[0], 25, "Synchronization");

  ASSERT_EQ("Synchronization:\n"
            "  BSIC: (1/2)\n"
            "  T1: 820\n"
            "  T2: 1\n"
            "  T3': 4\n",
            parser_->output_.str());
}

// TEST_F(ParserTest, CorrectlyIncrementsBurstType) {
//   const uint8_t input[] = {
//     // 25 bits
//     1, 0, 0, 1, 0, 1, 0, 0,
//     0, 1, 0, 1, 1, 0, 0, 1,
//     0, 1, 1, 0, 0, 0, 0, 0,
//     0,
//     // 10 parity bits
//     1, 0, 1, 0, 0, 0, 1, 1, 0, 1,
//     // 4 fill
//     0, 0, 0, 0};

//   parser_->Parse(&input[0], 25, "Synchronization");
//   parser_->Parse(&input[0], 25, "Synchronization");

//   ASSERT_EQ(2, parser_->messages_count_["SCH"]);
// }

TEST_F(ParserTest, ParseGroupCallControlEmptyBurst) {
  // http://www.etsi.org/deliver/etsi_ts/100900_100999/100938/08.04.00_60/ts_100938v080400p.pdf
  // 04.06 ETSI page 12
  // Bbis
  const uint8_t input[] = {  // MSB is to the right
    1, 0, 0, 0, 0, 0, 0, 0,  // 0x01 <- Layer 2 bits
    1, 1, 0, 1, 0, 1, 0, 0,  // 0x2B
    1, 1, 0, 1, 0, 1, 0, 0,  // 0x2B
    1, 1, 0, 1, 0, 1, 0, 0,  // 0x2B
    1, 1, 0, 1, 0, 1, 0, 0,  // 0x2B
    1, 1, 0, 1, 0, 1, 0, 0,  // 0x2B
    1, 1, 0, 1, 0, 1, 0, 0,  // 0x2B
    1, 1, 0, 1, 0, 1, 0, 0,  // 0x2B
    1, 1, 0, 1, 0, 1, 0, 0,  // 0x2B
    1, 1, 0, 1, 0, 1, 0, 0,  // 0x2B
    1, 1, 0, 1, 0, 1, 0, 0,  // 0x2B
    1, 1, 0, 1, 0, 1, 0, 0,  // 0x2B
    1, 1, 0, 1, 0, 1, 0, 0,  // 0x2B
    1, 1, 0, 1, 0, 1, 0, 0,  // 0x2B
    1, 1, 0, 1, 0, 1, 0, 0,  // 0x2B
    1, 1, 0, 1, 0, 1, 0, 0,  // 0x2B
    1, 1, 0, 1, 0, 1, 0, 0,  // 0x2B
    1, 1, 0, 1, 0, 1, 0, 0,  // 0x2B
    1, 1, 0, 1, 0, 1, 0, 0,  // 0x2B
    1, 1, 0, 1, 0, 1, 0, 0,  // 0x2B
    1, 1, 0, 1, 0, 1, 0, 0,  // 0x2B
    1, 1, 0, 1, 0, 1, 0, 0,  // 0x2B
    1, 1, 0, 1, 0, 1, 0, 0,  // 0x2B <- N201 = 23
    0, 1, 1, 1, 0, 0, 0, 1,  // 0x8E <- Parity bits
    0, 0, 0, 0, 1, 0, 1, 0,
    1, 1, 1, 1, 1, 1, 1, 0,
    0, 1, 0, 0, 1, 0, 1, 0,
    0, 0, 0, 1, 0, 1, 0, 1,
    0, 0, 0, 0};

  parser_->Parse(&input[0], 184, "Normal");

  ASSERT_EQ("Normal:\n"
            "  Pseudo Length: 0\n",
            parser_->output_.str());
}

TEST_F(ParserTest, ParseCallControl) {
  // http://www.etsi.org/deliver/etsi_ts/100900_100999/100938/08.04.00_60/ts_100938v080400p.pdf
  // 04.06 ETSI page 12
  // LAPDm Bter format
  //   FACCH or SDCCH: N201 = 23;
  const uint8_t input[] = {  // MSB is to the right
    // 31 06 1c 32 f8 66 55 fc 60 46 b0 00 00 01 2b 2b 2b 2b 2b 2b 2b
    // 2b 2b

    // Bits 1 to 4 of the first octet of a standard L3 message contain
    // the protocol discriminator (PD). A pseudo length of one octet
    // is right before PD.
    1, 0, 0, 0, 1, 1, 0, 0,  // 0x31 PSEUDO
    0, 1, 1, 0, 0, 0, 0, 0,  // 0x06 TI/PD
    0, 0, 1, 1, 1, 0, 0, 0,  // 0x1C Message type
    0, 1, 0, 0, 1, 1, 0, 0,  // 0x32
    0, 0, 0, 1, 1, 1, 1, 1,  // 0xF8
    0, 1, 1, 0, 0, 1, 1, 0,  // 0x66
    1, 0, 1, 0, 1, 0, 1, 0,  // 0x55
    0, 0, 1, 1, 1, 1, 1, 1,  // 0xFC
    0, 0, 0, 0, 0, 1, 1, 0,  // 0x60
    0, 1, 1, 0, 0, 0, 1, 0,  // 0x46
    0, 0, 0, 0, 1, 1, 0, 1,  // 0xB0
    0, 0, 0, 0, 0, 0, 0, 0,  // 0x00
    0, 0, 0, 0, 0, 0, 0, 0,  // 0x00
    1, 0, 0, 0, 0, 0, 0, 0,  // 0x01
    1, 1, 0, 1, 0, 1, 0, 0,  // 0x2B
    1, 1, 0, 1, 0, 1, 0, 0,  // 0x2B
    1, 1, 0, 1, 0, 1, 0, 0,  // 0x2B
    1, 1, 0, 1, 0, 1, 0, 0,  // 0x2B
    1, 1, 0, 1, 0, 1, 0, 0,  // 0x2B
    1, 1, 0, 1, 0, 1, 0, 0,  // 0x2B
    1, 1, 0, 1, 0, 1, 0, 0,  // 0x2B
    1, 1, 0, 1, 0, 1, 0, 0,  // 0x2B
    1, 1, 0, 1, 0, 1, 0, 0,  // 0x2B <- N201 = 23
    0, 1, 0, 1, 1, 0, 1, 1,  // Parity
    1, 0, 1, 0, 0, 0, 0, 1,
    1, 1, 0, 0, 0, 1, 0, 1,
    1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 1, 0, 1, 0, 0, 0,
    0, 0, 0, 0};

  parser_->Parse(&input[0], 184, "Normal");

  ASSERT_EQ("Normal:\n"
            "  Pseudo Length: 12\n"
            "  Protocol Discriminator: 6 (Radio Resources Management Messages)\n"
            "  Transaction Identifier: 0\n"
            "  Message Type: 56 (STATUS)\n"
            "  ...\n",
            parser_->output_.str());
}
