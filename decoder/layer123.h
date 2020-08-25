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

#ifndef JMP123_LAYER123_H
#define JMP123_LAYER123_H

#include "audio_buffer.h"
#include "audio_interface.h"
#include "header.h"
#include "synthesis.h"

namespace jmp123::decoder {
class LayerI_II_III {
 private:
  AudioBuffer audio_buffer_;

 public:
  Synthesis filter_;

 public:
  LayerI_II_III(Header const &h, std::unique_ptr<IAudio> audio);
  virtual ~LayerI_II_III() = default;

  /**
   * 从此字节输入流中给定偏移量处开始解码一帧。
   *
   * @param b
   *            源数据缓冲区。
   * @param off
   *            开始解码字节处的偏移量。
   * @return
   * 源数据缓冲区新的偏移量，用于计算解码下一帧数据的开始位置在源数据缓冲区的偏移量。
   */
  virtual int DecodeFrame(std::vector<uint8_t> const &b, int off) = 0;

  /**
   * 音频输出。完成一帧多相合成滤波后调用此方法将多相合成滤波输出的PCM数据写入音频输出对象。
   * @see AudioBuffer#output()
   */
  void OutputAudio();

  /**
   * 音频输出缓冲区的全部内容刷向音频输出对象并将缓冲区偏移量复位。
   *
   * @see AudioBuffer#flush()
   */
  virtual void Close();
};
}  // namespace jmp123::decoder

#endif  // JMP123_LAYER123_H
