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

#include "audio_buffer.h"
namespace jmp123::decoder {
AudioBuffer::AudioBuffer(std::unique_ptr<IAudio> audio, int size)
    : audio_(std::move(audio)), size_(size) {
  pcm_buf_ = std::make_unique<uint8_t[]>(size);
  off_     = std::unique_ptr<int[]>(new int[2]{0, 2});
}
}
void jmp123::decoder::AudioBuffer::Output() {
  if (off_[0] == size_) {
    if (audio_ != nullptr) audio_->Write(pcm_buf_.get(), size_);
    off_[0] = 0;
    off_[1] = 2;
  }
}
void jmp123::decoder::AudioBuffer::Flush() {
  if (audio_ != nullptr) {
    audio_->Write(pcm_buf_.get(), off_[0]);
    audio_->Drain();
  }
  off_[0] = 0;
  off_[1] = 2;
}
