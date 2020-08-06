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

#include "layer123.h"
namespace jmp123::decoder {

LayerI_II_III::LayerI_II_III(const Header &h, std::unique_ptr<IAudio> audio)
    : audio_buffer_(
        std::make_unique<AudioBuffer>(std::move(audio), 4 * h.GetPcmSize())),
      filter_(std::make_unique<Synthesis>(std::move(audio_buffer_),
                                          h.GetChannelCount())) {}

void LayerI_II_III::OutputAudio() { audio_buffer_->Output(); }
void LayerI_II_III::Close() { audio_buffer_->Flush(); }

}  // namespace jmp123::decoder
