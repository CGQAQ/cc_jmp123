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

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "bit_stream.h"
#include "layer123.h"
#include "synthesis_concurrent.h"

namespace jmp123::decoder {

class BitStreamMainData;

class LayerIII : public LayerI_II_III {
 public:
  /**
   * 一个粒度内一个声道的信息。哈夫曼解码用到part2_3_length等protected变量。
   */
  struct ChannelInformation {
    // 从位流读取数据依次初始化的14个变量
    int part2_3_length;
    int big_values;

    int global_gain;
    int scalefac_compress;
    int window_switching_flag;
    int block_type;
    int mixed_block_flag;

    std::array<int, 3> table_select;

    std::array<int, 3> subblock_gain;
    int  region0_count;
    int  region1_count;
    int  preflag;
    int  scalefac_scale;

    int count1table_select;

    // 这3个通过计算初始化
    int region1Start;
    int region2Start;
    int part2_length;  // 增益因子(scale-factor)比特数
  };



 private:
  Header    header_;
  int       channels_;
  BitStream bs_si_;
  // TODO: implement BitStreamMainData.java first
  std::unique_ptr<BitStreamMainData>           main_data_stream_;
  int                                          main_data_begin_;
  std::vector<int>                             scfsi_;
  std::vector<std::vector<ChannelInformation>> channel_info_;
  std::vector<int>                             sfb_index_long_;
  std::vector<int>                             sfb_index_short_;
  bool                                         is_mpeg_1_;
 public:
  int const granules_;

 private:
  SynthesisConcurrent                          filter_ch_0_;
  SynthesisConcurrent                          filter_ch_1;

  std::thread t1, t2;

 public:
  [[maybe_unused]] LayerIII(Header h, std::unique_ptr<IAudio> audio);
  ~LayerIII();

  // 1.
  //>>>>SIDE INFORMATION (part1)=============================================
  // private int part2_3_bits;//----debug
 private:
  int GetSideInfo(std::vector<uint8_t> const& b, int off);
  //<<<<SIDE INFORMATION=====================================================

 private:
  // 2.
  //>>>>SCALE FACTORS========================================================
  std::vector<std::array<int, 23>>     scalefacLong;   // [channels][23];
  std::vector<std::array<int, 13 * 3>> scalefacShort;  // [channels][13*3];
  std::array<int, 256> i_slen2;  // MPEG-2 slen for intensity stereo
  std::array<int, 512> n_slen2;  // MPEG-2 slen for 'normal' mode
  // slen: 增益因子(scalefactor)比特数
  std::array<std::array<std::array<uint8_t, 4>, 6>, 3> nr_of_sfb;  //[3][6][4]

  void GetScaleFactors_2(int gr, int ch);

  // MPEG-1
  std::array<int, 16> slen0 = {0, 0, 0, 0, 3, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4};
  std::array<int, 16> slen1 = {0, 1, 2, 3, 0, 1, 2, 3, 1, 2, 3, 1, 2, 3, 2, 3};
  void                GetScaleFactors_1(int gr, int ch);
  //<<<<SCALE FACTORS========================================================

 private:
  // 3.
  //>>>>HUFFMAN BITS=========================================================
  std::array<int, 32 * 18 + 4> hv;  //[32 * 18 + 4],暂存解得的哈夫曼值
  std::array<int, 2>           rzeroIndex;
  void                         huffBits(int gr, int ch);
  //<<<<HUFFMAN BITS=========================================================

 private:
  // 4.
  //>>>>REQUANTIZATION & REORDER=============================================
  std::vector<std::array<float, 32 * 18>> xrch0;       // [maxGr][32*18]
  std::vector<std::array<float, 32 * 18>> xrch1;       // [maxGr][32*18]
  std::array<float, 256 + 118 + 4>        floatPow2;   // [256 + 118 + 4]
  std::array<float, 8207>                 floatPowIS;  // [8207]
  std::array<int, 22>
                      widthLong;  // [22] 长块的增益因子频带(用一个增益因子逆量化频率线的条数)
  std::array<int, 13> widthShort;  // [13] 短块的增益因子频带
  int                 rzeroBandLong;
  std::array<int, 3>  rzeroBandShort;  //= new int[3];

