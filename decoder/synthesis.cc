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

#include "synthesis.h"
namespace jmp123::decoder {
Synthesis::Synthesis(AudioBuffer& ab, int channels)
    : audio_buffer_(ab),
      kStep_(channels == 2 ? 4 : 2),
      fifo_buf_(channels),
      fifo_index_(channels) {}
int  Synthesis::GetMaxPCM() const { return max_pcm_; }
void Synthesis::SynthesisSubBand(std::array<float, 32> const& samples, int ch) {
  auto&        fifo    = fifo_buf_[ch];
  auto&        pcm_buf = audio_buffer_.pcm_buf_;
  float        sum     = 0;
  unsigned int pcm_i   = 0; // java >>>
  int          off     = audio_buffer_.off_[ch];

  // 1. shift
  fifo_index_[ch] = (fifo_index_[ch] - 64) & 0x3ff;

  Dct32To64(samples, fifo, fifo_index_[ch]);

  switch (fifo_index_[ch]) {
    case 0:
      for (int i = 0; i < 32; i++, off += kStep_) {
        auto const& win = kDewin[i];
        sum             = win[0] * fifo[i];
        sum += win[1] * fifo[i + 96];
        sum += win[2] * fifo[i + 128];
        sum += win[3] * fifo[i + 224];
        sum += win[4] * fifo[i + 256];
        sum += win[5] * fifo[i + 352];
        sum += win[6] * fifo[i + 384];
        sum += win[7] * fifo[i + 480];
        sum += win[8] * fifo[i + 512];
        sum += win[9] * fifo[i + 608];
        sum += win[10] * fifo[i + 640];
        sum += win[11] * fifo[i + 736];
        sum += win[12] * fifo[i + 768];
        sum += win[13] * fifo[i + 864];
        sum += win[14] * fifo[i + 896];
        sum += win[15] * fifo[i + 992];
        pcm_i =
            sum > 32767 ? 32767 : (sum < -32768 ? -32768 : (int)sum);  // clip
        pcm_buf[off]     = pcm_i;
        pcm_buf[off + 1] = pcm_i >> 8u;
        if (pcm_i > max_pcm_) max_pcm_ = pcm_i;
      }
      break;
    case 64:
      // u_vector={64,160,192,288,320,416,448,544,576,672,704,800,832,928,960,32}
      for (int i = 0; i < 32; i++, off += kStep_) {
        auto const& win = kDewin[i];
        sum             = win[0] * fifo[i + 64];
        sum += win[1] * fifo[i + 160];
        sum += win[2] * fifo[i + 192];
        sum += win[3] * fifo[i + 288];
        sum += win[4] * fifo[i + 320];
        sum += win[5] * fifo[i + 416];
        sum += win[6] * fifo[i + 448];
        sum += win[7] * fifo[i + 544];
        sum += win[8] * fifo[i + 576];
        sum += win[9] * fifo[i + 672];
        sum += win[10] * fifo[i + 704];
        sum += win[11] * fifo[i + 800];
        sum += win[12] * fifo[i + 832];
        sum += win[13] * fifo[i + 928];
        sum += win[14] * fifo[i + 960];
        sum += win[15] * fifo[i + 32];
        pcm_i        = sum > 32767 ? 32767 : (sum < -32768 ? -32768 : (int)sum);
        pcm_buf[off] = pcm_i;
        pcm_buf[off + 1] = pcm_i >> 8u;
        if (pcm_i > max_pcm_) max_pcm_ = pcm_i;
      }
      break;
    case 128:
      // u_vector={128,224,256,352,384,480,512,608,640,736,768,864,896,992,0,96}
      for (int i = 0; i < 32; i++, off += kStep_) {
        auto const& win = kDewin[i];
        sum             = win[0] * fifo[i + 128];
        sum += win[1] * fifo[i + 224];
        sum += win[2] * fifo[i + 256];
        sum += win[3] * fifo[i + 352];
        sum += win[4] * fifo[i + 384];
        sum += win[5] * fifo[i + 480];
        sum += win[6] * fifo[i + 512];
        sum += win[7] * fifo[i + 608];
        sum += win[8] * fifo[i + 640];
        sum += win[9] * fifo[i + 736];
        sum += win[10] * fifo[i + 768];
        sum += win[11] * fifo[i + 864];
        sum += win[12] * fifo[i + 896];
        sum += win[13] * fifo[i + 992];
        sum += win[14] * fifo[i];
        sum += win[15] * fifo[i + 96];
        pcm_i        = sum > 32767 ? 32767 : (sum < -32768 ? -32768 : (int)sum);
        pcm_buf[off] = pcm_i;
        pcm_buf[off + 1] = pcm_i >> 8u;
        if (pcm_i > max_pcm_) max_pcm_ = pcm_i;
      }
      break;
    case 192:
      // u_vector={192,288,320,416,448,544,576,672,704,800,832,928,960,32,64,160}
      for (int i = 0; i < 32; i++, off += kStep_) {
        auto const& win = kDewin[i];
        sum             = win[0] * fifo[i + 192];
        sum += win[1] * fifo[i + 288];
        sum += win[2] * fifo[i + 320];
        sum += win[3] * fifo[i + 416];
        sum += win[4] * fifo[i + 448];
        sum += win[5] * fifo[i + 544];
        sum += win[6] * fifo[i + 576];
        sum += win[7] * fifo[i + 672];
        sum += win[8] * fifo[i + 704];
        sum += win[9] * fifo[i + 800];
        sum += win[10] * fifo[i + 832];
        sum += win[11] * fifo[i + 928];
        sum += win[12] * fifo[i + 960];
        sum += win[13] * fifo[i + 32];
        sum += win[14] * fifo[i + 64];
        sum += win[15] * fifo[i + 160];
        pcm_i        = sum > 32767 ? 32767 : (sum < -32768 ? -32768 : (int)sum);
        pcm_buf[off] = pcm_i;
        pcm_buf[off + 1] = pcm_i >> 8u;
        if (pcm_i > max_pcm_) max_pcm_ = pcm_i;
      }
      break;
    case 256:
      // u_vector={256,352,384,480,512,608,640,736,768,864,896,992,0,96,128,224}
      for (int i = 0; i < 32; i++, off += kStep_) {
        auto const& win = kDewin[i];
        sum             = win[0] * fifo[i + 256];
        sum += win[1] * fifo[i + 352];
        sum += win[2] * fifo[i + 384];
        sum += win[3] * fifo[i + 480];
        sum += win[4] * fifo[i + 512];
        sum += win[5] * fifo[i + 608];
        sum += win[6] * fifo[i + 640];
        sum += win[7] * fifo[i + 736];
        sum += win[8] * fifo[i + 768];
        sum += win[9] * fifo[i + 864];
        sum += win[10] * fifo[i + 896];
        sum += win[11] * fifo[i + 992];
        sum += win[12] * fifo[i];
        sum += win[13] * fifo[i + 96];
        sum += win[14] * fifo[i + 128];
        sum += win[15] * fifo[i + 224];
        pcm_i        = sum > 32767 ? 32767 : (sum < -32768 ? -32768 : (int)sum);
        pcm_buf[off] = pcm_i;
        pcm_buf[off + 1] = pcm_i >> 8u;
        if (pcm_i > max_pcm_) max_pcm_ = pcm_i;
      }
      break;
    case 320:
      // u_vector={320,416,448,544,576,672,704,800,832,928,960,32,64,160,192,288}
      for (int i = 0; i < 32; i++, off += kStep_) {
        auto const& win = kDewin[i];
        sum             = win[0] * fifo[i + 320];
        sum += win[1] * fifo[i + 416];
        sum += win[2] * fifo[i + 448];
        sum += win[3] * fifo[i + 544];
        sum += win[4] * fifo[i + 576];
        sum += win[5] * fifo[i + 672];
        sum += win[6] * fifo[i + 704];
        sum += win[7] * fifo[i + 800];
        sum += win[8] * fifo[i + 832];
        sum += win[9] * fifo[i + 928];
        sum += win[10] * fifo[i + 960];
        sum += win[11] * fifo[i + 32];
        sum += win[12] * fifo[i + 64];
        sum += win[13] * fifo[i + 160];
        sum += win[14] * fifo[i + 192];
        sum += win[15] * fifo[i + 288];
        pcm_i        = sum > 32767 ? 32767 : (sum < -32768 ? -32768 : (int)sum);
        pcm_buf[off] = pcm_i;
        pcm_buf[off + 1] = pcm_i >> 8u;
        if (pcm_i > max_pcm_) max_pcm_ = pcm_i;
      }
      break;
    case 384:
      // u_vector={384,480,512,608,640,736,768,864,896,992,0,96,128,224,256,352}
      for (int i = 0; i < 32; i++, off += kStep_) {
        auto const& win = kDewin[i];
        sum             = win[0] * fifo[i + 384];
        sum += win[1] * fifo[i + 480];
        sum += win[2] * fifo[i + 512];
        sum += win[3] * fifo[i + 608];
        sum += win[4] * fifo[i + 640];
        sum += win[5] * fifo[i + 736];
        sum += win[6] * fifo[i + 768];
        sum += win[7] * fifo[i + 864];
        sum += win[8] * fifo[i + 896];
        sum += win[9] * fifo[i + 992];
        sum += win[10] * fifo[i];
        sum += win[11] * fifo[i + 96];
        sum += win[12] * fifo[i + 128];
        sum += win[13] * fifo[i + 224];
        sum += win[14] * fifo[i + 256];
        sum += win[15] * fifo[i + 352];
        pcm_i = sum > 32767 ? 32767 : (sum < -32768 ? -32768 : (int)sum);

        pcm_buf[off]     = pcm_i;
        pcm_buf[off + 1] = pcm_i >> 8u;
        if (pcm_i > max_pcm_) max_pcm_ = pcm_i;
      }
      break;
    case 448:
      // u_vector={448,544,576,672,704,800,832,928,960,32,64,160,192,288,320,416}
      for (int i = 0; i < 32; i++, off += kStep_) {
        auto win = kDewin[i];
        sum      = win[0] * fifo[i + 448];
        sum += win[1] * fifo[i + 544];
        sum += win[2] * fifo[i + 576];
        sum += win[3] * fifo[i + 672];
        sum += win[4] * fifo[i + 704];
        sum += win[5] * fifo[i + 800];
        sum += win[6] * fifo[i + 832];
        sum += win[7] * fifo[i + 928];
        sum += win[8] * fifo[i + 960];
        sum += win[9] * fifo[i + 32];
        sum += win[10] * fifo[i + 64];
        sum += win[11] * fifo[i + 160];
        sum += win[12] * fifo[i + 192];
        sum += win[13] * fifo[i + 288];
        sum += win[14] * fifo[i + 320];
        sum += win[15] * fifo[i + 416];
        pcm_i        = sum > 32767 ? 32767 : (sum < -32768 ? -32768 : (int)sum);
        pcm_buf[off] = pcm_i;
        pcm_buf[off + 1] = pcm_i >> 8u;
        if (pcm_i > max_pcm_) max_pcm_ = pcm_i;
      }
      break;
    case 512:
      // u_vector={512,608,640,736,768,864,896,992,0,96,128,224,256,352,384,480}
      for (int i = 0; i < 32; i++, off += kStep_) {
        auto win = kDewin[i];
        sum      = win[0] * fifo[i + 512];
        sum += win[1] * fifo[i + 608];
        sum += win[2] * fifo[i + 640];
        sum += win[3] * fifo[i + 736];
        sum += win[4] * fifo[i + 768];
        sum += win[5] * fifo[i + 864];
        sum += win[6] * fifo[i + 896];
        sum += win[7] * fifo[i + 992];
        sum += win[8] * fifo[i];
        sum += win[9] * fifo[i + 96];
        sum += win[10] * fifo[i + 128];
        sum += win[11] * fifo[i + 224];
        sum += win[12] * fifo[i + 256];
        sum += win[13] * fifo[i + 352];
        sum += win[14] * fifo[i + 384];
        sum += win[15] * fifo[i + 480];
        pcm_i        = sum > 32767 ? 32767 : (sum < -32768 ? -32768 : (int)sum);
        pcm_buf[off] = pcm_i;
        pcm_buf[off + 1] = pcm_i >> 8u;
        if (pcm_i > max_pcm_) max_pcm_ = pcm_i;
      }
      break;
    case 576:
      // u_vector={576,672,704,800,832,928,960,32,64,160,192,288,320,416,448,544}
      for (int i = 0; i < 32; i++, off += kStep_) {
        auto win = kDewin[i];
        sum      = win[0] * fifo[i + 576];
        sum += win[1] * fifo[i + 672];
        sum += win[2] * fifo[i + 704];
        sum += win[3] * fifo[i + 800];
        sum += win[4] * fifo[i + 832];
        sum += win[5] * fifo[i + 928];
        sum += win[6] * fifo[i + 960];
        sum += win[7] * fifo[i + 32];
        sum += win[8] * fifo[i + 64];
        sum += win[9] * fifo[i + 160];
        sum += win[10] * fifo[i + 192];
        sum += win[11] * fifo[i + 288];
        sum += win[12] * fifo[i + 320];
        sum += win[13] * fifo[i + 416];
        sum += win[14] * fifo[i + 448];
        sum += win[15] * fifo[i + 544];
        pcm_i        = sum > 32767 ? 32767 : (sum < -32768 ? -32768 : (int)sum);
        pcm_buf[off] = pcm_i;
        pcm_buf[off + 1] = (pcm_i >> 8u);
        if (pcm_i > max_pcm_) max_pcm_ = pcm_i;
      }
      break;
    case 640:
      // u_vector={640,736,768,864,896,992,0,96,128,224,256,352,384,480,512,608}
      for (int i = 0; i < 32; i++, off += kStep_) {
        auto const& win = kDewin[i];
        sum             = win[0] * fifo[i + 640];
        sum += win[1] * fifo[i + 736];
        sum += win[2] * fifo[i + 768];
        sum += win[3] * fifo[i + 864];
        sum += win[4] * fifo[i + 896];
        sum += win[5] * fifo[i + 992];
        sum += win[6] * fifo[i];
        sum += win[7] * fifo[i + 96];
        sum += win[8] * fifo[i + 128];
        sum += win[9] * fifo[i + 224];
        sum += win[10] * fifo[i + 256];
        sum += win[11] * fifo[i + 352];
        sum += win[12] * fifo[i + 384];
        sum += win[13] * fifo[i + 480];
        sum += win[14] * fifo[i + 512];
        sum += win[15] * fifo[i + 608];
        pcm_i        = sum > 32767 ? 32767 : (sum < -32768 ? -32768 : (int)sum);
        pcm_buf[off] = pcm_i;
        pcm_buf[off + 1] = (pcm_i >> 8u);
        if (pcm_i > max_pcm_) max_pcm_ = pcm_i;
      }
      break;
    case 704:
      // u_vector={704,800,832,928,960,32,64,160,192,288,320,416,448,544,576,672}
      for (int i = 0; i < 32; i++, off += kStep_) {
        auto const& win = kDewin[i];
        sum             = win[0] * fifo[i + 704];
        sum += win[1] * fifo[i + 800];
        sum += win[2] * fifo[i + 832];
        sum += win[3] * fifo[i + 928];
        sum += win[4] * fifo[i + 960];
        sum += win[5] * fifo[i + 32];
        sum += win[6] * fifo[i + 64];
        sum += win[7] * fifo[i + 160];
        sum += win[8] * fifo[i + 192];
        sum += win[9] * fifo[i + 288];
        sum += win[10] * fifo[i + 320];
        sum += win[11] * fifo[i + 416];
        sum += win[12] * fifo[i + 448];
        sum += win[13] * fifo[i + 544];
        sum += win[14] * fifo[i + 576];
        sum += win[15] * fifo[i + 672];
        pcm_i        = sum > 32767 ? 32767 : (sum < -32768 ? -32768 : (int)sum);
        pcm_buf[off] = pcm_i;
        pcm_buf[off + 1] = (pcm_i >> 8u);
        if (pcm_i > max_pcm_) max_pcm_ = pcm_i;
      }
      break;
    case 768:
      // u_vector={768,864,896,992,0,96,128,224,256,352,384,480,512,608,640,736}
      for (int i = 0; i < 32; i++, off += kStep_) {
        auto const& win = kDewin[i];
        sum             = win[0] * fifo[i + 768];
        sum += win[1] * fifo[i + 864];
        sum += win[2] * fifo[i + 896];
        sum += win[3] * fifo[i + 992];
        sum += win[4] * fifo[i];
        sum += win[5] * fifo[i + 96];
        sum += win[6] * fifo[i + 128];
        sum += win[7] * fifo[i + 224];
        sum += win[8] * fifo[i + 256];
        sum += win[9] * fifo[i + 352];
        sum += win[10] * fifo[i + 384];
        sum += win[11] * fifo[i + 480];
        sum += win[12] * fifo[i + 512];
        sum += win[13] * fifo[i + 608];
        sum += win[14] * fifo[i + 640];
        sum += win[15] * fifo[i + 736];
        pcm_i        = sum > 32767 ? 32767 : (sum < -32768 ? -32768 : (int)sum);
        pcm_buf[off] = pcm_i;
        pcm_buf[off + 1] = (pcm_i >> 8u);
        if (pcm_i > max_pcm_) max_pcm_ = pcm_i;
      }
      break;
    case 832:
      // u_vector={832,928,960,32,64,160,192,288,320,416,448,544,576,672,704,800}
      for (int i = 0; i < 32; i++, off += kStep_) {
        auto const& win = kDewin[i];
        sum             = win[0] * fifo[i + 832];
        sum += win[1] * fifo[i + 928];
        sum += win[2] * fifo[i + 960];
        sum += win[3] * fifo[i + 32];
        sum += win[4] * fifo[i + 64];
        sum += win[5] * fifo[i + 160];
        sum += win[6] * fifo[i + 192];
        sum += win[7] * fifo[i + 288];
        sum += win[8] * fifo[i + 320];
        sum += win[9] * fifo[i + 416];
        sum += win[10] * fifo[i + 448];
        sum += win[11] * fifo[i + 544];
        sum += win[12] * fifo[i + 576];
        sum += win[13] * fifo[i + 672];
        sum += win[14] * fifo[i + 704];
        sum += win[15] * fifo[i + 800];
        pcm_i        = sum > 32767 ? 32767 : (sum < -32768 ? -32768 : (int)sum);
        pcm_buf[off] = pcm_i;
        pcm_buf[off + 1] = (pcm_i >> 8u);
        if (pcm_i > max_pcm_) max_pcm_ = pcm_i;
      }
      break;
    case 896:
      // u_vector={896,992,0,96,128,224,256,352,384,480,512,608,640,736,768,864}
      for (int i = 0; i < 32; i++, off += kStep_) {
        auto const& win = kDewin[i];
        sum             = win[0] * fifo[i + 896];
        sum += win[1] * fifo[i + 992];
        sum += win[2] * fifo[i];
        sum += win[3] * fifo[i + 96];
        sum += win[4] * fifo[i + 128];
        sum += win[5] * fifo[i + 224];
        sum += win[6] * fifo[i + 256];
        sum += win[7] * fifo[i + 352];
        sum += win[8] * fifo[i + 384];
        sum += win[9] * fifo[i + 480];
        sum += win[10] * fifo[i + 512];
        sum += win[11] * fifo[i + 608];
        sum += win[12] * fifo[i + 640];
        sum += win[13] * fifo[i + 736];
        sum += win[14] * fifo[i + 768];
        sum += win[15] * fifo[i + 864];
        pcm_i        = sum > 32767 ? 32767 : (sum < -32768 ? -32768 : (int)sum);
        pcm_buf[off] = pcm_i;
        pcm_buf[off + 1] = pcm_i >> 8u;
        if (pcm_i > max_pcm_) max_pcm_ = pcm_i;
      }
      break;
    case 960:
      // u_vector={960,32,64,160,192,288,320,416,448,544,576,672,704,800,832,928}
      for (int i = 0; i < 32; i++, off += kStep_) {
        auto const& win = kDewin[i];
        sum             = win[0] * fifo[i + 960];
        sum += win[1] * fifo[i + 32];
        sum += win[2] * fifo[i + 64];
        sum += win[3] * fifo[i + 160];
        sum += win[4] * fifo[i + 192];
        sum += win[5] * fifo[i + 288];
        sum += win[6] * fifo[i + 320];
        sum += win[7] * fifo[i + 416];
        sum += win[8] * fifo[i + 448];
        sum += win[9] * fifo[i + 544];
        sum += win[10] * fifo[i + 576];
        sum += win[11] * fifo[i + 672];
        sum += win[12] * fifo[i + 704];
        sum += win[13] * fifo[i + 800];
        sum += win[14] * fifo[i + 832];
        sum += win[15] * fifo[i + 928];
        pcm_i        = sum > 32767 ? 32767 : (sum < -32768 ? -32768 : (int)sum);
        pcm_buf[off] = pcm_i;
        pcm_buf[off + 1] = pcm_i >> 8u;
        if (pcm_i > max_pcm_) max_pcm_ = pcm_i;
      }
      break;
  }
  audio_buffer_.off_[ch] = off;
}
void Dct32To64(std::array<float, 32> const& src, std::array<float, 1024>& dest,
               int off) {
  auto& in  = src;
  auto& out = dest;
  int   i   = off;
  float in0, in1, in2, in3, in4, in5, in6, in7, in8, in9, in10, in11, in12,
      in13, in14, in15;
  float out0, out1, out2, out3, out4, out5, out6, out7, out8, out9, out10,
      out11, out12, out13, out14, out15;
  float d8_0, d8_1, d8_2, d8_3, d8_4, d8_5, d8_6, d8_7;
  float ein0, ein1, oin0, oin1;

  //>>>>>>>>>>>>>>>>
  // 用DCT16计算DCT32输出[0..31]的偶数下标元素
  in0  = in[0] + in[31];
  in1  = in[1] + in[30];
  in2  = in[2] + in[29];
  in3  = in[3] + in[28];
  in4  = in[4] + in[27];
  in5  = in[5] + in[26];
  in6  = in[6] + in[25];
  in7  = in[7] + in[24];
  in8  = in[8] + in[23];
  in9  = in[9] + in[22];
  in10 = in[10] + in[21];
  in11 = in[11] + in[20];
  in12 = in[12] + in[19];
  in13 = in[13] + in[18];
  in14 = in[14] + in[17];
  in15 = in[15] + in[16];

  // DCT16
  //{
  //>>>>>>>> 用DCT8计算DCT16输出[0..15]的偶数下标元素
  d8_0 = in0 + in15;
  d8_1 = in1 + in14;
  d8_2 = in2 + in13;
  d8_3 = in3 + in12;
  d8_4 = in4 + in11;
  d8_5 = in5 + in10;
  d8_6 = in6 + in9;
  d8_7 = in7 + in8;

  // DCT8. 加(减)法29,乘法12次
  //{
  //>>>>e 用DCT4计算DCT8的输出[0..7]的偶数下标元素
  out1 = d8_0 + d8_7;
  out3 = d8_1 + d8_6;
  out5 = d8_2 + d8_5;
  out7 = d8_3 + d8_4;

  //>>e DCT2
  ein0        = out1 + out7;
  ein1        = out3 + out5;
  out[i + 48] = -ein0 - ein1;
  out[i]      = (ein0 - ein1) * 0.7071068f;  // 0.5/cos(PI/4)

  //>>o DCT2
  oin0 = (out1 - out7) * 0.5411961f;  // 0.5/cos( PI/8)
  oin1 = (out3 - out5) * 1.3065630f;  // 0.5/cos(3PI/8)

  out2  = oin0 + oin1;
  out12 = (oin0 - oin1) * 0.7071068f;  // cos(PI/4)

  out[i + 40] = out[i + 56] = -out2 - out12;
  out[i + 8]                = out12;
  //<<<<e 完成计算DCT8的输出[0..7]的偶数下标元素

  //>>>>o 用DCT4计算DCT8的输出[0..7]的奇数下标元素
  // o DCT4 part1
  out1 = (d8_0 - d8_7) * 0.5097956f;  // 0.5/cos( PI/16)
  out3 = (d8_1 - d8_6) * 0.6013449f;  // 0.5/cos(3PI/16)
  out5 = (d8_2 - d8_5) * 0.8999762f;  // 0.5/cos(5PI/16)
  out7 = (d8_3 - d8_4) * 2.5629154f;  // 0.5/cos(7PI/16)

  // o DCT4 part2

  // e DCT2 part1
  ein0 = out1 + out7;
  ein1 = out3 + out5;

  // o DCT2 part1
  oin0 = (out1 - out7) * 0.5411961f;  // 0.5/cos(PI/8)
  oin1 = (out3 - out5) * 1.3065630f;  // 0.5/cos(3PI/8)

  // e DCT2 part2
  out1 = ein0 + ein1;
  out5 = (ein0 - ein1) * 0.7071068f;  // cos(PI/4)

  // o DCT2 part2
  out3 = oin0 + oin1;
  out7 = (oin0 - oin1) * 0.7071068f;  // cos(PI/4)
  out3 += out7;

  // o DCT4 part3
  out[i + 44] = out[i + 52] = -out1 - out3;  // out1+=out3
  out[i + 36] = out[i + 60] = -out3 - out5;  // out3+=out5
  out[i + 4]                = out5 + out7;   // out5+=out7
  out[i + 12]               = out7;
  //<<<<o 完成计算DCT8的输出[0..7]的奇数下标元素
  //}
  //<<<<<<<< 完成计算DCT16输出[0..15]的偶数下标元素

  //-----------------------------------------------------------------

  //>>>>>>>> 用DCT8计算DCT16输出[0..15]的奇数下标元素
  d8_0 = (in0 - in15) * 0.5024193f;  // 0.5/cos( 1 * PI/32)
  d8_1 = (in1 - in14) * 0.5224986f;  // 0.5/cos( 3 * PI/32)
  d8_2 = (in2 - in13) * 0.5669440f;  // 0.5/cos( 5 * PI/32)
  d8_3 = (in3 - in12) * 0.6468218f;  // 0.5/cos( 7 * PI/32)
  d8_4 = (in4 - in11) * 0.7881546f;  // 0.5/cos( 9 * PI/32)
  d8_5 = (in5 - in10) * 1.0606777f;  // 0.5/cos(11 * PI/32)
  d8_6 = (in6 - in9) * 1.7224471f;   // 0.5/cos(13 * PI/32)
  d8_7 = (in7 - in8) * 5.1011486f;   // 0.5/cos(15 * PI/32)

  // DCT8
  //{
  //>>>>e 用DCT4计算DCT8的输出[0..7]的偶数下标元素.
  out3  = d8_0 + d8_7;
  out7  = d8_1 + d8_6;
  out11 = d8_2 + d8_5;
  out15 = d8_3 + d8_4;

  //>>e DCT2
  ein0 = out3 + out15;
  ein1 = out7 + out11;
  out1 = ein0 + ein1;
  out9 = (ein0 - ein1) * 0.7071068f;  // 0.5/cos(PI/4)

  //>>o DCT2
  oin0 = (out3 - out15) * 0.5411961f;  // 0.5/cos( PI/8)
  oin1 = (out7 - out11) * 1.3065630f;  // 0.5/cos(3PI/8)

  out5  = oin0 + oin1;
  out13 = (oin0 - oin1) * 0.7071068f;  // cos(PI/4)

  out5 += out13;
  //<<<<e 完成计算DCT8的输出[0..7]的偶数下标元素

  //>>>>o 用DCT4计算DCT8的输出[0..7]的奇数下标元素
  // o DCT4 part1
  out3  = (d8_0 - d8_7) * 0.5097956f;  // 0.5/cos( PI/16)
  out7  = (d8_1 - d8_6) * 0.6013449f;  // 0.5/cos(3PI/16)
  out11 = (d8_2 - d8_5) * 0.8999762f;  // 0.5/cos(5PI/16)
  out15 = (d8_3 - d8_4) * 2.5629154f;  // 0.5/cos(7PI/16)

  // o DCT4 part2

  // e DCT2 part1
  ein0 = out3 + out15;
  ein1 = out7 + out11;

  // o DCT2 part1
  oin0 = (out3 - out15) * 0.5411961f;  // 0.5/cos(PI/8)
  oin1 = (out7 - out11) * 1.3065630f;  // 0.5/cos(3PI/8)

  // e DCT2 part2
  out3  = ein0 + ein1;
  out11 = (ein0 - ein1) * 0.7071068f;  // cos(PI/4)

  // o DCT2 part2
  out7  = oin0 + oin1;
  out15 = (oin0 - oin1) * 0.7071068f;  // cos(PI/4)
  out7 += out15;

  // o DCT4 part3
  out3 += out7;
  out7 += out11;
  out11 += out15;
  //<<<<o 完成计算DCT8的输出[0..7]的奇数下标元素
  //}

  out[i + 46] = out[i + 50] = -out1 - out3;   // out1 += out3
  out[i + 42] = out[i + 54] = -out3 - out5;   // out3 += out5
  out[i + 38] = out[i + 58] = -out5 - out7;   // out5 += out7
  out[i + 34] = out[i + 62] = -out7 - out9;   // out7 += out9
  out[i + 2]                = out9 + out11;   // out9 += out11
  out[i + 6]                = out11 + out13;  // out11 += out13
  out[i + 10]               = out13 + out15;  // out13 += out15
  //<<<<<<<< 完成计算DCT16输出[0..15]的奇数下标元素
  //}
  out[i + 14] = out15;  // out[i + 14]=out32[30]
  //<<<<<<<<<<<<<<<<
  // 完成计算DCT32输出[0..31]的偶数下标元素

  //=====================================================================

  //>>>>>>>>>>>>>>>>
  // 用DCT16计算DCT32输出[0..31]的奇数下标元素
  in0  = (in[0] - in[31]) * 0.5006030f;   // 0.5/cos( 1 * PI/64)
  in1  = (in[1] - in[30]) * 0.5054710f;   // 0.5/cos( 3 * PI/64)
  in2  = (in[2] - in[29]) * 0.5154473f;   // 0.5/cos( 5 * PI/64)
  in3  = (in[3] - in[28]) * 0.5310426f;   // 0.5/cos( 7 * PI/64)
  in4  = (in[4] - in[27]) * 0.5531039f;   // 0.5/cos( 9 * PI/64)
  in5  = (in[5] - in[26]) * 0.5829350f;   // 0.5/cos(11 * PI/64)
  in6  = (in[6] - in[25]) * 0.6225041f;   // 0.5/cos(13 * PI/64)
  in7  = (in[7] - in[24]) * 0.6748083f;   // 0.5/cos(15 * PI/64)
  in8  = (in[8] - in[23]) * 0.7445362f;   // 0.5/cos(17 * PI/64)
  in9  = (in[9] - in[22]) * 0.8393496f;   // 0.5/cos(19 * PI/64)
  in10 = (in[10] - in[21]) * 0.9725682f;  // 0.5/cos(21 * PI/64)
  in11 = (in[11] - in[20]) * 1.1694399f;  // 0.5/cos(23 * PI/64)
  in12 = (in[12] - in[19]) * 1.4841646f;  // 0.5/cos(25 * PI/64)
  in13 = (in[13] - in[18]) * 2.0577810f;  // 0.5/cos(27 * PI/64)
  in14 = (in[14] - in[17]) * 3.4076084f;  // 0.5/cos(29 * PI/64)
  in15 = (in[15] - in[16]) * 10.190008f;  // 0.5/cos(31 * PI/64)

  // DCT16
  //{
  //>>>>>>>> 用DCT8计算DCT16输出[0..15]的偶数下标元素
  d8_0 = in0 + in15;
  d8_1 = in1 + in14;
  d8_2 = in2 + in13;
  d8_3 = in3 + in12;
  d8_4 = in4 + in11;
  d8_5 = in5 + in10;
  d8_6 = in6 + in9;
  d8_7 = in7 + in8;

  // DCT8
  //{
  //>>>>e 用DCT4计算DCT8的输出[0..7]的偶数下标元素
  out1 = d8_0 + d8_7;
  out3 = d8_1 + d8_6;
  out5 = d8_2 + d8_5;
  out7 = d8_3 + d8_4;

  //>>e DCT2
  ein0 = out1 + out7;
  ein1 = out3 + out5;
  out0 = ein0 + ein1;
  out8 = (ein0 - ein1) * 0.7071068f;  // 0.5/cos(PI/4)

  //>>o DCT2
  oin0 = (out1 - out7) * 0.5411961f;  // 0.5/cos( PI/8)
  oin1 = (out3 - out5) * 1.3065630f;  // 0.5/cos(3PI/8)

  out4  = oin0 + oin1;
  out12 = (oin0 - oin1) * 0.7071068f;  // cos(PI/4)

  out4 += out12;
  //<<<<e 完成计算DCT8的输出[0..7]的偶数下标元素

  //>>>>o 用DCT4计算DCT8的输出[0..7]的奇数下标元素
  // o DCT4 part1
  out1 = (d8_0 - d8_7) * 0.5097956f;  // 0.5/cos( PI/16)
  out3 = (d8_1 - d8_6) * 0.6013449f;  // 0.5/cos(3PI/16)
  out5 = (d8_2 - d8_5) * 0.8999762f;  // 0.5/cos(5PI/16)
  out7 = (d8_3 - d8_4) * 2.5629154f;  // 0.5/cos(7PI/16)

  // o DCT4 part2

  // e DCT2 part1
  ein0 = out1 + out7;
  ein1 = out3 + out5;

  // o DCT2 part1
  oin0 = (out1 - out7) * 0.5411961f;  // 0.5/cos(PI/8)
  oin1 = (out3 - out5) * 1.3065630f;  // 0.5/cos(3PI/8)

  // e DCT2 part2
  out2  = ein0 + ein1;
  out10 = (ein0 - ein1) * 0.7071068f;  // cos(PI/4)

  // o DCT2 part2
  out6  = oin0 + oin1;
  out14 = (oin0 - oin1) * 0.7071068f;
  out6 += out14;

  // o DCT4 part3
  out2 += out6;
  out6 += out10;
  out10 += out14;
  //<<<<o 完成计算DCT8的输出[0..7]的奇数下标元素
  //}
  //<<<<<<<< 完成计算DCT16输出[0..15]的偶数下标元素

  //-----------------------------------------------------------------

  //>>>>>>>> 用DCT8计算DCT16输出[0..15]的奇数下标元素
  d8_0 = (in0 - in15) * 0.5024193f;  // 0.5/cos( 1 * PI/32)
  d8_1 = (in1 - in14) * 0.5224986f;  // 0.5/cos( 3 * PI/32)
  d8_2 = (in2 - in13) * 0.5669440f;  // 0.5/cos( 5 * PI/32)
  d8_3 = (in3 - in12) * 0.6468218f;  // 0.5/cos( 7 * PI/32)
  d8_4 = (in4 - in11) * 0.7881546f;  // 0.5/cos( 9 * PI/32)
  d8_5 = (in5 - in10) * 1.0606777f;  // 0.5/cos(11 * PI/32)
  d8_6 = (in6 - in9) * 1.7224471f;   // 0.5/cos(13 * PI/32)
  d8_7 = (in7 - in8) * 5.1011486f;   // 0.5/cos(15 * PI/32)

  // DCT8
  //{
  //>>>>e 用DCT4计算DCT8的输出[0..7]的偶数下标元素.
  out1 = d8_0 + d8_7;
  out3 = d8_1 + d8_6;
  out5 = d8_2 + d8_5;
  out7 = d8_3 + d8_4;

  //>>e DCT2
  ein0 = out1 + out7;
  ein1 = out3 + out5;
  in0  = ein0 + ein1;                 // out0->in0,out4->in4
  in4  = (ein0 - ein1) * 0.7071068f;  // 0.5/cos(PI/4)

  //>>o DCT2
  oin0 = (out1 - out7) * 0.5411961f;  // 0.5/cos( PI/8)
  oin1 = (out3 - out5) * 1.3065630f;  // 0.5/cos(3PI/8)

  in2 = oin0 + oin1;                 // out2->in2,out6->in6
  in6 = (oin0 - oin1) * 0.7071068f;  // cos(PI/4)

  in2 += in6;
  //<<<<e 完成计算DCT8的输出[0..7]的偶数下标元素

  //>>>>o 用DCT4计算DCT8的输出[0..7]的奇数下标元素
  // o DCT4 part1
  out1 = (d8_0 - d8_7) * 0.5097956f;  // 0.5/cos( PI/16)
  out3 = (d8_1 - d8_6) * 0.6013449f;  // 0.5/cos(3PI/16)
  out5 = (d8_2 - d8_5) * 0.8999762f;  // 0.5/cos(5PI/16)
  out7 = (d8_3 - d8_4) * 2.5629154f;  // 0.5/cos(7PI/16)

  // o DCT4 part2

  // e DCT2 part1
  ein0 = out1 + out7;
  ein1 = out3 + out5;

  // o DCT2 part1
  oin0 = (out1 - out7) * 0.5411961f;  // 0.5/cos(PI/8)
  oin1 = (out3 - out5) * 1.3065630f;  // 0.5/cos(3PI/8)

  // e DCT2 part2
  out1 = ein0 + ein1;
  out5 = (ein0 - ein1) * 0.7071068f;  // cos(PI/4)

  // o DCT2 part2
  out3  = oin0 + oin1;
  out15 = (oin0 - oin1) * 0.7071068f;
  out3 += out15;

  // o DCT4 part3
  out1 += out3;
  out3 += out5;
  out5 += out15;
  //<<<<o 完成计算DCT8的输出[0..7]的奇数下标元素
  //}
  // out15=out7
  out13 = in6 + out15;  // out13=out6+out7
  out11 = out5 + in6;   // out11=out5+out6
  out9  = in4 + out5;   // out9 =out4+out5
  out7  = out3 + in4;   // out7 =out3+out4
  out5  = in2 + out3;   // out5 =out2+out3
  out3  = out1 + in2;   // out3 =out1+out2
  out1 += in0;          // out1 =out0+out1
  //<<<<<<<< 完成计算DCT16输出[0..15]的奇数下标元素
  //}

  // DCT32out[i]=out[i]+out[i+1]; DCT32out[31]=out[15]
  out[i + 47] = out[i + 49] = -out0 - out1;
  out[i + 45] = out[i + 51] = -out1 - out2;
  out[i + 43] = out[i + 53] = -out2 - out3;
  out[i + 41] = out[i + 55] = -out3 - out4;
  out[i + 39] = out[i + 57] = -out4 - out5;
  out[i + 37] = out[i + 59] = -out5 - out6;
  out[i + 35] = out[i + 61] = -out6 - out7;
  out[i + 33] = out[i + 63] = -out7 - out8;
  out[i + 1]                = out8 + out9;
  out[i + 3]                = out9 + out10;
  out[i + 5]                = out10 + out11;
  out[i + 7]                = out11 + out12;
  out[i + 9]                = out12 + out13;
  out[i + 11]               = out13 + out14;
  out[i + 13]               = out14 + out15;
  out[i + 15]               = out15;
  //<<<<<<<<<<<<<<<<

  out[i + 16] = 0;

  out[i + 17] = -out15;  // out[i + 17] = -out[i + 15]
  out[i + 18] = -out[i + 14];
  out[i + 19] = -out[i + 13];
  out[i + 20] = -out[i + 12];
  out[i + 21] = -out[i + 11];
  out[i + 22] = -out[i + 10];
  out[i + 23] = -out[i + 9];
  out[i + 24] = -out[i + 8];
  out[i + 25] = -out[i + 7];
  out[i + 26] = -out[i + 6];
  out[i + 27] = -out[i + 5];
  out[i + 28] = -out[i + 4];
  out[i + 29] = -out[i + 3];
  out[i + 30] = -out[i + 2];
  out[i + 31] = -out[i + 1];
  out[i + 32] = -out[i];
}
}  // namespace jmp123::decoder