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

#include "layer_2.h"
namespace jmp123::decoder{
LayerII::LayerII(Header h, std::unique_ptr<IAudio> audio)
    : LayerI_II_III(h, std::move(audio)),
      header_(std::move(h)),
      channels_(h.GetChannelCount()),
      bs_(std::make_unique<BitStream>(4096, 512)) {
  // aidx, sblimit
  if (header_.GetVersion() == static_cast<int>(MPEGVersion::kMPEG2)) {
    aidx_     = 4;
    sb_limit_ = 30;
  } else {
    aidx_ = kAIDXTable[header_.GetSamplingFrequency()][2 - channels_]
    [header_.GetBitrateIndex()];
    if (aidx_ == 0)
      sb_limit_ = 27;
    else if (aidx_ == 1)
      sb_limit_ = 30;
    else if (aidx_ == 2)
      sb_limit_ = 8;
    else
      sb_limit_ = 12;
  }
}
void LayerII::Requantization(int index, int gr, int ch, int sb) {
  int nb = 0, s = 0, c = 0;
  int n_levels = kCQ_Steps[index];
  if ((nb = kGroup[index]) != 0) {
    c = bs_->GetBits_17(kCQ_Bits[index]);
    for (s = 0; s < 3; ++s) {
      sample_code_[s] = c % n_levels;
      c /= n_levels;
    }
    n_levels = (1 << nb) - 1; // 用于计算fractional
  }
  else {
    nb = kCQ_Bits[index];
    for (s = 0; s < 3; ++s) {
      sample_code_[s] = bs_->GetBits_17(nb);
    }
  }

  for (s = 0; s < 3; ++s) {
    float fractional = 2.0f * sample_code_[s] / static_cast<float>(n_levels + 1) -1.f;
    // s'' = C * (s''' + D)
    syin_[ch][s][sb] = kCQ_C[index] * (fractional + kCQ_D[index]);
    // s' = factor * s''
    syin_[ch][s][sb] *= kFactor[scale_factor_[ch][sb][gr >> 2]];
  }
}
void LayerII::Stereo(int index, int gr, int sb) {
  int nb = 0, s = 0, c = 0;
  int n_levels = kCQ_Steps[index];
  if ((nb = kGroup[index]) != 0) {
    c = bs_->GetBits_17(kCQ_Bits[index]);
    for (s = 0; s < 3; ++s) {
      sample_code_[s] = c % n_levels;
      c /= n_levels;
    }
    n_levels = (1 << nb) - 1;
  }
  else {
    nb = kCQ_Bits[index];
    for (s = 0; s < 3 ; ++s ){
      sample_code_[s] = bs_->GetBits_17(nb);
    }
  }

  for (s = 0; s < 3; ++s) {
    float fractional = 2.0f * sample_code_[s] / static_cast<float>(n_levels + 1) - 1.f;
    // s'' = C * (s''' + D)
    syin_[0][s][sb] = syin_[1][s][sb] = kCQ_C[index] * (fractional + kCQ_D[index]);
    // s' = factor * s''
    syin_[0][s][sb] *= kFactor[scale_factor_[0][sb][gr << 2]];
    syin_[1][s][sb] *= kFactor[scale_factor_[1][sb][gr << 2]];
  }
}
int LayerII::DecodeFrame(std::vector<uint8_t> const &b, int off) {
  int main_data_begin = 0, bound = 0, sb = 0, ch = 0;
  int main_data_bytes = header_.GetMainDataSize();
  if (bs_->Append(b, off, main_data_bytes) < main_data_bytes)
    return off + main_data_bytes;  // skip
  off += main_data_bytes;
  main_data_begin = bs_->GetBytePos();
  bound =
      (header_.GetMode() == 1) ? ((header_.GetModeExtension() + 1) << 2) : 32;
  if (bound > sb_limit_) bound = sb_limit_;

  /*
   * 1. Bit allocation decoding
   */
  for (sb = 0; sb < bound; sb++)
    for (ch = 0; ch < channels_; ch++)
      allocation_[ch][sb] = bs_->GetBits_9(kNBal[aidx_][sb]);  // 2..4 bits
  for (sb = bound; sb < sb_limit_; sb++)
    allocation_[1][sb] = allocation_[0][sb] =
        bs_->GetBits_9(kNBal[aidx_][sb]);

  /*
   * 2. Scalefactor selection information decoding
   */
  for (sb = 0; sb < sb_limit_; sb++)
    for (ch = 0; ch < channels_; ch++)
      if (allocation_[ch][sb] != 0)
        scfsi_[ch][sb] = bs_->GetBits_9(2);
      else
        scfsi_[ch][sb] = 0;

  /*
   * 3. Scalefactor decoding
   */
  for (sb = 0; sb < sb_limit_; ++sb)
    for (ch = 0; ch < channels_; ++ch)
      if (allocation_[ch][sb] != 0) {
        scale_factor_[ch][sb][0] = bs_->GetBits_9(6);
        switch (scfsi_[ch][sb]) {
          case 2:
            scale_factor_[ch][sb][2] = scale_factor_[ch][sb][1] =
                scale_factor_[ch][sb][0];
            break;
          case 0:
            scale_factor_[ch][sb][1] = bs_->GetBits_9(6);
          case 1:
          case 3:
            scale_factor_[ch][sb][2] = bs_->GetBits_9(6);
        }
        if ((scfsi_[ch][sb] & 1) == 1)
          scale_factor_[ch][sb][1] =
              scale_factor_[ch][sb][scfsi_[ch][sb] - 1];
      }

  int gr, index, s;
  for (gr = 0; gr < 12; gr++) {
    /*
     * 4. Requantization of subband samples
     */
    for (sb = 0; sb < bound; sb++)
      for (ch = 0; ch < channels_; ch++)
        if ((index = allocation_[ch][sb]) != 0) {
          index = kOffsetTable[kBitAlloc_Offset[kSBQuant_Offset[aidx_][sb]]]
          [index - 1];
          Requantization(index, gr, ch, sb);
        } else
          syin_[ch][0][sb] = syin_[ch][1][sb] = syin_[ch][2][sb] = 0;

    // mode=1(Joint Stereo)
    for (sb = bound; sb < sb_limit_; sb++)
      if ((index = allocation_[0][sb]) != 0) {
        index = kOffsetTable[kBitAlloc_Offset[kSBQuant_Offset[aidx_][sb]]]
        [index - 1];
        Stereo(index, gr, sb);
      } else
        for (ch = 0; ch < channels_; ch++)
          syin_[ch][0][sb] = syin_[ch][1][sb] = syin_[ch][2][sb] = 0;

    for (ch = 0; ch < channels_; ch++)
      for (s = 0; s < 3; s++)
        for (sb = sb_limit_; sb < 32; sb++) syin_[ch][s][sb] = 0;

    /*
     * 5. Synthesis subband filter
     */
    for (ch = 0; ch < channels_; ch++)
      for (s = 0; s < 3; s++) filter_.SynthesisSubBand(syin_[ch][s], ch);
  }  // for(gr...)

  /*
   * 6. Ancillary bits
   */
  int discard = main_data_bytes + main_data_begin - bs_->GetBytePos();
  bs_->SkipBytes(discard);

  /*
   * 7. output super.OutputAudio();
   */
  OutputAudio();

  return off;
}
}
