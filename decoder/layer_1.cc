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

#include "layer_1.h"

namespace jmp123::decoder {
float LayerI::Requantization(int ch, int sb, int nb) {
  int   sample_code = bs_->GetBits_17(nb);
  int   n_levels    = 1 << nb;
  float requ =
      2.0f * static_cast<float>(sample_code) / static_cast<float>(n_levels)
      - 1.0f;  // s'''
  requ += std::pow(2, 1 - nb);
  requ *=
      static_cast<float>(n_levels) / static_cast<float>(n_levels - 1);  // s''
  requ *= kFactor[scale_factor_[ch][sb]];                               // s'
  return requ;
}
int LayerI::DecodeFrame(std::vector<uint8_t> const &b, int off) {
  int sb = 0, gr = 0, ch = 0, nb = 0;
  int nch = header_.GetChannelCount();
  int bound =
      (header_.GetMode() == 1) ? ((header_.GetModeExtension() + 1) * 4) : 32;
  int main_data_bytes = header_.GetMainDataSize();
  if (bs_->Append(b, off, main_data_bytes) < main_data_bytes) return -1;
  off += main_data_bytes;
  int main_data_begin = bs_->GetBytePos();

  // 1. Bit allocation decoding
  for (sb = 0; sb < bound; sb++)
    for (ch = 0; ch < nch; ++ch) {
      nb = bs_->GetBits_9(4);
      if (nb == 15) return -2;
      allocation_[ch][sb] = (nb != 0) ? (nb + 1) : 0;
    }
  for (sb = bound; sb < 32; sb++) {
    nb = bs_->GetBits_9(4);
    if (nb == 15) return -2;
    allocation_[0][sb] = (nb != 0) ? (nb + 1) : 0;
  }

  // 2. Scalefactor decoding
  for (sb = 0; sb < 32; sb++)
    for (ch = 0; ch < nch; ch++)
      if (allocation_[ch][sb] != 0) scale_factor_[ch][sb] = bs_->GetBits_9(6);

  for (gr = 0; gr < 12; gr++) {
    // 3. Requantization of subband samples
    for (sb = 0; sb < bound; sb++)
      for (ch = 0; ch < nch; ch++) {
        nb = allocation_[ch][sb];
        if (nb == 0)
          syin_[ch][sb] = 0;
        else
          syin_[ch][sb] = Requantization(ch, sb, nb);
      }
    // mode=1(Joint Stereo)
    for (sb = bound; sb < 32; sb++)
      if ((nb = allocation_[0][sb]) != 0)
        for (ch = 0; ch < nch; ch++) syin_[ch][sb] = Requantization(ch, sb, nb);
      else
        for (ch = 0; ch < nch; ch++) syin_[ch][sb] = 0;

    // 4. Synthesis subband filter
    for (ch = 0; ch < nch; ch++) filter_.SynthesisSubBand(syin_[ch], ch);
  }
  // 5. Ancillary bits
  int discard = main_data_bytes + main_data_begin - bs_->GetBytePos();
  bs_->SkipBytes(discard);

  // 6. output
  OutputAudio();

  return off;
}
}  // namespace jmp123::decoder