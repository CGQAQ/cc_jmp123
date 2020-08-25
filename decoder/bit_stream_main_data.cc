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

#include "bit_stream_main_data.h"
namespace jmp123::decoder {
BitStreamMainData::BitStreamMainData(int len, int extra)
    : BitStream(len, extra) {
  htbv_[0]  = htbv0.data();  // hlen=0
  htbv_[1]  = htbv1.data();
  htbv_[2]  = htbv2.data();
  htbv_[3]  = htbv3.data();
  htbv_[4]  = htbv0.data();  // not used
  htbv_[5]  = htbv5.data();
  htbv_[6]  = htbv6.data();
  htbv_[7]  = htbv7.data();
  htbv_[8]  = htbv8.data();
  htbv_[9]  = htbv9.data();
  htbv_[10] = htbv10.data();
  htbv_[11] = htbv11.data();
  htbv_[12] = htbv12.data();
  htbv_[13] = htbv13.data();
  htbv_[14] = htbv0.data();  // not used
  htbv_[15] = htbv15.data();
  htbv_[16] = htbv_[17] = htbv_[18] = htbv_[19] = htbv_[20] = htbv_[21] =
      htbv_[22] = htbv_[23] = htbv16.data();
  htbv_[24] = htbv_[25] = htbv_[26] = htbv_[27] = htbv_[28] = htbv_[29] =
      htbv_[30] = htbv_[31] = htbv24.data();

  lin_.fill(0);
  lin_[16] = 1;
  lin_[17] = 2;
  lin_[18] = 3;
  lin_[19] = 4;
  lin_[20] = 6;
  lin_[21] = 8;
  lin_[22] = 10;
  lin_[23] = 13;
  lin_[24] = 4;
  lin_[25] = 5;
  lin_[26] = 6;
  lin_[27] = 7;
  lin_[28] = 8;
  lin_[29] = 9;
  lin_[30] = 11;
  lin_[31] = 13;
}
int BitStreamMainData::DecodeHuff(const LayerIII::ChannelInformation& ci,
                                  std::array<int, 32 * 18 + 4>& hv) {
  unsigned int tmp = 0, lin_bits = 0, max_idx = 0, idx = 0;
  auto&        b = bit_reservoir_;

  /*
   * 1. 初始化num,mask,part3len,region
   * mask: 暂存位流缓冲区不超过32比特数据,位流2级缓冲
   * num: mask剩余的比特数
   * part3len: 哈夫曼编码的主数据(main_data)的比特数
   */
  int part3len = ci.part2_3_length - ci.part2_length;
  int x        = ci.region1Start;     // region1
  int y        = ci.region2Start;     // region2
  int i        = ci.big_values << 1;  // bv
  if (i > 574) i = 574;               // 错误的big_value置为0 ?
  if (x < i) {
    region_[0] = x;
    if (y < i) {
      region_[1] = y;
      region_[2] = i;
    } else
      region_[1] = region_[2] = i;
  } else
    region_[0] = region_[1] = region_[2] = i;

  /*
   * 2. 使位流缓冲区按字节对齐
   */
  int num  = (8 - bit_pos_) & 7;
  uint32_t mask = 0;
  if (num > 0) {
    mask = GetBits_9(num);
    mask <<= 32 - num;
    part3len -= num;
  }

  /*
   * 3. 解码大值区
   */
  for (i = 0; i < 3; i++) {
    max_idx   = region_[i];
    tmp       = ci.table_select[i];
    auto const &htab = htbv_[tmp];
    lin_bits  = lin_[tmp];
    while (idx < max_idx) {
      if (part3len + num <= 0) {  //检测位流是否有错误
        num -= part3len + num;
        break;
      }

      while (num < 24) {  // refresh mask
        mask |= (b[byte_pos_++] & 0xff) << (24 - num);
        num += 8;
        part3len -= 8;
      }
      tmp = mask;
      y   = htab[tmp >> 30];
      while (y < 0) {
        tmp <<= 2;
        y = htab[(tmp >> 30) - y];
      }
      x = y >> 8;  // x暂存hlen
      num -= x;
      mask <<= x;

      x = (y >> 4) & 0xf;  // 解得x,y
      y &= 0xf;

      if (x != 0) {
        if (x == 15 && lin_bits != 0) {
          while (num < 24) {  // refresh mask
            mask |= (b[byte_pos_++] & 0xff) << (24 - num);
            num += 8;
            part3len -= 8;
          }
          x += mask >> (32 - lin_bits);  // 循环右移
          num -= lin_bits;
          mask <<= lin_bits;
        }
        hv[idx++] = (mask < 0) ? -x : x;
        num--;
        mask <<= 1;
      } else
        hv[idx++] = 0;

      if (y != 0) {
        if (y == 15 && lin_bits != 0) {
          while (num < 24) {  // refresh mask
            mask |= (b[byte_pos_++] & 0xff) << (24 - num);
            num += 8;
            part3len -= 8;
          }
          y += mask >> (32 - lin_bits);
          num -= lin_bits;
          mask <<= lin_bits;
        }
        hv[idx++] = (mask < 0) ? -y : y;
        num--;
        mask <<= 1;
      } else
        hv[idx++] = 0;
    }
  }

  /*
   * 4. 解码count1区
   */
  auto htab = (ci.count1table_select == 0) ? htc0.data() : htc1.data();
  while (idx < 572) {
    while (num < 10) {  // 6(max hlen)+4(signed bit)
      mask |= (b[byte_pos_++] & 0xff) << (24 - num);
      num += 8;
      part3len -= 8;
    }
    tmp = mask;
    y   = htab[tmp >> 28];
    while (y < 0) {
      tmp <<= 4;
      y = htab[(tmp >> 28) - y];
    }

    x = y >> 8;  // hlen
    mask <<= x;
    num -= x;
    // 修改num后立即检测(当前粒度内的当前声道的)主数据是否处理完，即使有数据损坏的文件也能继续解码。
    if (part3len + num <= 0) {
      num -= part3len + num;
      break;
    }

    // 一个码字(hcod)解码得到4个值
    if ((y <<= 28) < 0) {
      hv[idx++] = mask < 0 ? -1 : 1;
      num--;
      mask <<= 1;
    } else
      hv[idx++] = 0;
    if ((y <<= 1) < 0) {
      hv[idx++] = mask < 0 ? -1 : 1;
      num--;
      mask <<= 1;
    } else
      hv[idx++] = 0;
    if ((y <<= 1) < 0) {
      hv[idx++] = mask < 0 ? -1 : 1;
      num--;
      mask <<= 1;
    } else
      hv[idx++] = 0;
    if ((y <<= 1) < 0) {
      hv[idx++] = mask < 0 ? -1 : 1;
      num--;
      mask <<= 1;
    } else
      hv[idx++] = 0;
  }

  if (num > 0) {  // num位归还到位流缓冲区，也可以通过调用skipBits(-num)实现归还
    bit_pos_ = -num;  // 第6步还要使用num，借用bitPos暂存

    byte_pos_ += bit_pos_ >> 3;
    bit_pos_ &= 0x7;
  }

  /*
   * 5. rzone区直接置0,即hv[nozeroIndex..575]=0
   */
  int nozeroIndex = idx;
  while (idx < 576) hv[idx++] = 0;

  /*
   * 6. 丢弃附属位（ancillary_bit）。附属位不超过多少位？
   */
  part3len += num;
  if (part3len > 0) {  // 这还不一定是附属位，码流有错误也有可能出现这种情况
    while (part3len > 9) {
      GetBits_9(9);  // 不再是字节对齐的
      part3len -= 9;
    }
    GetBits_9(part3len);
  }

  return nozeroIndex;
}

}  // namespace jmp123::decoder