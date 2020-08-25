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

#ifndef JMP123_LAYER_1_H
#define JMP123_LAYER_1_H

#include <array>
#include <cmath>
#include <memory>

#include "bit_stream.h"
#include "header.h"
#include "layer123.h"

namespace jmp123::decoder {
class LayerI : public LayerI_II_III {
  Header const&                          header_;
  std::unique_ptr<BitStream>             bs_;
  std::array<std::array<uint8_t, 32>, 2> allocation_{};
  std::array<std::array<uint8_t, 32>, 2> scale_factor_{};
  std::array<std::array<float, 32>, 2>   syin_{};

 public:
  LayerI(Header const& h, std::unique_ptr<IAudio> audio)
      : LayerI_II_III(h, std::move(audio)),
        header_(h),
        bs_(std::make_unique<BitStream>(4096, 512)) {}

 private:
  /*
   * 逆量化公式:
   * s'' = (2^nb / (2^nb - 1)) * (s''' + 2^(-nb + 1))
   * s' = factor * s''
   */
  float Requantization(int ch, int sb, int nb);

 public:
  int DecodeFrame(std::vector<uint8_t> const& b, int off) override;
};
}  // namespace jmp123::decoder

#endif  // JMP123_LAYER_1_H
