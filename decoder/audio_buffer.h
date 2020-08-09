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

#ifndef JMP123_AUDIO_BUFFER_H
#define JMP123_AUDIO_BUFFER_H
#include <memory>

#include "audio_interface.h"
namespace jmp123::decoder {
class AudioBuffer {
 public:
  std::unique_ptr<uint8_t[]> pcm_buf_;
  std::array<int, 2>         off_;

 private:
  std::unique_ptr<IAudio> audio_;
  int                     size_;

 public:
  /**
   * 音频输出缓冲区构建器。
   *
   * @param audio
   *            音频输出对象。如果指定为null则调用 {@link #output()}
   * 不产生输出，仅清空缓冲区。
   * @param size
   *            音频输出缓冲区长度，单位“字节”。
   */
  AudioBuffer(std::unique_ptr<IAudio>& audio, int size);

  /**
   * 音频输出缓冲区的内容刷向音频输出对象并将缓冲区偏移量复位。当缓冲区填满时才向音频输出对象写入，但调用者并不需要知道当前缓冲区是否已经填满。
   * 防止缓冲区溢出， 每解码完一帧应调用此方法一次。
   */
  void Output();

  /**
   * 音频输出缓冲区的全部内容刷向音频输出对象并将缓冲区偏移量复位。在解码完一个文件的最后一帧后调用此方法，将缓冲区剩余内容写向音频输出对象。
   */
  void Flush();
};
}  // namespace jmp123::decoder

#endif  // JMP123_AUDIO_BUFFER_H
