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

#ifndef JMP123_SYNTHESIS_H
#define JMP123_SYNTHESIS_H
#include <array>
#include <vector>

#include "audio_buffer.h"
#include "tables.h"
namespace jmp123::decoder {
class Synthesis {
 private:
  AudioBuffer& audio_buffer_;

  /*
   * 向PCM缓冲区写入数据的步长值，左右声道的PCM数据在PCM缓冲区内是交替排列的。
   * 指示解码某一声道时写入一次数据后，下一次应该写入的位置。
   */
  int const kStep_;

  /*
   * 暂存DCT输出结果的FIFO队列。
   */
  std::vector<std::array<float, 1024>> fifo_buf_;

  /*
   * fifobuf的偏移量，用它完成FIFO队列的移位操作。
   */
  std::vector<int> fifo_index_;

  int max_pcm_;

 public:
  /**
   * 获取PCM最大峰值。
   * @return PCM样本的最大峰值。该最大值可用于音量规格化。
   */
  [[maybe_unused]] int GetMaxPCM() const;

  /**
   * 一个子带多相合成滤波。
   *
   * @param samples
   *            源数据，为32个样本值。
   * @param ch
   *            当前的声道。左声道0，右声道1。
   */
  void SynthesisSubBand(std::array<float, 32> samples, int ch);

  /**
   * 一个子带的矩阵运算。
   * @param src 输入的32个样本值。
   * @param dest 暂存输出值的长度为1024个元素的FIFO队列。
   * @param off
   * FIFO队列的偏移量。一个子带一次矩阵运算输出64个值连续存储到FIFO队列，存储的起始位置由off指定。
   */
  void Dct32To64(std::array<float, 32>   src, std::array<float, 1024> dest, int off);


  /**
   * 子带多相合成滤波构造器。
   *
   * @param ab
   *            音频输出对象。
   *
   * @param channels
   *            声道数，用于计算输出PCM时的步长值。
   */
  Synthesis(AudioBuffer &ab, int channels);
};
}  // namespace jmp123::decoder

#endif  // JMP123_SYNTHESIS_H
