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

#ifndef JMP123_LAYER_2_H
#define JMP123_LAYER_2_H

#include <array>
#include <memory>

#include "bit_stream.h"
#include "header.h"
#include "layer123.h"

namespace jmp123::decoder {
class LayerII : LayerI_II_III {
 private:
  Header                                 header_;
  std::unique_ptr<BitStream>             bs_;
  int                                    channels_, aidx_, sb_limit_;
  std::array<std::array<uint8_t, 32>, 2> allocation_;
  std::array<std::array<uint8_t, 32>, 2> scfsi_;
  std::array<std::array<std::array<uint8_t, 3>, 32>, 2> scale_factor_;
  std::array<int, 3>                                    sample_code_;
  std::array<std::array<std::array<float, 32>, 3>, 2>   syin_;

 public:
  LayerII(Header h, std::unique_ptr<IAudio> audio);

 private:
  void Requantization(int index, int gr, int ch, int sb);

  void Stereo(int index, int gr, int sb);

 public:
  int DecodeFrame(std::vector<uint8_t>, int off) override;
};
}  // namespace jmp123::decoder

#endif  // JMP123_LAYER_2_H
