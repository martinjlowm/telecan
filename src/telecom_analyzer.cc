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

#include <getopt.h>
#include <libgen.h>

#include "helper_functions.h"
#include "device/hackrf.h"
#include "gsm/analyzer.h"
#include "gsm/constants.h"
// #include "umts/analyzer.h"
// #include "lte/analyzer.h"

int g_verbosity = 0;
int g_debug = 0;

void usage(char *prog) {
  printf("telecan\n");
  printf("\nUsage:\n");
  printf("\tBase Station scan:\n");
  printf("\t\t%s -s <band indicator>\n", basename(prog));
  printf("\n");
  printf("\tTune to channel frequency:\n");
  printf("\t\t%s -b <band indicator> -c <channel number>\n", basename(prog));
  printf("\n");
  printf("Where options are:\n");
  printf("\t-s\tband to scan (GSM850, GSM-R, GSM900, EGSM, DCS, PCS)\n");
  printf("\t-c\tchannel of nearby GSM base station\n");
  printf("\t-b\tband indicator (GSM850, GSM-R, GSM900, EGSM, DCS, PCS)\n");

  exit(-1);
}

int main(int argc, char **argv) {
  bool scan_bts = false;
  int option, band_indicator, channel = -1;
  int64_t sample_rate = kSampleRate;

  Device::HackRF *sdr;

  while ((option = getopt(argc, argv, "c:s:b:h?")) != EOF) {
    switch (option) {
      case 'c':
        channel = strtoul(optarg, 0, 0);
        break;

      case 's':
        scan_bts = true;

      case 'b':
        if ((band_indicator = str_to_bi(optarg)) == -1) {
          fprintf(stderr, "error: bad band indicator: ``%s''\n", optarg);
          usage(argv[0]);
        }
        break;

      case 'h':
      case '?':
      default:
        usage(argv[0]);
        break;
    }
  }

  sdr = new Device::HackRF(sample_rate);
  if (!sdr) {
    fprintf(stderr, "Error: sdr_source\n");
    return -1;
  }

  if (band_indicator == BI_NOT_DEFINED) {
    fprintf(stderr, "Error: Band not defined\n");
    return -1;
  }

  if (channel < 0) {
    scan_bts = true;
  }

  BaseAnalyzer *analyzer;
  if (band_indicator == GSM_850 ||
      band_indicator == GSM_R_900 ||
      band_indicator == GSM_900 ||
      band_indicator == GSM_E_900 ||
      band_indicator == DCS_1800 ||
      band_indicator == PCS_1900) {
    analyzer = new GSM::Analyzer(sdr, band_indicator, scan_bts);

    if (analyzer->HasScanned()) {
      std::map<int, double> channels = analyzer->GetAvailableChannels();
      std::map<int, double>::iterator channel_iterator;

      printf("Scan is complete. Found %lu channels.\n", channels.size());
      for (channel_iterator = channels.begin();
           channel_iterator != channels.end();
           ++channel_iterator) {
        printf("\tChannel: %d\tFrequency:%.4fHz\n",
               channel_iterator->first,
               channel_iterator->second);
      }
      channel = 782;
      // channel = // Choose from GUI <--------------
      // Allow the user to choose one of these frequencies at some
      // point.
    }
  // } else if (band_indicator == UMTS_2100) {
  //   analyzer = new UMTS::Analyzer(band_indicator);
  // } else if (band_indicator == LTE_1900) {
  //    analyzer = new LTE::Analyzer(band_indicator);
  } else {
    return 0;
  }
  analyzer->set_current_channel(channel);

  // Run synchronization state machine
  analyzer->Analyze();

  return 0;
}
