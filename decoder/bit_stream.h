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

#ifndef JMP123_BIT_STREAM_H
#define JMP123_BIT_STREAM_H

#include <cstdint>
#include <memory>
#include <vector>

namespace jmp123::decoder {
class BitStream {
 protected:
  int                  bit_pos_;
  int                  byte_pos_;
  std::vector<uint8_t> bit_reservoir_;

 private:
  int end_pos_;  // bit_reservoir_ 已填入字节数
  int max_off_;

 public:
  BitStream(int len, int extr);

  int Append(std::vector<uint8_t>const& b, int off, int len);

  void Feed(std::vector<uint8_t> const &other , int off);

  int Get_1_Bit();

  int GetBits_17(int n);

  int GetBits_9(int n);

  [[nodiscard]] int GetBytePos() const { return byte_pos_; }

  [[nodiscard]] int GetSize() const { return end_pos_; }

  void SkipBytes(int n);

  void SkipBits(int n);
};
}  // namespace jmp123::decoder

#endif  // JMP123_BIT_STREAM_H
