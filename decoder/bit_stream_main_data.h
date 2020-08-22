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

#ifndef JMP123_BIT_STREAM_MAIN_DATA_H
#define JMP123_BIT_STREAM_MAIN_DATA_H

#include "bit_stream.h"
#include "layer_3.h"
#include "tables.h"

namespace jmp123::decoder {

class BitStreamMainData : public BitStream {
 private:
  std::array<int, 3>  region_{};
  std::array<int const*, 32> htbv_{};
  std::array<int, 32> lin_{};

 public:
  /**
   * 创建一个位流BitStreamMainData对象，位流的缓冲区大小len指定，位流的缓冲区尾部空出的长度由extra指定。
   *
   * @param len
   *            缓冲区可访问长度。
   * @param extra
   *            缓冲区尾部空出的字节数。
   * @see BitStream #BitStream(int, int)
   */
  BitStreamMainData(int len, int extra);

  /**
   * 一个粒度组内的一个声道哈夫曼解码。
   *
   * @param ci
   *            一粒度内的一声道信息。
   * @param hv
   *            接收解码得到的576个值。
   * @return 576减去rzone区长度。
   */
  int DecodeHuff(LayerIII::ChannelInformation const& ci, std::array<int, 32 * 18 + 4>& hv);
};

}  // namespace jmp123::decoder

#endif  // JMP123_BIT_STREAM_MAIN_DATA_H
