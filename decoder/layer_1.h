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

#include "bit_stream.h"
#include "header.h"

namespace jmp123::decoder {
class LayerI {
  Header                                 header_;
  BitStream                              bs_;
  std::array<std::array<uint8_t, 32>, 2> allocation_;
  std::array<std::array<uint8_t, 32>, 2> scale_factor_;
  std::array<std::array<float, 32>, 2>   syin_;

 public:
  // TODO: Implement it, but it depends on AudioBuffer and Layer123, so I will implement those tow first
};
}  // namespace jmp123::decoder

#endif  // JMP123_LAYER_1_H
