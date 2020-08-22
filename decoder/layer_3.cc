//
//    JMP123 Porting into c++ 20
//    Copyright (C) 2020  Jason <m.jason.liu@outlook.com>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License along
//    with this program; if not, write to the Free Software Foundation, Inc.,
//    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//

#include "layer_3.h"

#include <cmath>
#include <memory>
#include <thread>

#include "bit_stream.h"
#include "bit_stream_main_data.h"
#include "synthesis_concurrent.h"
#include "tables.h"

namespace jmp123::decoder {

[[maybe_unused]] LayerIII::LayerIII(Header h, std::unique_ptr<IAudio> audio)
    : LayerI_II_III(h, std::move(audio)),
      header_(std::move(h)),
      channels_(h.GetChannelCount()),
      bs_si_(BitStream(0, 0)),
      is_mpeg_1_(header_.GetVersion()
                 == static_cast<int>(static_cast<int>(MPEGVersion::kMPEG1))),
      granules_(is_mpeg_1_ ? 2 : 1),
      filter_ch_0_(SynthesisConcurrent(*this, 0)),
      filter_ch_1((SynthesisConcurrent(*this, 1))) {
  semaphore_        = channels_;
  main_data_stream_ = std::make_unique<BitStreamMainData>(4096, 512);
  scfsi_            = std::vector<int>(channels_, 0);
  //    scalefacLong = new int[channels_][23];
  scalefacLong = decltype(scalefacLong)(channels_);  // new int[channels_ * 23]
  // scalefacShort = new int[channels_][3 * 13];
  scalefacShort =
      decltype(scalefacShort)(channels_);  // new int[channels_ * 3 * 13]
  hv.fill(0);
  channel_info_ = decltype(channel_info_)(
      granules_,
      std::vector<ChannelInformation>(channels_));  // granules_ * channels_

  // new Thread(filterCh0, "synthesis_left").start();
  // DO WE NEED JOIN IT???
  // condition_variable is not movable nor copiable, so we can't do
  // std::thread t1{filter_ch_0_};
  // TODO: Maybe we should change thread callback function from operator() to
  // normal function like `void Run();`
  // TODO: CONSIDER SAVE HANDLE AND JOIN IT WHEN DESTRUCT
  t1    = std::thread{&SynthesisConcurrent::operator(), &filter_ch_0_};
  xrch0 = filter_ch_0_.GetBuffer();
  preBlckCh0.fill(0);
  if (channels_ == 2) {
    t2    = std::thread{&SynthesisConcurrent::operator(), &filter_ch_1};
    xrch1 = filter_ch_1.GetBuffer();
    preBlckCh1.fill(0);
  }

  int i = 0;

  // 用于查表求 v^(4/3)，v是经哈夫曼解码出的一个(正)值，该值的范围是0..8191
  for (i = 0; i < 8207; i++) floatPowIS[i] = (float)std::pow(i, 4.0 / 3.0);

  // 用于查表求 2^(-0.25 * i)
  // 按公式短块时i最大值: 210 - 0   + 8 * 7 + 4 * 15 + 2 = 328
  // 长块或短块时i最小值: 210 - 255 + 0     + 0      + 0 = -45
  // 查表法时下标范围为0..328+45.
  for (i = 0; i < 374; i++)
    floatPow2[i] = (float)std::pow(2.0, -0.25 * (i - 45));

  //---------------------------------------------------------------------
  //待解码文件的不同特征用到不同的变量.初始化:
  //---------------------------------------------------------------------
  int sfreq = header_.GetSamplingFrequency();
  sfreq += is_mpeg_1_
               ? 0
               : ((header_.GetVersion()
                   == static_cast<int>(static_cast<int>(MPEGVersion::kMPEG2)))
                      ? 3
                      : 6);

  // ANNEX B,Table 3-B.8. Layer III scalefactor bands
  switch (sfreq) {
    case 0:
      // MPEG-1, sampling_frequency=0, 44.1kHz
      sfb_index_long_ = {0,  4,  8,   12,  16,  20,  24,  30,  36,  44,  52, 62,
                         74, 90, 110, 134, 162, 196, 238, 288, 342, 418, 576};
      sfb_index_short_ = {0,  4,  8,  12, 16,  22,  30,
                          40, 52, 66, 84, 106, 136, 192};
      break;
    case 1:
      // MPEG-1, sampling_frequency=1, 48kHz
      sfb_index_long_ = {0,  4,  8,   12,  16,  20,  24,  30,  36,  42,  50, 60,
                         72, 88, 106, 128, 156, 190, 230, 276, 330, 384, 576};
      sfb_index_short_ = {0,  4,  8,  12, 16,  22,  28,
                          38, 50, 64, 80, 100, 126, 192};
      break;
    case 2:
      // MPEG-1, sampling_frequency=2, 32kHz
      sfb_index_long_  = {0,   4,   8,   12,  16,  20,  24,  30,
                         36,  44,  54,  66,  82,  102, 126, 156,
                         194, 240, 296, 364, 448, 550, 576};
      sfb_index_short_ = {0,  4,  8,  12,  16,  22,  30,
                          42, 58, 78, 104, 138, 180, 192};
      break;

    case 3:
      // MPEG-2, sampling_frequency=0, 22.05kHz
      sfb_index_long_  = {0,   6,   12,  18,  24,  30,  36,  44,
                         54,  66,  80,  96,  116, 140, 168, 200,
                         238, 284, 336, 396, 464, 522, 576};
      sfb_index_short_ = {0,  4,  8,  12,  18,  24,  32,
                          42, 56, 74, 100, 132, 174, 192};
      break;
    case 4:
      // MPEG-2, sampling_frequency=1, 24kHz
      sfb_index_long_  = {0,   6,   12,  18,  24,  30,  36,  44,
                         54,  66,  80,  96,  114, 136, 162, 194,
                         232, 278, 330, 394, 464, 540, 576};
      sfb_index_short_ = {0,  4,  8,  12,  18,  26,  36,
                          48, 62, 80, 104, 136, 180, 192};
      break;
    case 5:
      // MPEG-2, sampling_frequency=2, 16kHz
      sfb_index_long_  = {0,   6,   12,  18,  24,  30,  36,  44,
                         54,  66,  80,  96,  116, 140, 168, 200,
                         238, 284, 336, 396, 464, 522, 576};
      sfb_index_short_ = {0,  4,  8,  12,  18,  26,  36,
                          48, 62, 80, 104, 134, 174, 192};
      break;
    case 6:
      // MPEG-2.5, sampling_frequency=0, 11.025kHz
      sfb_index_long_  = {0,   6,   12,  18,  24,  30,  36,  44,
                         54,  66,  80,  96,  116, 140, 168, 200,
                         238, 284, 336, 396, 464, 522, 576};
      sfb_index_short_ = {0,  4,  8,  12,  18,  26,  36,
                          48, 62, 80, 104, 134, 174, 192};
      break;
    case 7:
      // MPEG-2.5, sampling_frequency=1, 12kHz
      sfb_index_long_  = {0,   6,   12,  18,  24,  30,  36,  44,
                         54,  66,  80,  96,  116, 140, 168, 200,
                         238, 284, 336, 396, 464, 522, 576};
      sfb_index_short_ = {0,  4,  8,  12,  18,  26,  36,
                          48, 62, 80, 104, 134, 174, 192};
      break;
    case 8:
      // MPEG-2.5, sampling_frequency=2, 8kHz
      sfb_index_long_  = {0,   12,  24,  36,  48,  60,  72,  88,
                         108, 132, 160, 192, 232, 280, 336, 400,
                         476, 566, 568, 570, 572, 574, 576};
      sfb_index_short_ = {0,  8,   16,  24,  36,  52,  72,
                          96, 124, 160, 162, 164, 166, 192};
      break;
  }
  for (i = 0; i < 22; i++)
    widthLong[i] = sfb_index_long_[i + 1] - sfb_index_long_[i];
  for (i = 0; i < 13; i++)
    widthShort[i] = sfb_index_short_[i + 1] - sfb_index_short_[i];
  //-----------------------------------------------------------------
  if (is_mpeg_1_) {
    // MPEG-1, intensity_stereo
    is_coef = {0.0f,         0.211324865f, 0.366025404f, 0.5f,
               0.633974596f, 0.788675135f, 1.0f};
  } else {
    // MPEG-2, intensity_stereo
    lsf_is_coef = {
        {0.840896415f, 0.707106781f, 0.594603558f, 0.5f, 0.420448208f,
         0.353553391f, 0.297301779f, 0.25f, 0.210224104f, 0.176776695f,
         0.148650889f, 0.125f, 0.105112052f, 0.088388348f, 0.074325445f},
        {0.707106781f, 0.5f, 0.353553391f, 0.25f, 0.176776695f, 0.125f,
         0.088388348f, 0.0625f, 0.044194174f, 0.03125f, 0.022097087f, 0.015625f,
         0.011048543f, 0.0078125f, 0.005524272f}};

    i_slen2.fill(0);  // MPEG-2 slen for intensity_stereo
    n_slen2.fill(0);  // MPEG-2 slen for normal mode
    nr_of_sfb = {     // ISO/IEC 13818-3 subclause 2.4.3.2 nr_of_sfbx x=1..4
                 std::to_array({std::to_array<uint8_t, 4>({6, 5, 5, 5}),
                                std::to_array<uint8_t, 4>({6, 5, 7, 3}),
                                std::to_array<uint8_t, 4>({11, 10, 0, 0}),
                                std::to_array<uint8_t, 4>({7, 7, 7, 0}),
                                std::to_array<uint8_t, 4>({6, 6, 6, 3}),
                                std::to_array<uint8_t, 4>({8, 8, 5, 0})}),
                 std::to_array({std::to_array<uint8_t, 4>({9, 9, 9, 9}),
                                std::to_array<uint8_t, 4>({9, 9, 12, 6}),
                                std::to_array<uint8_t, 4>({18, 18, 0, 0}),
                                std::to_array<uint8_t, 4>({12, 12, 12, 0}),
                                std::to_array<uint8_t, 4>({12, 9, 9, 6}),
                                std::to_array<uint8_t, 4>({15, 12, 9, 0})}),
                 std::to_array({std::to_array<uint8_t, 4>({6, 9, 9, 9}),
                                std::to_array<uint8_t, 4>({6, 9, 12, 6}),
                                std::to_array<uint8_t, 4>({15, 18, 0, 0}),
                                std::to_array<uint8_t, 4>({6, 15, 12, 0}),
                                std::to_array<uint8_t, 4>({6, 12, 9, 6}),
                                std::to_array<uint8_t, 4>({6, 18, 9, 0})})};

    // ISO/IEC 13818-3 subclause 2.4.3.2 slenx, x=1..4
    int j, k, l, n;
    for (i = 0; i < 5; i++)
      for (j = 0; j < 6; j++)
        for (k = 0; k < 6; k++) {
          n          = k + j * 6 + i * 36;
          i_slen2[n] = i | (j << 3) | (k << 6) | (3 << 12);
        }
    for (i = 0; i < 4; i++)
      for (j = 0; j < 4; j++)
        for (k = 0; k < 4; k++) {
          n                = k + (j << 2) + (i << 4);
          i_slen2[n + 180] = i | (j << 3) | (k << 6) | (4 << 12);
        }
    for (i = 0; i < 4; i++)
      for (j = 0; j < 3; j++) {
        n                = j + i * 3;
        i_slen2[n + 244] = i | (j << 3) | (5 << 12);
        n_slen2[n + 500] = i | (j << 3) | (2 << 12) | (1 << 15);
      }
    for (i = 0; i < 5; i++)
      for (j = 0; j < 5; j++)
        for (k = 0; k < 4; k++)
          for (l = 0; l < 4; l++) {
            n          = l + (k << 2) + (j << 4) + i * 80;
            n_slen2[n] = i | (j << 3) | (k << 6) | (l << 9);
          }
    for (i = 0; i < 5; i++)
      for (j = 0; j < 5; j++)
        for (k = 0; k < 4; k++) {
          n                = k + (j << 2) + i * 20;
          n_slen2[n + 400] = i | (j << 3) | (k << 6) | (1 << 12);
        }
  }
}

// 1.
//>>>>SIDE INFORMATION (part1)=============================================
// private int part2_3_bits;//----debug
int LayerIII::GetSideInfo(std::vector<uint8_t> const& b, int off) {
  int ch, gr;
  // part2_3_bits = 0;
  bs_si_.Feed(b, off);

  if (is_mpeg_1_) {
    main_data_begin_ = bs_si_.GetBits_9(9);
    if (channels_ == 1) {
      bs_si_.SkipBits(5);  // private_bits
      scfsi_[0] = bs_si_.GetBits_9(4);
    } else {
      bs_si_.SkipBits(3);  // private_bits
      scfsi_[0] = bs_si_.GetBits_9(4);
      scfsi_[1] = bs_si_.GetBits_9(4);
    }

    for (gr = 0; gr < 2; gr++) {
      for (ch = 0; ch < channels_; ch++) {
        auto& ci          = channel_info_[gr][ch];
        ci.part2_3_length = bs_si_.GetBits_17(12);
        // part2_3_bits += ci.part2_3_length;
        ci.big_values            = bs_si_.GetBits_9(9);
        ci.global_gain           = bs_si_.GetBits_9(8);
        ci.scalefac_compress     = bs_si_.GetBits_9(4);
        ci.window_switching_flag = bs_si_.Get_1_Bit();
        if ((ci.window_switching_flag) != 0) {
          ci.block_type       = bs_si_.GetBits_9(2);
          ci.mixed_block_flag = bs_si_.Get_1_Bit();
          ci.table_select[0]  = bs_si_.GetBits_9(5);
          ci.table_select[1]  = bs_si_.GetBits_9(5);
          ci.subblock_gain[0] = bs_si_.GetBits_9(3);
          ci.subblock_gain[1] = bs_si_.GetBits_9(3);
          ci.subblock_gain[2] = bs_si_.GetBits_9(3);
          if (ci.block_type == 0)
            return -1;
          else if (ci.block_type == 2 && ci.mixed_block_flag == 0)
            ci.region0_count = 8;
          else
            ci.region0_count = 7;
          ci.region1_count = 20 - ci.region0_count;
        } else {
          ci.table_select[0] = bs_si_.GetBits_9(5);
          ci.table_select[1] = bs_si_.GetBits_9(5);
          ci.table_select[2] = bs_si_.GetBits_9(5);
          ci.region0_count   = bs_si_.GetBits_9(4);
          ci.region1_count   = bs_si_.GetBits_9(3);
          ci.block_type      = 0;
        }
        ci.preflag            = bs_si_.Get_1_Bit();
        ci.scalefac_scale     = bs_si_.Get_1_Bit();
        ci.count1table_select = bs_si_.Get_1_Bit();
      }
    }
  } else {
    // MPEG-2
    main_data_begin_ = bs_si_.GetBits_9(8);
    if (channels_ == 1)
      bs_si_.Get_1_Bit();
    else
      bs_si_.GetBits_9(2);
    for (ch = 0; ch < channels_; ch++) {
      auto& ci          = channel_info_[0][ch];
      ci.part2_3_length = bs_si_.GetBits_17(12);
      // part2_3_bits += ci.part2_3_length;
      ci.big_values            = bs_si_.GetBits_9(9);
      ci.global_gain           = bs_si_.GetBits_9(8);
      ci.scalefac_compress     = bs_si_.GetBits_9(9);
      ci.window_switching_flag = bs_si_.Get_1_Bit();
      if ((ci.window_switching_flag) != 0) {
        ci.block_type       = bs_si_.GetBits_9(2);
        ci.mixed_block_flag = bs_si_.Get_1_Bit();
        ci.table_select[0]  = bs_si_.GetBits_9(5);
        ci.table_select[1]  = bs_si_.GetBits_9(5);
        ci.subblock_gain[0] = bs_si_.GetBits_9(3);
        ci.subblock_gain[1] = bs_si_.GetBits_9(3);
        ci.subblock_gain[2] = bs_si_.GetBits_9(3);
        if (ci.block_type == 0)
          return -1;
        else if (ci.block_type == 2 && ci.mixed_block_flag == 0)
          ci.region0_count = 8;
        else {
          ci.region0_count = 7;
          ci.region1_count = 20 - ci.region0_count;
        }
      } else {
        ci.table_select[0]  = bs_si_.GetBits_9(5);
        ci.table_select[1]  = bs_si_.GetBits_9(5);
        ci.table_select[2]  = bs_si_.GetBits_9(5);
        ci.region0_count    = bs_si_.GetBits_9(4);
        ci.region1_count    = bs_si_.GetBits_9(3);
        ci.block_type       = 0;
        ci.mixed_block_flag = 0;
      }
      ci.scalefac_scale     = bs_si_.Get_1_Bit();
      ci.count1table_select = bs_si_.Get_1_Bit();
    }
  }

  return off + header_.GetSideInfoSize();
}
void LayerIII::GetScaleFactors_2(int gr, int ch) {
  uint8_t i, band, slen, num, n = 0, scf = 0;
  bool    i_stereo = header_.IsIntensityStereo();
  auto&   ci       = channel_info_[gr][ch];
  auto&   l        = scalefacLong[ch];
  auto&   s        = scalefacShort[ch];

  rzeroBandLong = 0;
  if ((ch > 0) && i_stereo)
    slen = i_slen2[ci.scalefac_compress >> 1];
  else
    slen = n_slen2[ci.scalefac_compress];

  ci.preflag      = (slen >> 15) & 0x1;
  ci.part2_length = 0;
  if (ci.block_type == 2) {
    n++;
    if ((ci.mixed_block_flag) != 0) n++;
    auto nr = nr_of_sfb[n][(slen >> 12) & 0x7];

    for (i = 0; i < 4; i++) {
      num = slen & 0x7;
      slen >>= 3;
      if (num != 0) {
        for (band = 0; band < nr[i]; band++)
          s[scf++] = main_data_stream_->GetBits_17(num);
        ci.part2_length += nr[i] * num;
      } else
        for (band = 0; band < nr[i]; band++) s[scf++] = 0;
    }

    n = (n << 1) + 1;
    for (i = 0; i < n; i++) s[scf++] = 0;
  } else {
    auto nr = nr_of_sfb[n][(slen >> 12) & 0x7];
    for (i = 0; i < 4; i++) {
      num = slen & 0x7;
      slen >>= 3;
      if (num != 0) {
        for (band = 0; band < nr[i]; band++)
          l[scf++] = main_data_stream_->GetBits_17(num);
        ci.part2_length += nr[i] * num;
      } else
        for (band = 0; band < nr[i]; band++) l[scf++] = 0;
    }

    n = (n << 1) + 1;
    for (i = 0; i < n; i++) l[scf++] = 0;
  }
}
void LayerIII::GetScaleFactors_1(int gr, int ch) {
  auto& ci   = channel_info_[gr][ch];
  int   len0 = slen0[ci.scalefac_compress];
  int   len1 = slen1[ci.scalefac_compress];
  auto& l    = scalefacLong[ch];
  auto& s    = scalefacShort[ch];
  int   scf;

  ci.part2_length = 0;

  if (ci.window_switching_flag != 0 && ci.block_type == 2) {
    if (ci.mixed_block_flag != 0) {
      // MIXED block
      ci.part2_length = 17 * len0 + 18 * len1;
      for (scf = 0; scf < 8; scf++) l[scf] = main_data_stream_->GetBits_9(len0);
      for (scf = 9; scf < 18; scf++)
        s[scf] = main_data_stream_->GetBits_9(len0);
      for (scf = 18; scf < 36; scf++)
        s[scf] = main_data_stream_->GetBits_9(len1);
    } else {
      // pure SHORT block
      ci.part2_length = 18 * (len0 + len1);
      for (scf = 0; scf < 18; scf++)
        s[scf] = main_data_stream_->GetBits_9(len0);
      for (scf = 18; scf < 36; scf++)
        s[scf] = main_data_stream_->GetBits_9(len1);
    }
  } else {
    // LONG types 0,1,3
    int k = scfsi_[ch];
    if (gr == 0) {
      ci.part2_length = 10 * (len0 + len1) + len0;
      for (scf = 0; scf < 11; scf++)
        l[scf] = main_data_stream_->GetBits_9(len0);
      for (scf = 11; scf < 21; scf++)
        l[scf] = main_data_stream_->GetBits_9(len1);
    } else {
      ci.part2_length = 0;
      if ((k & 8) == 0) {
        for (scf = 0; scf < 6; scf++)
          l[scf] = main_data_stream_->GetBits_9(len0);
        ci.part2_length += 6 * len0;
      }
      if ((k & 4) == 0) {
        for (scf = 6; scf < 11; scf++)
          l[scf] = main_data_stream_->GetBits_9(len0);
        ci.part2_length += 5 * len0;
      }
      if ((k & 2) == 0) {
        for (scf = 11; scf < 16; scf++)
          l[scf] = main_data_stream_->GetBits_9(len1);
        ci.part2_length += 5 * len1;
      }
      if ((k & 1) == 0) {
        for (scf = 16; scf < 21; scf++)
          l[scf] = main_data_stream_->GetBits_9(len1);
        ci.part2_length += 5 * len1;
      }
    }
  }
}
void LayerIII::huffBits(int gr, int ch) {
  ChannelInformation ci = channel_info_[gr][ch];
  int                r1, r2;

  if (ci.window_switching_flag != 0) {
    int ver = header_.GetVersion();
    if (ver == static_cast<int>(MPEGVersion::kMPEG1)
        || (ver == static_cast<int>(MPEGVersion::kMPEG2)
            && ci.block_type == 2)) {
      ci.region1Start = 36;
      ci.region2Start = 576;
    } else {
      if (ver == static_cast<int>(MPEGVersion::kMPEG25)) {
        if (ci.block_type == 2 && ci.mixed_block_flag == 0)
          ci.region1Start = sfb_index_long_[6];
        else
          ci.region1Start = sfb_index_long_[8];
        ci.region2Start = 576;
      } else {
        ci.region1Start = 54;
        ci.region2Start = 576;
      }
    }
  } else {
    r1 = ci.region0_count + 1;
    r2 = r1 + ci.region1_count + 1;
    if (r2 > static_cast<int>(sfb_index_long_.size() - 1))
      r2 = sfb_index_long_.size() - 1;
    ci.region1Start = sfb_index_long_[r1];
    ci.region2Start = sfb_index_long_[r2];
  }

  rzeroIndex[ch] = main_data_stream_->DecodeHuff(ci, hv);  // 哈夫曼解码
}
void LayerIII::Requantizer(int gr, int ch, std::array<float, 32 * 18>& xrch) {
  auto               l       = scalefacLong[ch];
  ChannelInformation ci      = channel_info_[gr][ch];
  bool               preflag = ci.preflag == 1;
  int                shift   = 1 + ci.scalefac_scale;
  int                maxi    = rzeroIndex[ch];
  float              requVal;
  int bi = 0, sfb = 0, width, pre, val, hvIdx = 0, xri = 0, scf = 0;
  int xriStart = 0;  // 用于计算短块重排序后的下标
  int pow2i    = 255 - ci.global_gain;

  if (header_.IsMS()) pow2i += 2;  // 若声道模式为ms_stereo,要除以根2

  // pure SHORT blocks:
  // window_switching_flag=1, block_type=2, mixed_block_flag=0

  if (ci.window_switching_flag == 1 && ci.block_type == 2) {
    rzeroBandShort[0] = rzeroBandShort[1] = rzeroBandShort[2] = -1;
    if (ci.mixed_block_flag == 1) {
      /*
       * 混合块:
       * 混合块的前8个频带是长块。 前8块各用一个增益因子逆量化，这8个增益因子
       * 的频带总和为36， 这36条频率线用长块公式逆量化。
       */
      rzeroBandLong = -1;
      for (; sfb < 8; sfb++) {
        pre     = preflag ? pretab[sfb] : 0;
        requVal = floatPow2[pow2i + ((l[sfb] + pre) << shift)];
        width   = widthLong[sfb];
        for (bi = 0; bi < width; bi++) {
          val = hv[hvIdx];  // 哈夫曼值
          if (val < 0) {
            xrch[hvIdx]   = -requVal * floatPowIS[-val];
            rzeroBandLong = sfb;
          } else if (val > 0) {
            xrch[hvIdx]   = requVal * floatPowIS[val];
            rzeroBandLong = sfb;
          } else
            xrch[hvIdx] = 0;
          hvIdx++;
        }
      }

      /*
       * 混合块的后9个频带是被加窗的短块，其每一块同一窗口内3个值的增益因子频带相同。
       * 后9块增益因子对应的频率子带值为widthShort[3..11]
       */
      rzeroBandShort[0] = rzeroBandShort[1] = rzeroBandShort[2] = 2;
      rzeroBandLong++;
      sfb      = 3;
      scf      = 9;
      xriStart = 36;  // 为短块重排序准备好下标
    }

    // 短块(混合块中的短块和纯短块)
    auto s       = scalefacShort[ch];
    auto subgain = ci.subblock_gain;
    subgain[0] <<= 3;
    subgain[1] <<= 3;
    subgain[2] <<= 3;
    int win;
    for (; hvIdx < maxi; sfb++) {
      width = widthShort[sfb];
      for (win = 0; win < 3; win++) {
        requVal = floatPow2[pow2i + subgain[win] + (s[scf++] << shift)];
        xri     = xriStart + win;
        for (bi = 0; bi < width; bi++) {
          val = hv[hvIdx];
          if (val < 0) {
            xrch[xri]           = -requVal * floatPowIS[-val];
            rzeroBandShort[win] = sfb;
          } else if (val > 0) {
            xrch[xri]           = requVal * floatPowIS[val];
            rzeroBandShort[win] = sfb;
          } else
            xrch[xri] = 0;
          hvIdx++;
          xri += 3;
        }
      }
      xriStart = xri - 2;
    }
    rzeroBandShort[0]++;
    rzeroBandShort[1]++;
    rzeroBandShort[2]++;
    rzeroBandLong++;
  } else {
    // 长块
    xri = -1;
    for (; hvIdx < maxi; sfb++) {
      pre     = preflag ? pretab[sfb] : 0;
      requVal = floatPow2[pow2i + ((l[sfb] + pre) << shift)];
      bi      = hvIdx + widthLong[sfb];
      for (; hvIdx < bi; hvIdx++) {
        val = hv[hvIdx];
        if (val < 0) {
          xrch[hvIdx] = -requVal * floatPowIS[-val];
          xri         = sfb;
        } else if (val > 0) {
          xrch[hvIdx] = requVal * floatPowIS[val];
          xri         = sfb;
        } else
          xrch[hvIdx] = 0;
      }
    }
    rzeroBandLong = xri + 1;
  }

  // 不逆量化0值区,置0.
  for (; hvIdx < 576; hvIdx++) xrch[hvIdx] = 0;
}
void LayerIII::ms_stereo(int gr) {
  auto &xr0 = (*xrch0)[gr], &xr1 = (*xrch1)[gr];
  int   rzero_xr =
      (rzeroIndex[0] > rzeroIndex[1]) ? rzeroIndex[0] : rzeroIndex[1];
  int   xri;
  float tmp0, tmp1;

  for (xri = 0; xri < rzero_xr; xri++) {
    tmp0     = xr0[xri];
    tmp1     = xr1[xri];
    xr0[xri] = tmp0 + tmp1;
    xr1[xri] = tmp0 - tmp1;
  }
  rzeroIndex[0] = rzeroIndex[1] = rzero_xr;  // ...不然可能导致声音细节丢失
}
void LayerIII::is_lines_1(int pos, int idx0, int width, int step, int gr) {
  float xr0;
  for (int w = width; w > 0; w--) {
    xr0                = (*xrch0)[gr][idx0];
    (*xrch0)[gr][idx0] = xr0 * is_coef[pos];
    (*xrch1)[gr][idx0] = xr0 * is_coef[6 - pos];
    idx0 += step;
  }
}
void LayerIII::is_lines_2(int tab2, int pos, int idx0, int width, int step,
                          int gr) {
  float xr0;
  for (int w = width; w > 0; w--) {
    xr0 = (*xrch0)[gr][idx0];
    if (pos == 0)
      (*xrch1)[gr][idx0] = xr0;
    else {
      if ((pos & 1) == 0)
        (*xrch1)[gr][idx0] = xr0 * lsf_is_coef[tab2][(pos - 1) >> 1];
      else {
        (*xrch0)[gr][idx0] = xr0 * lsf_is_coef[tab2][(pos - 1) >> 1];
        (*xrch1)[gr][idx0] = xr0;
      }
    }
    idx0 += step;
  }
}
void LayerIII::intensity_stereo(int gr) {
  ChannelInformation ci = channel_info_[gr][1];  //信息保存在右声道
  int                scf, idx, sfb;
  if (channel_info_[gr][0].mixed_block_flag != ci.mixed_block_flag
      || channel_info_[gr][0].block_type != ci.block_type)
    return;

  if (is_mpeg_1_) {  // MPEG-1
    if (ci.block_type == 2) {
      // MPEG-1, short block/mixed block
      int w3;
      for (w3 = 0; w3 < 3; w3++) {
        sfb = rzeroBandShort[w3];  // 混合块sfb最小为3
        for (; sfb < 12; sfb++) {
          idx = 3 * sfb_index_short_[sfb] + w3;
          scf = scalefacShort[1][3 * sfb + w3];
          if (scf >= 7) continue;
          is_lines_1(scf, idx, widthShort[sfb], 3, gr);
        }
      }
    } else {
      // MPEG-1, long block
      for (sfb = rzeroBandLong; sfb <= 21; sfb++) {
        scf = scalefacLong[1][sfb];
        if (scf < 7)
          is_lines_1(scf, sfb_index_long_[sfb], widthLong[sfb], 1, gr);
      }
    }
  } else {  // MPEG-2
    int tab2 = ci.scalefac_compress & 0x1;
    if (ci.block_type == 2) {
      // MPEG-2, short block/mixed block
      int w3;
      for (w3 = 0; w3 < 3; w3++) {
        sfb = rzeroBandShort[w3];  // 混合块sfb最小为3
        for (; sfb < 12; sfb++) {
          idx = 3 * sfb_index_short_[sfb] + w3;
          scf = scalefacShort[1][3 * sfb + w3];
          is_lines_2(tab2, scf, idx, widthShort[sfb], 3, gr);
        }
      }
    } else {
      // MPEG-2, long block
      for (sfb = rzeroBandLong; sfb <= 21; sfb++)
        is_lines_2(tab2, scalefacLong[1][sfb], sfb_index_long_[sfb],
                   widthLong[sfb], 1, gr);
    }
  }
}
void LayerIII::antialias(int gr, int ch, std::array<float, 32 * 18>& xrch) {
  auto& xr = xrch;
  int   i, maxidx;
  float bu, bd;

  if (channel_info_[gr][ch].block_type == 2) {
    if (channel_info_[gr][ch].mixed_block_flag == 0) return;
    maxidx = 18;
  } else
    maxidx = rzeroIndex[ch] - 18;

  for (i = 0; i < maxidx; i += 18) {
    bu         = xr[i + 17];
    bd         = xr[i + 18];
    xr[i + 17] = bu * 0.85749293f + bd * 0.51449576f;
    xr[i + 18] = bd * 0.85749293f - bu * 0.51449576f;
    bu         = xr[i + 16];
    bd         = xr[i + 19];
    xr[i + 16] = bu * 0.8817420f + bd * 0.47173197f;
    xr[i + 19] = bd * 0.8817420f - bu * 0.47173197f;
    bu         = xr[i + 15];
    bd         = xr[i + 20];
    xr[i + 15] = bu * 0.94962865f + bd * 0.31337745f;
    xr[i + 20] = bd * 0.94962865f - bu * 0.31337745f;
    bu         = xr[i + 14];
    bd         = xr[i + 21];
    xr[i + 14] = bu * 0.98331459f + bd * 0.18191320f;
    xr[i + 21] = bd * 0.98331459f - bu * 0.18191320f;
    bu         = xr[i + 13];
    bd         = xr[i + 22];
    xr[i + 13] = bu * 0.99551782f + bd * 0.09457419f;
    xr[i + 22] = bd * 0.99551782f - bu * 0.09457419f;
    bu         = xr[i + 12];
    bd         = xr[i + 23];
    xr[i + 12] = bu * 0.99916056f + bd * 0.04096558f;
    xr[i + 23] = bd * 0.99916056f - bu * 0.04096558f;
    bu         = xr[i + 11];
    bd         = xr[i + 24];
    xr[i + 11] = bu * 0.99989920f + bd * 0.0141986f;
    xr[i + 24] = bd * 0.99989920f - bu * 0.0141986f;
    bu         = xr[i + 10];
    bd         = xr[i + 25];
    xr[i + 10] = bu * 0.99999316f + bd * 3.69997467e-3f;
    xr[i + 25] = bd * 0.99999316f - bu * 3.69997467e-3f;
  }
}
void LayerIII::imdct12(std::array<float, 32 * 18>& xrch,
                       std::array<float, 32 * 18>& pre, int off) {
  auto& io = xrch;
  int   i, j;
  float in1, in2, in3, in4;
  float out0, out1, out2, out3, out4, out5, tmp;
  float out6 = 0, out7 = 0, out8 = 0, out9 = 0, out10 = 0, out11 = 0;
  float out12 = 0, out13 = 0, out14 = 0, out15 = 0, out16 = 0, out17 = 0;
  float f0 = 0, f1 = 0, f2 = 0, f3 = 0, f4 = 0, f5 = 0;

  for (j = 0; j != 3; j++) {
    i = j + off;
    //>>>>>>>>>>>> 12-point IMDCT
    //>>>>>> 6-point IDCT
    io[15 + i] += (io[12 + i] += io[9 + i]) + io[6 + i];
    io[9 + i] += (io[6 + i] += io[3 + i]) + io[i];
    io[3 + i] += io[i];

    //>>> 3-point IDCT on even
    out1 = (in1 = io[i]) - (in2 = io[12 + i]);
    in3  = in1 + in2 * 0.5f;
    in4  = io[6 + i] * 0.8660254f;
    out0 = in3 + in4;
    out2 = in3 - in4;
    //<<< End 3-point IDCT on even

    //>>> 3-point IDCT on odd (for 6-point IDCT)
    out4 = ((in1 = io[3 + i]) - (in2 = io[15 + i])) * 0.7071068f;
    in3  = in1 + in2 * 0.5f;
    in4  = io[9 + i] * 0.8660254f;
    out5 = (in3 + in4) * 0.5176381f;
    out3 = (in3 - in4) * 1.9318516f;
    //<<< End 3-point IDCT on odd

    // Output: butterflies on 2,3-point IDCT's (for 6-point IDCT)
    tmp = out0;
    out0 += out5;
    out5 = tmp - out5;
    tmp  = out1;
    out1 += out4;
    out4 = tmp - out4;
    tmp  = out2;
    out2 += out3;
    out3 = tmp - out3;
    //<<<<<< End 6-point IDCT
    //<<<<<<<<<<<< End 12-point IDCT

    tmp = out3 * 0.1072064f;
    switch (j) {
      case 0:
        out6  = tmp;
        out7  = out4 * 0.5f;
        out8  = out5 * 2.3319512f;
        out9  = -out5 * 3.0390580f;
        out10 = -out4 * 1.2071068f;
        out11 = -tmp * 7.5957541f;

        f0 = out2 * 0.6248445f;
        f1 = out1 * 0.5f;
        f2 = out0 * 0.4000996f;
        f3 = out0 * 0.3070072f;
        f4 = out1 * 0.2071068f;
        f5 = out2 * 0.0822623f;
        break;
      case 1:
        out12 = tmp - f0;
        out13 = out4 * 0.5f - f1;
        out14 = out5 * 2.3319512f - f2;
        out15 = -out5 * 3.0390580f - f3;
        out16 = -out4 * 1.2071068f - f4;
        out17 = -tmp * 7.5957541f - f5;

        f0 = out2 * 0.6248445f;
        f1 = out1 * 0.5f;
        f2 = out0 * 0.4000996f;
        f3 = out0 * 0.3070072f;
        f4 = out1 * 0.2071068f;
        f5 = out2 * 0.0822623f;
        break;
      case 2:
        // output
        i          = off;
        io[i + 0]  = pre[i + 0];
        io[i + 1]  = pre[i + 1];
        io[i + 2]  = pre[i + 2];
        io[i + 3]  = pre[i + 3];
        io[i + 4]  = pre[i + 4];
        io[i + 5]  = pre[i + 5];
        io[i + 6]  = pre[i + 6] + out6;
        io[i + 7]  = pre[i + 7] + out7;
        io[i + 8]  = pre[i + 8] + out8;
        io[i + 9]  = pre[i + 9] + out9;
        io[i + 10] = pre[i + 10] + out10;
        io[i + 11] = pre[i + 11] + out11;
        io[i + 12] = pre[i + 12] + out12;
        io[i + 13] = pre[i + 13] + out13;
        io[i + 14] = pre[i + 14] + out14;
        io[i + 15] = pre[i + 15] + out15;
        io[i + 16] = pre[i + 16] + out16;
        io[i + 17] = pre[i + 17] + out17;

        pre[i + 0]  = tmp - f0;
        pre[i + 1]  = out4 * 0.5f - f1;
        pre[i + 2]  = out5 * 2.3319512f - f2;
        pre[i + 3]  = -out5 * 3.0390580f - f3;
        pre[i + 4]  = -out4 * 1.2071068f - f4;
        pre[i + 5]  = -tmp * 7.5957541f - f5;
        pre[i + 6]  = -out2 * 0.6248445f;
        pre[i + 7]  = -out1 * 0.5f;
        pre[i + 8]  = -out0 * 0.4000996f;
        pre[i + 9]  = -out0 * 0.3070072f;
        pre[i + 10] = -out1 * 0.2071068f;
        pre[i + 11] = -out2 * 0.0822623f;
        pre[i + 12] = pre[i + 13] = pre[i + 14] = 0;
        pre[i + 15] = pre[i + 16] = pre[i + 17] = 0;
    }
  }
}
void LayerIII::imdct36(std::array<float, 32 * 18>& xrch,
                       std::array<float, 32 * 18>& preBlck, int off,
                       int block_type) {
  auto& io  = xrch;
  auto& pre = preBlck;
  int   i   = off;
  float in0, in1, in2, in3, in4, in5, in6, in7, in8, in9, in10, in11;
  float in12, in13, in14, in15, in16, in17;
  float out0, out1, out2, out3, out4, out5, out6, out7, out8, out9;
  float out10, out11, out12, out13, out14, out15, out16, out17, tmp;

  //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 36-point IDCT
  //>>>>>>>>>>>>>>>>>> 18-point IDCT for odd
  io[i + 17] += (io[i + 16] += io[i + 15]) + io[i + 14];
  io[i + 15] += (io[i + 14] += io[i + 13]) + io[i + 12];
  io[i + 13] += (io[i + 12] += io[i + 11]) + io[i + 10];
  io[i + 11] += (io[i + 10] += io[i + 9]) + io[i + 8];
  io[i + 9] += (io[i + 8] += io[i + 7]) + io[i + 6];
  io[i + 7] += (io[i + 6] += io[i + 5]) + io[i + 4];
  io[i + 5] += (io[i + 4] += io[i + 3]) + io[i + 2];
  io[i + 3] += (io[i + 2] += io[i + 1]) + io[i + 0];
  io[i + 1] += io[i + 0];

  //>>>>>>>>> 9-point IDCT on even
  /*
   *  for(m = 0; m < 9; m++) {
   *      sum = 0;
   *      for(n = 0; n < 18; n += 2)
   *          sum += in[n] * cos(PI36 * (2 * m + 1) * n);
   *      out18[m] = sum;
   *  }
   */
  in0 = io[i + 0] + io[i + 12] * 0.5f;
  in1 = io[i + 0] - io[i + 12];
  in2 = io[i + 8] + io[i + 16] - io[i + 4];

  out4 = in1 + in2;

  in3 = in1 - in2 * 0.5f;
  in4 = (io[i + 10] + io[i + 14] - io[i + 2]) * 0.8660254f;  // cos(PI/6)

  out1 = in3 - in4;
  out7 = in3 + in4;

  in5 = (io[i + 4] + io[i + 8]) * 0.9396926f;    // cos( PI/9)
  in6 = (io[i + 16] - io[i + 8]) * 0.1736482f;   // cos(4PI/9)
  in7 = -(io[i + 4] + io[i + 16]) * 0.7660444f;  // cos(2PI/9)

  in17 = in0 - in5 - in7;
  in8  = in5 + in0 + in6;
  in9  = in0 + in7 - in6;

  in12 = io[i + 6] * 0.8660254f;                  // cos(PI/6)
  in10 = (io[i + 2] + io[i + 10]) * 0.9848078f;   // cos(PI/18)
  in11 = (io[i + 14] - io[i + 10]) * 0.3420201f;  // cos(7PI/18)

  in13 = in10 + in11 + in12;

  out0 = in8 + in13;
  out8 = in8 - in13;

  in14 = -(io[i + 2] + io[i + 14]) * 0.6427876f;  // cos(5PI/18)
  in15 = in10 + in14 - in12;
  in16 = in11 - in14 - in12;

  out3 = in9 + in15;
  out5 = in9 - in15;

  out2 = in17 + in16;
  out6 = in17 - in16;
  //<<<<<<<<< End 9-point IDCT on even

  //>>>>>>>>> 9-point IDCT on odd
  /*
   *  for(m = 0; m < 9; m++) {
   *      sum = 0;
   *      for(n = 0;n < 18; n += 2)
   *          sum += in[n + 1] * cos(PI36 * (2 * m + 1) * n);
   *      out18[17-m] = sum;
   * }
   */
  in0 = io[i + 1] + io[i + 13] * 0.5f;  // cos(PI/3)
  in1 = io[i + 1] - io[i + 13];
  in2 = io[i + 9] + io[i + 17] - io[i + 5];

  out13 = (in1 + in2) * 0.7071068f;  // cos(PI/4)

  in3 = in1 - in2 * 0.5f;
  in4 = (io[i + 11] + io[i + 15] - io[i + 3]) * 0.8660254f;  // cos(PI/6)

  out16 = (in3 - in4) * 0.5176381f;  // 0.5/cos( PI/12)
  out10 = (in3 + in4) * 1.9318517f;  // 0.5/cos(5PI/12)

  in5 = (io[i + 5] + io[i + 9]) * 0.9396926f;    // cos( PI/9)
  in6 = (io[i + 17] - io[i + 9]) * 0.1736482f;   // cos(4PI/9)
  in7 = -(io[i + 5] + io[i + 17]) * 0.7660444f;  // cos(2PI/9)

  in17 = in0 - in5 - in7;
  in8  = in5 + in0 + in6;
  in9  = in0 + in7 - in6;

  in12 = io[i + 7] * 0.8660254f;                  // cos(PI/6)
  in10 = (io[i + 3] + io[i + 11]) * 0.9848078f;   // cos(PI/18)
  in11 = (io[i + 15] - io[i + 11]) * 0.3420201f;  // cos(7PI/18)

  in13 = in10 + in11 + in12;

  out17 = (in8 + in13) * 0.5019099f;  // 0.5/cos(PI/36)
  out9  = (in8 - in13) * 5.7368566f;  // 0.5/cos(17PI/36)

  in14 = -(io[i + 3] + io[i + 15]) * 0.6427876f;  // cos(5PI/18)
  in15 = in10 + in14 - in12;
  in16 = in11 - in14 - in12;

  out14 = (in9 + in15) * 0.6103873f;  // 0.5/cos(7PI/36)
  out12 = (in9 - in15) * 0.8717234f;  // 0.5/cos(11PI/36)

  out15 = (in17 + in16) * 0.5516890f;  // 0.5/cos(5PI/36)
  out11 = (in17 - in16) * 1.1831008f;  // 0.5/cos(13PI/36)
  //<<<<<<<<< End. 9-point IDCT on odd

  // Butterflies on 9-point IDCT's
  tmp = out0;
  out0 += out17;
  out17 = tmp - out17;
  tmp   = out1;
  out1 += out16;
  out16 = tmp - out16;
  tmp   = out2;
  out2 += out15;
  out15 = tmp - out15;
  tmp   = out3;
  out3 += out14;
  out14 = tmp - out14;
  tmp   = out4;
  out4 += out13;
  out13 = tmp - out13;
  tmp   = out5;
  out5 += out12;
  out12 = tmp - out12;
  tmp   = out6;
  out6 += out11;
  out11 = tmp - out11;
  tmp   = out7;
  out7 += out10;
  out10 = tmp - out10;
  tmp   = out8;
  out8 += out9;
  out9 = tmp - out9;
  //<<<<<<<<<<<<<<<<<< End of 18-point IDCT
  //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< End of 36-point IDCT

  // output
  auto const &win = kIMDCTWin[block_type];

  io[i + 0]  = pre[i + 0] + out9 * win[0];
  io[i + 1]  = pre[i + 1] + out10 * win[1];
  io[i + 2]  = pre[i + 2] + out11 * win[2];
  io[i + 3]  = pre[i + 3] + out12 * win[3];
  io[i + 4]  = pre[i + 4] + out13 * win[4];
  io[i + 5]  = pre[i + 5] + out14 * win[5];
  io[i + 6]  = pre[i + 6] + out15 * win[6];
  io[i + 7]  = pre[i + 7] + out16 * win[7];
  io[i + 8]  = pre[i + 8] + out17 * win[8];
  io[i + 9]  = pre[i + 9] + out17 * win[9];
  io[i + 10] = pre[i + 10] + out16 * win[10];
  io[i + 11] = pre[i + 11] + out15 * win[11];
  io[i + 12] = pre[i + 12] + out14 * win[12];
  io[i + 13] = pre[i + 13] + out13 * win[13];
  io[i + 14] = pre[i + 14] + out12 * win[14];
  io[i + 15] = pre[i + 15] + out11 * win[15];
  io[i + 16] = pre[i + 16] + out10 * win[16];
  io[i + 17] = pre[i + 17] + out9 * win[17];

  pre[i + 0]  = out8 * win[18];
  pre[i + 1]  = out7 * win[19];
  pre[i + 2]  = out6 * win[20];
  pre[i + 3]  = out5 * win[21];
  pre[i + 4]  = out4 * win[22];
  pre[i + 5]  = out3 * win[23];
  pre[i + 6]  = out2 * win[24];
  pre[i + 7]  = out1 * win[25];
  pre[i + 8]  = out0 * win[26];
  pre[i + 9]  = out0 * win[27];
  pre[i + 10] = out1 * win[28];
  pre[i + 11] = out2 * win[29];
  pre[i + 12] = out3 * win[30];
  pre[i + 13] = out4 * win[31];
  pre[i + 14] = out5 * win[32];
  pre[i + 15] = out6 * win[33];
  pre[i + 16] = out7 * win[34];
  pre[i + 17] = out8 * win[35];
}
void LayerIII::hybrid(int gr, int ch, std::array<float, 32 * 18>& xrch,
                      std::array<float, 32 * 18>& preb) {
  auto& ci   = channel_info_[gr][ch];
  int   maxi = rzeroIndex[ch];
  int   i, block_type;

  for (i = 0; i < maxi; i += 18) {
    block_type = ((ci.window_switching_flag != 0) && (ci.mixed_block_flag != 0)
                  && (i < 36))
                     ? 0
                     : ci.block_type;

    if (block_type == 2)
      imdct12(xrch, preb, i);
    else
      imdct36(xrch, preb, i, block_type);
  }

  // 0值区
  for (; i < 576; i++) {
    xrch[i] = preb[i];
    preb[i] = 0;
  }
}
int LayerIII::DecodeFrame(std::vector<uint8_t> const& b, int off) {
  /*
   * part1 : side information
   */
  int i = GetSideInfo(b, off);
  if (i < 0) return off + header_.GetFrameSize() - 4;  // 跳过这一帧
  off = i;

  /*
   * part2_3: scale factors + huffman bits
   * length: ((part2_3_bits + 7) >> 3) bytes
   */
  int maindataSize = header_.GetMainDataSize();
  int bufSize      = main_data_stream_->GetSize();
  if (bufSize < main_data_begin_) {
    // 若出错，不解码当前这一帧， 将主数据(main_data)填入位流缓冲区后返回，
    // 在解码下一帧时全部或部分利用填入的这些主数据。
    main_data_stream_->Append(b, off, maindataSize);
    return off + maindataSize;
  }

  // 丢弃上一帧的填充位
  int discard = bufSize - main_data_stream_->GetBytePos() - main_data_begin_;
  main_data_stream_->SkipBytes(discard);

  // 主数据添加到位流缓冲区
  main_data_stream_->Append(b, off, maindataSize);
  off += maindataSize;
  // main_data_stream_->mark();//----debug

  for (int gr = 0; gr < granules_; gr++) {
    if (is_mpeg_1_)
      GetScaleFactors_1(gr, 0);
    else
      GetScaleFactors_2(gr, 0);
    huffBits(gr, 0);
    Requantizer(gr, 0, (*xrch0)[gr]);

    if (channels_ == 2) {
      if (is_mpeg_1_)
        GetScaleFactors_1(gr, 1);
      else
        GetScaleFactors_2(gr, 1);
      huffBits(gr, 1);
      Requantizer(gr, 1, (*xrch1)[gr]);

      if (header_.IsMS()) ms_stereo(gr);
      if (header_.IsIntensityStereo()) intensity_stereo(gr);
    }

    antialias(gr, 0, (*xrch0)[gr]);
    hybrid(gr, 0, (*xrch0)[gr], preBlckCh0);

    if (channels_ == 2) {
      antialias(gr, 1, (*xrch1)[gr]);
      hybrid(gr, 1, (*xrch1)[gr], preBlckCh1);
    }
  }
  // int part2_3_bytes = main_data_stream_->getMark();//----debug
  // 可以在这调用main_data_stream_->skipBits(part2_3_bits & 7)丢弃填充位，
  // 更好的方法是放在解码下一帧主数据之前处理，如果位流错误，可以顺便纠正。

  try {
    std::unique_lock lock{notifier_mutex_};
    while (semaphore_ < channels_)  // 等待上一帧channels个声道完成多相合成滤波
      notifier_.wait(lock);
    semaphore_ = 0;  // 信号量置0
                     //实现播放
  } catch (std::exception& e) {
    Close();
    return off;
  }
  OutputAudio();

  // 异步多相合成滤波
  xrch0 = filter_ch_0_.StartSynthesis();
  if (channels_ == 2) xrch1 = filter_ch_1.StartSynthesis();
  return off;
}
void LayerIII::Close() {
  std::lock_guard lock{notifier_mutex_};
  semaphore_ = channels_;
  notifier_.notify_one();

  LayerI_II_III::Close();

  filter_ch_0_.Shutdown();
  if (channels_ == 2) filter_ch_1.Shutdown();
}

void LayerIII::SubmitSynthesis() {
  std::lock_guard lock{notifier_mutex_};
  if (++semaphore_ == channels_) notifier_.notify_one();
}

}  // namespace jmp123::decoder
