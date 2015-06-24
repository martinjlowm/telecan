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

// Based on Piotr's gr-gsm implementation and GSMsim

#include <iostream>

#include "helper_functions.h"
#include "gsm/constants.h"
#include "gsm/viterbi_detector.h"

// 2^{L_h - 1} = 2^4 = (2^{L_h}) / 2
const uint32_t GSM::ViterbiDetector::num_paths_ = 1 << (kCIRLength - 1);
const uint32_t GSM::ViterbiDetector::prev_table_[num_paths_][2] = {
  {0, 8}, {0, 8}, {1, 9}, {1, 9},
  {2,10}, {2,10}, {3,11}, {3,11},
  {4,12}, {4,12}, {5,13}, {5,13},
  {6,14}, {6,14}, {7,15}, {7,15}
};
const uint32_t GSM::ViterbiDetector::parity_table_[num_paths_] = {
  0, 1, 1, 0, 0, 1, 1, 0,
  0, 1, 1, 0, 0, 1, 1, 0
};
const uint32_t GSM::ViterbiDetector::stop_states_[2] = {4, 12};

GSM::ViterbiDetector::ViterbiDetector() {
  start_state_ = 3;
  num_stops_ = 2;
}

void GSM::ViterbiDetector::Estimate(const gr_complex *filtered_burst,
                                    const gr_complex *Rhh,
                                    float *output) {
  float increment[8];
  float path_metrics1[16];
  float path_metrics2[16];
  float paths_difference;
  float *new_path_metrics;
  float *old_path_metrics;
  float *tmp;
  float trans_table[kBurstSize][16];
  float pm_candidate1, pm_candidate2;
  bool real_imag;
  float input_symbol_real, input_symbol_imag;
  uint32_t i, sample_num;

  /*
   * Setup first path metrics, so only state pointed by start_state is
   * possible.  Start_state metric is equal to zero, the rest is
   * written with some very low value, which makes them practically
   * impossible to occur.
   */
  for (i = 0; i < num_paths_; i++) {
    path_metrics1[i] = -10e30;
  }
  path_metrics1[start_state_] = 0;

  /*
   * Compute Increment - a table of values which does not change for
   * subsequent input samples.  Increment is table of reference levels
   * for computation of branch metrics:
   *
   *    branch metric = (+/-)received_sample (+/-) reference_level
   */
  // There are 8 different combinations of a run with CIR length 4.
  //
  //               +|--- 7
  //           +|---|
  //            |  -|--- 6
  //       +|---|
  //        |   |  +|--- 5
  //        |  -|---|
  //        |      -|--- 4
  //   +|---|
  //    |   |      +|--- 3
  //    |   |  +|---|
  //    |   |   |  -|--- 2
  //    |  -|---|
  //    |       |  +|--- 1
  //    |      -|---|
  //    |          -|--- 0
  // ---|
  //

  increment[0] = -Rhh[1].imag() -Rhh[2].real() -Rhh[3].imag() +Rhh[4].real();
  increment[1] = Rhh[1].imag() -Rhh[2].real() -Rhh[3].imag() +Rhh[4].real();
  increment[2] = -Rhh[1].imag() +Rhh[2].real() -Rhh[3].imag() +Rhh[4].real();
  increment[3] = Rhh[1].imag() +Rhh[2].real() -Rhh[3].imag() +Rhh[4].real();
  increment[4] = -Rhh[1].imag() -Rhh[2].real() +Rhh[3].imag() +Rhh[4].real();
  increment[5] = Rhh[1].imag() -Rhh[2].real() +Rhh[3].imag() +Rhh[4].real();
  increment[6] = -Rhh[1].imag() +Rhh[2].real() +Rhh[3].imag() +Rhh[4].real();
  increment[7] = Rhh[1].imag() +Rhh[2].real() +Rhh[3].imag() +Rhh[4].real();

  /*
   * Computation of path metrics and decisions (Add-Compare-Select).
   * It's composed of two parts: one for odd input samples (imaginary
   * numbers) and one for even samples (real numbers).
   *
   * Each part is composed of independent (parallelisable) statements
   * like this one:
   *
   *      pm_candidate1 = old_path_metrics[0] -input_symbol_imag +increment[2];
   *      pm_candidate2 = old_path_metrics[8] -input_symbol_imag -increment[5];
   *      paths_difference=pm_candidate2-pm_candidate1;
   *      new_path_metrics[1]=(paths_difference<0) ? pm_candidate1 : pm_candidate2;
   *      trans_table[sample_nr][1] = paths_difference;
   *
   * This is very good point for optimisations (SIMD or OpenMP) as
   * it's most time consuming part of this function.
   */

  sample_num=0;
  old_path_metrics=path_metrics1;
  new_path_metrics=path_metrics2;
  while (sample_num < kBurstSize) {
    //Processing imag states
    real_imag=1;
    input_symbol_imag = filtered_burst[sample_num].imag();

    pm_candidate1 = old_path_metrics[0] +input_symbol_imag -increment[2];
    pm_candidate2 = old_path_metrics[8] +input_symbol_imag +increment[5];
    paths_difference=pm_candidate2-pm_candidate1;
    new_path_metrics[0]=(paths_difference<0) ? pm_candidate1 : pm_candidate2;
    trans_table[sample_num][0] = paths_difference;

    pm_candidate1 = old_path_metrics[0] -input_symbol_imag +increment[2];
    pm_candidate2 = old_path_metrics[8] -input_symbol_imag -increment[5];
    paths_difference=pm_candidate2-pm_candidate1;
    new_path_metrics[1]=(paths_difference<0) ? pm_candidate1 : pm_candidate2;
    trans_table[sample_num][1] = paths_difference;

    pm_candidate1 = old_path_metrics[1] +input_symbol_imag -increment[3];
    pm_candidate2 = old_path_metrics[9] +input_symbol_imag +increment[4];
    paths_difference=pm_candidate2-pm_candidate1;
    new_path_metrics[2]=(paths_difference<0) ? pm_candidate1 : pm_candidate2;
    trans_table[sample_num][2] = paths_difference;

    pm_candidate1 = old_path_metrics[1] -input_symbol_imag +increment[3];
    pm_candidate2 = old_path_metrics[9] -input_symbol_imag -increment[4];
    paths_difference=pm_candidate2-pm_candidate1;
    new_path_metrics[3]=(paths_difference<0) ? pm_candidate1 : pm_candidate2;
    trans_table[sample_num][3] = paths_difference;

    pm_candidate1 = old_path_metrics[2] +input_symbol_imag -increment[0];
    pm_candidate2 = old_path_metrics[10] +input_symbol_imag +increment[7];
    paths_difference=pm_candidate2-pm_candidate1;
    new_path_metrics[4]=(paths_difference<0) ? pm_candidate1 : pm_candidate2;
    trans_table[sample_num][4] = paths_difference;

    pm_candidate1 = old_path_metrics[2] -input_symbol_imag +increment[0];
    pm_candidate2 = old_path_metrics[10] -input_symbol_imag -increment[7];
    paths_difference=pm_candidate2-pm_candidate1;
    new_path_metrics[5]=(paths_difference<0) ? pm_candidate1 : pm_candidate2;
    trans_table[sample_num][5] = paths_difference;

    pm_candidate1 = old_path_metrics[3] +input_symbol_imag -increment[1];
    pm_candidate2 = old_path_metrics[11] +input_symbol_imag +increment[6];
    paths_difference=pm_candidate2-pm_candidate1;
    new_path_metrics[6]=(paths_difference<0) ? pm_candidate1 : pm_candidate2;
    trans_table[sample_num][6] = paths_difference;

    pm_candidate1 = old_path_metrics[3] -input_symbol_imag +increment[1];
    pm_candidate2 = old_path_metrics[11] -input_symbol_imag -increment[6];
    paths_difference=pm_candidate2-pm_candidate1;
    new_path_metrics[7]=(paths_difference<0) ? pm_candidate1 : pm_candidate2;
    trans_table[sample_num][7] = paths_difference;

    pm_candidate1 = old_path_metrics[4] +input_symbol_imag -increment[6];
    pm_candidate2 = old_path_metrics[12] +input_symbol_imag +increment[1];
    paths_difference=pm_candidate2-pm_candidate1;
    new_path_metrics[8]=(paths_difference<0) ? pm_candidate1 : pm_candidate2;
    trans_table[sample_num][8] = paths_difference;

    pm_candidate1 = old_path_metrics[4] -input_symbol_imag +increment[6];
    pm_candidate2 = old_path_metrics[12] -input_symbol_imag -increment[1];
    paths_difference=pm_candidate2-pm_candidate1;
    new_path_metrics[9]=(paths_difference<0) ? pm_candidate1 : pm_candidate2;
    trans_table[sample_num][9] = paths_difference;

    pm_candidate1 = old_path_metrics[5] +input_symbol_imag -increment[7];
    pm_candidate2 = old_path_metrics[13] +input_symbol_imag +increment[0];
    paths_difference=pm_candidate2-pm_candidate1;
    new_path_metrics[10]=(paths_difference<0) ? pm_candidate1 : pm_candidate2;
    trans_table[sample_num][10] = paths_difference;

    pm_candidate1 = old_path_metrics[5] -input_symbol_imag +increment[7];
    pm_candidate2 = old_path_metrics[13] -input_symbol_imag -increment[0];
    paths_difference=pm_candidate2-pm_candidate1;
    new_path_metrics[11]=(paths_difference<0) ? pm_candidate1 : pm_candidate2;
    trans_table[sample_num][11] = paths_difference;

    pm_candidate1 = old_path_metrics[6] +input_symbol_imag -increment[4];
    pm_candidate2 = old_path_metrics[14] +input_symbol_imag +increment[3];
    paths_difference=pm_candidate2-pm_candidate1;
    new_path_metrics[12]=(paths_difference<0) ? pm_candidate1 : pm_candidate2;
    trans_table[sample_num][12] = paths_difference;

    pm_candidate1 = old_path_metrics[6] -input_symbol_imag +increment[4];
    pm_candidate2 = old_path_metrics[14] -input_symbol_imag -increment[3];
    paths_difference=pm_candidate2-pm_candidate1;
    new_path_metrics[13]=(paths_difference<0) ? pm_candidate1 : pm_candidate2;
    trans_table[sample_num][13] = paths_difference;

    pm_candidate1 = old_path_metrics[7] +input_symbol_imag -increment[5];
    pm_candidate2 = old_path_metrics[15] +input_symbol_imag +increment[2];
    paths_difference=pm_candidate2-pm_candidate1;
    new_path_metrics[14]=(paths_difference<0) ? pm_candidate1 : pm_candidate2;
    trans_table[sample_num][14] = paths_difference;

    pm_candidate1 = old_path_metrics[7] -input_symbol_imag +increment[5];
    pm_candidate2 = old_path_metrics[15] -input_symbol_imag -increment[2];
    paths_difference=pm_candidate2-pm_candidate1;
    new_path_metrics[15]=(paths_difference<0) ? pm_candidate1 : pm_candidate2;
    trans_table[sample_num][15] = paths_difference;
    tmp=old_path_metrics;
    old_path_metrics=new_path_metrics;
    new_path_metrics=tmp;

    sample_num++;
    if(sample_num==kBurstSize)
      break;

    //Processing real states
    real_imag=0;
    input_symbol_real = filtered_burst[sample_num].real();

    pm_candidate1 = old_path_metrics[0] -input_symbol_real -increment[7];
    pm_candidate2 = old_path_metrics[8] -input_symbol_real +increment[0];
    paths_difference=pm_candidate2-pm_candidate1;
    new_path_metrics[0]=(paths_difference<0) ? pm_candidate1 : pm_candidate2;
    trans_table[sample_num][0] = paths_difference;

    pm_candidate1 = old_path_metrics[0] +input_symbol_real +increment[7];
    pm_candidate2 = old_path_metrics[8] +input_symbol_real -increment[0];
    paths_difference=pm_candidate2-pm_candidate1;
    new_path_metrics[1]=(paths_difference<0) ? pm_candidate1 : pm_candidate2;
    trans_table[sample_num][1] = paths_difference;

    pm_candidate1 = old_path_metrics[1] -input_symbol_real -increment[6];
    pm_candidate2 = old_path_metrics[9] -input_symbol_real +increment[1];
    paths_difference=pm_candidate2-pm_candidate1;
    new_path_metrics[2]=(paths_difference<0) ? pm_candidate1 : pm_candidate2;
    trans_table[sample_num][2] = paths_difference;

    pm_candidate1 = old_path_metrics[1] +input_symbol_real +increment[6];
    pm_candidate2 = old_path_metrics[9] +input_symbol_real -increment[1];
    paths_difference=pm_candidate2-pm_candidate1;
    new_path_metrics[3]=(paths_difference<0) ? pm_candidate1 : pm_candidate2;
    trans_table[sample_num][3] = paths_difference;

    pm_candidate1 = old_path_metrics[2] -input_symbol_real -increment[5];
    pm_candidate2 = old_path_metrics[10] -input_symbol_real +increment[2];
    paths_difference=pm_candidate2-pm_candidate1;
    new_path_metrics[4]=(paths_difference<0) ? pm_candidate1 : pm_candidate2;
    trans_table[sample_num][4] = paths_difference;

    pm_candidate1 = old_path_metrics[2] +input_symbol_real +increment[5];
    pm_candidate2 = old_path_metrics[10] +input_symbol_real -increment[2];
    paths_difference=pm_candidate2-pm_candidate1;
    new_path_metrics[5]=(paths_difference<0) ? pm_candidate1 : pm_candidate2;
    trans_table[sample_num][5] = paths_difference;

    pm_candidate1 = old_path_metrics[3] -input_symbol_real -increment[4];
    pm_candidate2 = old_path_metrics[11] -input_symbol_real +increment[3];
    paths_difference=pm_candidate2-pm_candidate1;
    new_path_metrics[6]=(paths_difference<0) ? pm_candidate1 : pm_candidate2;
    trans_table[sample_num][6] = paths_difference;

    pm_candidate1 = old_path_metrics[3] +input_symbol_real +increment[4];
    pm_candidate2 = old_path_metrics[11] +input_symbol_real -increment[3];
    paths_difference=pm_candidate2-pm_candidate1;
    new_path_metrics[7]=(paths_difference<0) ? pm_candidate1 : pm_candidate2;
    trans_table[sample_num][7] = paths_difference;

    pm_candidate1 = old_path_metrics[4] -input_symbol_real -increment[3];
    pm_candidate2 = old_path_metrics[12] -input_symbol_real +increment[4];
    paths_difference=pm_candidate2-pm_candidate1;
    new_path_metrics[8]=(paths_difference<0) ? pm_candidate1 : pm_candidate2;
    trans_table[sample_num][8] = paths_difference;

    pm_candidate1 = old_path_metrics[4] +input_symbol_real +increment[3];
    pm_candidate2 = old_path_metrics[12] +input_symbol_real -increment[4];
    paths_difference=pm_candidate2-pm_candidate1;
    new_path_metrics[9]=(paths_difference<0) ? pm_candidate1 : pm_candidate2;
    trans_table[sample_num][9] = paths_difference;

    pm_candidate1 = old_path_metrics[5] -input_symbol_real -increment[2];
    pm_candidate2 = old_path_metrics[13] -input_symbol_real +increment[5];
    paths_difference=pm_candidate2-pm_candidate1;
    new_path_metrics[10]=(paths_difference<0) ? pm_candidate1 : pm_candidate2;
    trans_table[sample_num][10] = paths_difference;

    pm_candidate1 = old_path_metrics[5] +input_symbol_real +increment[2];
    pm_candidate2 = old_path_metrics[13] +input_symbol_real -increment[5];
    paths_difference=pm_candidate2-pm_candidate1;
    new_path_metrics[11]=(paths_difference<0) ? pm_candidate1 : pm_candidate2;
    trans_table[sample_num][11] = paths_difference;

    pm_candidate1 = old_path_metrics[6] -input_symbol_real -increment[1];
    pm_candidate2 = old_path_metrics[14] -input_symbol_real +increment[6];
    paths_difference=pm_candidate2-pm_candidate1;
    new_path_metrics[12]=(paths_difference<0) ? pm_candidate1 : pm_candidate2;
    trans_table[sample_num][12] = paths_difference;

    pm_candidate1 = old_path_metrics[6] +input_symbol_real +increment[1];
    pm_candidate2 = old_path_metrics[14] +input_symbol_real -increment[6];
    paths_difference=pm_candidate2-pm_candidate1;
    new_path_metrics[13]=(paths_difference<0) ? pm_candidate1 : pm_candidate2;
    trans_table[sample_num][13] = paths_difference;

    pm_candidate1 = old_path_metrics[7] -input_symbol_real -increment[0];
    pm_candidate2 = old_path_metrics[15] -input_symbol_real +increment[7];
    paths_difference=pm_candidate2-pm_candidate1;
    new_path_metrics[14]=(paths_difference<0) ? pm_candidate1 : pm_candidate2;
    trans_table[sample_num][14] = paths_difference;

    pm_candidate1 = old_path_metrics[7] +input_symbol_real +increment[0];
    pm_candidate2 = old_path_metrics[15] +input_symbol_real -increment[7];
    paths_difference=pm_candidate2-pm_candidate1;
    new_path_metrics[15]=(paths_difference<0) ? pm_candidate1 : pm_candidate2;
    trans_table[sample_num][15] = paths_difference;

    tmp=old_path_metrics;
    old_path_metrics=new_path_metrics;
    new_path_metrics=tmp;

    sample_num++;
  }

  /*
   * Find the best from the stop states by comparing their path
   * metrics.  Not every stop state is always possible, so we are
   * searching in a subset of them.
   */
  uint32_t best_stop_state;
  float stop_state_metric, max_stop_state_metric;
  best_stop_state = stop_states_[0];
  max_stop_state_metric = old_path_metrics[best_stop_state];

  for (i = 1; i < num_stops_; i++) {
    stop_state_metric = old_path_metrics[stop_states_[i]];

    if (stop_state_metric > max_stop_state_metric) {
      max_stop_state_metric = stop_state_metric;
      best_stop_state = stop_states_[i];
    }
  }

  /*
   * Traceback and differential decoding of received sequence.
   * Decisions stored in trans_table are used to restore best path in
   * the trellis.
   */
  sample_num = kBurstSize;
  uint32_t state_num = best_stop_state;
  uint32_t decision;
  bool out_bit = 0;

  while (sample_num > 0) {
    sample_num--;
    decision = trans_table[sample_num][state_num] > 0;

    if (decision != out_bit) {
      output[sample_num] = -trans_table[sample_num][state_num];
    } else {
      output[sample_num] = trans_table[sample_num][state_num];
    }

    out_bit = out_bit ^ real_imag ^ parity_table_[state_num];
    state_num = prev_table_[state_num][decision];
    real_imag = !real_imag;
  }
}
