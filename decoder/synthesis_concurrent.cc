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

#include "synthesis_concurrent.h"

#include "layer_3.h"

namespace jmp123::decoder {
SynthesisConcurrent::SynthesisConcurrent(LayerIII& owner, int ch)
    : owner_(owner), ch_(ch), pause_(true), alive_(true) {
  samples_.fill(0);
  pre_xr_ = std::vector<std::array<float, 32 * 18>>(owner.granules_);
  cur_xr_ = std::vector<std::array<float, 32 * 18>>(owner.granules_);
}
std::vector<std::array<float, 32 * 18>> const& SynthesisConcurrent::StartSynthesis() {
  // 1. 交换缓冲区
  //  auto p  = pre_xr_;
  //  pre_xr_ = cur_xr_;
  //  cur_xr_ = p;
  swap(pre_xr_, cur_xr_);

  // 2. 通知run()干活
  std::lock_guard lock{ this->pause_mutex_ };
  pause_ = false;
  notifier_.notify_one();

  // 3. 返回"空闲的"缓冲区，该缓冲区内的数据已被run()方法使用完毕
  return pre_xr_;
}
std::vector<std::array<float, 32 * 18>>  const& SynthesisConcurrent::GetBuffer() { return pre_xr_; }
void SynthesisConcurrent::Shutdown() {
  alive_ = false;
  pause_ = false;
  notifier_.notify_one();
}
void SynthesisConcurrent::operator()() {
  int   gr = 0, sub = 0, ss = 0, i = 0;
  int   granules = owner_.granules_;
  auto& filter   = owner_.filter_;

  while (alive_) {
    std::unique_lock<std::mutex> lock(pause_mutex_);
    while (pause_) notifier_.wait(lock);
    pause_ = true;
    lock.unlock();

    for (gr = 0; gr < granules; ++gr) {
      auto& xr = cur_xr_[gr];
      for (ss = 0; ss < 18; ss += 2) {
          for (i = ss, sub = 0; sub < 32; sub++, i += 18) {
              samples_[sub] = xr[i];
          }
          filter.SynthesisSubBand(samples_, ch_);

          for (i = ss + 1, sub = 0; sub < 32; sub += 2, i += 36) {
              samples_[sub] = xr[i];

              // 多相频率倒置(INVERSE QUANTIZE SAMPLES)
              samples_[sub + 1] = -xr[i + 18];
          }
          filter.SynthesisSubBand(samples_, ch_);
      }
    }
    // 3. 提交结果
    owner_.SubmitSynthesis();
  }
}
}  // namespace jmp123::decoder
