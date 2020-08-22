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

#ifndef JMP123_SYNTHESIS_CONCURRENT_H
#define JMP123_SYNTHESIS_CONCURRENT_H

#include <array>
#include <atomic>
#include <condition_variable>
#include <vector>

#include "synthesis.h"

namespace jmp123::decoder {

class LayerIII;

class SynthesisConcurrent {
 private:
  LayerIII& owner_;
  int                                     ch_;
  std::array<float, 32>                   samples_{};
  std::vector<std::array<float, 32 * 18>> pre_xr_;
  std::vector<std::array<float, 32 * 18>> cur_xr_;

  std::condition_variable notifier_;
  std::atomic_bool        pause_, alive_;

  std::mutex pause_mutex_;
  // c++ 20, but no compiler except gcc 10 support it now
  // so im gonna use std::thread instead
  //  std::jthread

 public:
  SynthesisConcurrent(LayerIII & owner, int ch);

  /**
   * 交换缓冲区并唤醒SynthesisConcurrent线程。
   *
   * @return
   * 一个空闲的缓冲区，该缓冲区用于使用SynthesisConcurrent线程的对象在逆量化、抗锯齿和IMDCT时暂存数据。
   */
  std::vector<std::array<float, 32 * 18>>*  StartSynthesis();

  /**
   * 获取一个空闲的缓冲区。
   *
   * @return
   * 一个空闲的缓冲区，该缓冲区用于使用SynthesisConcurrent的对象在逆量化、抗锯齿和IMDCT时暂存数据。
   */
  std::vector<std::array<float, 32 * 18>>* GetBuffer();

  /**
   * 关闭SynthesisConcurrent线程。
   */
  void Shutdown();

  /**
   * 使用SynthesisConcurrent的对象创建一个线程时，启动该线程将导致在独立执行的线程中调用SynthesisConcurrent的
   * run 方法进行异步多相合成滤波。
   */
  void operator()();
};

}  // namespace jmp123::decoder

#endif  // JMP123_SYNTHESIS_CONCURRENT_H