  /**
   * 逆量化并对短块(纯短块和混合块中的短块)重排序.在逆量化时赋值的变量:<br>
   * rzero_bandL -- 长块非零哈夫曼值的频带数,用于强度立体声(intensity
   *stereo)处理<br> rzero_bandS --
   *短块非零哈夫曼值的频带数,用于强度立体声处理<br> rzero_index --
   *非零哈夫曼值的"子带"数 <p> Layer3 逆量化公式ISO/IEC 11172-3, 2.4.3.4 <p> XRi
   *= pow(2, 0.25 * (global_gain - 210)) <br> if (LONG block types) <br> 　　XRi
   **= pow(2, -0.5 * (1 + scalefac_scale) * (L[sfb] + preflag * pretab[sfb]))
   *<br> if (SHORT block types) { <br> 　　XRi *= pow(2, 0.25 * -8 *
   *subblock_gain[sfb]) <br> 　　XRi *= pow(2, 0.25 * -2 * (1 + scalefac_scale)
   ** S[scf]) } <br> XRi *= sign(haffVal) * pow(abs(haffVal), 4/3) <br>
   *
   * @param gr 当前粒度。
   * @param ch 当前声道。
   * @param xrch 保存逆量化输出的576个值。
   */
  void Requantizer(int gr, int ch, std::array<float, 32 * 18>& xrch);
  //<<<<REQUANTIZATION & REORDER=============================================

  // 5.
  //>>>>STEREO===============================================================

  // 在requantizer方法内已经作了除以根2处理,ms_stereo内不再除以根2.
  void ms_stereo(int gr);

  std::vector<std::vector<float>> lsf_is_coef;
  std::vector<float>              is_coef;
  // 解码一个频带强度立体声,MPEG-1
  void is_lines_1(int pos, int idx0, int width, int step, int gr);

  // 解码一个频带强度立体声,MPEG-2
  void is_lines_2(int tab2, int pos, int idx0, int width, int step, int gr);

  /*
   * 强度立体声(intensity stereo)解码
   *
   * ISO/IEC
   * 11172-3不对混合块中的长块作强度立体声处理,但很多MP3解码程序都作了处理.
   */
  void intensity_stereo(int gr);
  //<<<<STEREO===============================================================
 private:
  // 6.
  //>>>>ANTIALIAS============================================================
  void antialias(int gr, int ch, std::array<float, 32 * 18> & xrch);
  //<<<<ANTIALIAS============================================================

 private:
  // 7.
  //>>>>HYBRID(synthesize via iMDCT)=========================================
  void imdct12(std::array<float, 32 * 18> & xrch,
               std::array<float, 32 * 18>& pre, int off);

  void imdct36(std::array<float, 32 * 18> & xrch,
               std::array<float, 32 * 18>& preBlck, int off, int block_type);

  std::array<float, 32 * 18> preBlckCh0;  // [32*18],左声道FIFO队列
  std::array<float, 32 * 18> preBlckCh1;  // [32*18],右声道FIFO
  void hybrid(int gr, int ch, std::array<float, 32 * 18>& xrch,
              std::array<float, 32 * 18>& preb);
  //<<<<HYBRID(synthesize via iMDCT)=========================================

  // 8.
  //>>>>INVERSE QUANTIZE SAMPLES=============================================
  //
  // 在decoder.ConcurrentSynthesis.run 方法内实现多相频率倒置
  //
  //<<<<INVERSE QUANTIZE SAMPLES=============================================

  // 9.
  //>>>>SYNTHESIZE VIA POLYPHASE MDCT========================================
  //
  // 在decoder.ConcurrentSynthesis.run()方法内调用filter.synthesisSubBand()
  // 实现多相合成滤波
  //
  //<<<<SYNTHESIZE VIA POLYPHASE MDCT========================================

  // 10.
  //>>>>OUTPUT PCM SAMPLES===================================================
  //
  // jmp123.decoder.AudioBuffer, jmp123.output.Audio
  //
 public:
  //<<<<OUTPUT PCM SAMPLES===================================================
  std::condition_variable notifier_;
  std::mutex              notifier_mutex_;
  std::atomic_int         semaphore_;

  /**
   * 解码1帧Layer Ⅲ
   */

  int DecodeFrame(std::vector<uint8_t> const &b, int off) override;

  /**
   * 关闭帧的解码。如果用多线程并发解码，这些并发的解码线程将被终止。
   * @see Layer123#close()
   */
  void Close() override;
  /**
   * 滤波线程完成一次的滤波任务后向调用者提交结果。滤波线程完成一次滤波任务后调用该方法。
   */
  void SubmitSynthesis();
};
}  // namespace jmp123::decoder

#endif  // JMP123_LAYER_3_H
