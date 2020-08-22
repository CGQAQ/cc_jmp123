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

#ifndef JMP123_AUDIO_INTERFACE_H
#define JMP123_AUDIO_INTERFACE_H

#include <memory>
#include <string>

#include "header.h"

namespace jmp123::decoder {
class IAudio {
 public:
  /**
   * 打开源数据行（音频输出设备）。
   *
   * @param h
   *            {@link Header}对象
   * @param artist
   *            艺术家。可以为 null 值。
   * @return 打开成功返回true，否则返回false。
   * @see Header
   */
  virtual bool Open(Header h, std::unique_ptr<std::string>) = 0;

  /**
   * 将音频数据写入混频器。所请求的源数据是从PCM缓冲区 b
   * 中读取的（从数组中首字节开始），并且将被写入数据行的缓冲区。
   * 如果调用者试图写入多于当前可写入数据量的数据，则此方法在写入所请求数据量之前一直阻塞。即使要写入的请求数据量大于数据行的缓冲区大小
   * ，此方法也适用。不过，如果在写入请求的数据量之前数据行已关闭、停止或刷新，则该方法不再阻塞。
   *
   * @param b
   *            源数据。
   * @param size
   *            写入的数据长度。
   * @return 向混频器器写入PCM数据的字节数。
   */
  virtual int Write(std::vector<uint8_t> const& b) = 0;

  /**
   * 根据b指定的值向混频器器写入数据或暂停向混频器器写入数据。
   *
   * @param b
   *        true表示向混频器器写入数据（即正常播放输出）；fase表示暂停向混频器器写入数据。
   */
  virtual void Start(bool b) = 0;

  /**
   * 通过在清空源数据行的内部缓冲区之前继续向混频器器写入数据，排空源数据行中的列队数据。在完成排空操作之前，此方法发生阻塞。因为这是一个阻塞方法，
   * 所以应小心使用它。如果在队列中有数据的终止行上调用
   * drain()，则在该行正在运行和数据队列变空之前，此方法将发生阻塞。如果通过一个线程调用
   * drain()，另一个线程继续填充数据队列，则该操作不能完成排空操作。此方法总是在关闭源数据行时返回。
   * <p>
   * 上述描述，看了让人挠头。换种说法，就是正常播放（不被用户终止）完一个文件时调用该方法，把已经向音频输出缓冲写入的数据刷一下，使之播放输出
   * 。
   * 如果不刷，文件解码完会立即关闭解码器，音频输出设备会随解码器关闭，那么最后一点儿已经被解码的数据不能播放输出。
   */
  virtual void Drain() = 0;

  /**
   * 关闭源数据行（音频输出设备）。指示可以释放的该源数据行使用的所有系统资源。
   */
  virtual void close() = 0;

  /**
   * 刷新输出消息提示。由于在正常播放输出过程中 {@link #write(byte[], int)}
   * 方法总是以相对固定的频率被调用，利用这一特性，可以向调用者提供一个时钟，便于调用者定时向播放器用户接口（UI）输出某些信息。
   *
   * @param msg
   *            调用者指定的消息。
   * @see #write(byte[], int)
   * @see Header#getFrameDuration()
   * @see Header#getFrames()
   * @see Header#getDuration()
   */
  virtual void refreshMessage(std::string msg) = 0;


  virtual ~IAudio() = default;
};
}  // namespace jmp123::decoder

#endif  // JMP123_AUDIO_INTERFACE_H
