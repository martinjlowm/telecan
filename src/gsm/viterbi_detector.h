/* -*- c++ -*- */
/*
 * @file
 * @author Piotr Krysik <ptrkrysik@gmail.com>
 * @section LICENSE
 *
 * Gr-gsm is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * Gr-gsm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gr-gsm; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

// Based on Piotr's implementation and GSMsim

#ifndef GSM_VITERBI_DETECTOR_H_
#define GSM_VITERBI_DETECTOR_H_

#include "helper_functions.h"
#include "gsm/sequence_estimator.h"

namespace GSM {

class ViterbiDetector {
 public:
  ViterbiDetector();
  ~ViterbiDetector();

  void Estimate(const gr_complex *filtered_burst,
                const gr_complex *Rhh,
                float *output);

 protected:

 private:
  uint32_t start_state_;
  uint32_t num_stops_;

  static const uint32_t stop_states_[];
  static const uint32_t num_paths_;
  static const uint32_t prev_table_[][2];
  static const uint32_t parity_table_[];
};

}  // namespace GSM

#endif  // GSM_VITERBI_DETECTOR_H_
