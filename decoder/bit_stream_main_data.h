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

namespace jmp123::decoder {

class BitStreamMainData : BitStream {
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
  BitStreamMainData(int len, int extra) : BitStream(len, extra) {}
  int DecodeHuff() {return 1;}
};

}

#endif  // JMP123_BIT_STREAM_MAIN_DATA_H
