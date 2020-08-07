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

#ifndef JMP123_LAYER_3_H
#define JMP123_LAYER_3_H

#include "bit_stream.h"
#include "bit_stream_main_data.h"
#include "layer123.h"
#include "synthesis_concurrent.h"
namespace jmp123::decoder {
class LayerIII : public LayerI_II_III {
 public:
  struct ChannelInformation {
    // 从位流读取数据依次初始化的14个变量
    int part2_3_length;
    int big_values;

    int global_gain;
    int scalefac_compress;
    int window_switching_flag;
    int block_type;
    int mixed_block_flag;

    int* table_select;

    int* subblock_gain;
    int  region0_count;
    int  region1_count;
    int  preflag;
    int  scalefac_scale;

    int count1table_select;

    // 这3个通过计算初始化
    int region1Start;
    int region2Start;
    int part2_length;  // 增益因子(scale-factor)比特数

    ChannelInformation() {
      table_select  = new int[3];
      subblock_gain = new int[3];
    }
    ~ChannelInformation() {
      delete[] table_select;
      delete[] subblock_gain;
    }
  };

 public:
  int granules_;

 private:
  int       channels_;
  Header    header_;
  BitStream bs_si_;
  // TODO: implement BitStreamMainData.java first
  BitStreamMainData                     main_data_stream_;
  int                                   main_data_begin_;
  std::unique_ptr<int[]>                scfsi_;
  std::unique_ptr<ChannelInformation**> channel_info_;
  std::unique_ptr<int[]>                sfb_index_long_;
  std::unique_ptr<int[]>                sfb_index_short_;
  bool                                  is_mpeg_1_;
  SynthesisConcurrent                   filter_ch_0_;
  SynthesisConcurrent                   filter_ch_1;
  int                                   semaphore;

 public:
};
}  // namespace jmp123::decoder

#endif  // JMP123_LAYER_3_H
