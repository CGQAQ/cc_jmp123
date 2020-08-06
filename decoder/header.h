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

#ifndef JMP123_HEADER_H
#define JMP123_HEADER_H

#include <fmt/core.h>

#include <array>
#include <iostream>
#include <sstream>
#include <string>

namespace jmp123::decoder {

enum class MPEGVersion : int {
  kMPEG1    = 3,
  kMPEG2    = 2,
  kReserved = 1,
  kMPEG25   = 0,
};

constexpr std::array<std::array<std::array<int, 15>, 3>, 2> kBitrateTable{
    std::to_array<std::array<int, 15>, 3>(
        {// MPEG-1
         // Layer I
         {0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448},
         // Layer II
         {0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384},
         // Layer III
         {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320}}),
    std::to_array<std::array<int, 15>, 3>(
        {// MPEG-2/2.5
         // Layer I
         {0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256},
         // Layer II
         {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160},
         // Layer III
         {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160}})};

constexpr std::array<std::array<int, 4>, 4> kSamplingRateTable{
    std::to_array({11025, 12000, 8000, 0}),
    std::to_array({0, 0, 0, 0}),
    std::to_array({22050, 24000, 16000, 0}),
    std::to_array({44100, 48000, 32000, 0}),
};

class Header {
 private:
  MPEGVersion ver_id_;
  int         layer_;
  int         protection_bit_;
  int         bitrate_index_;
  int         sampling_frequency_;
  int         padding_bit_;
  int         mode_;
  int         mode_extension_;

  int  frame_size_;
  int  main_data_size_;
  int  side_info_size_;
  int  lsf_;
  int  header_mask_;
  bool is_MS_;
  bool is_intensity_;
  bool sync_;

 private:
  long  track_length_;
  long  track_frames_;
  float frame_duration_;
  float duration_;
  int   frame_counter_;

  std::stringstream            vbr_info_;
  std::unique_ptr<uint8_t[]>   vbr_toc_;
  int                          toc_number_, toc_per_, toc_factor_;
  std::unique_ptr<std::string> str_bitrate_;

  std::stringstream progress_;
  int               progress_index_;

 public:
  void Initialize(long track_length, int duration);

  void ParseHeader(int h);

  [[nodiscard]] int Byte_2_Int(uint8_t b[], int off);

  [[nodiscard]] int Byte_2_Short(uint8_t b[], int off);

  bool Available(int h, int curmask);

 private:
  int idx_;

 public:
  [[nodiscard]] int           Offset() const;
  bool                        SyncFrame(uint8_t* b, int off, int end_pos);
  [[nodiscard]] bool          IsMS() const;
  [[nodiscard]] bool          IsIntensityStereo() const;
  [[nodiscard]] constexpr int GetBitrate() const;
  [[nodiscard]] constexpr int GetBitrateIndex() const;
  [[nodiscard]] constexpr int GetChannelCount() const;
  [[nodiscard]] constexpr int GetMode() const;
  [[nodiscard]] constexpr int GetModeExtension() const;
  [[nodiscard]] constexpr int GetVersion() const;
  [[nodiscard]] constexpr int GetLayer() const;
  [[nodiscard]] constexpr int GetSamplingFrequency() const;
  [[nodiscard]] constexpr int GetSamplingRate() const;
  [[nodiscard]] constexpr int GetMainDataSize() const;
  [[nodiscard]] constexpr int GetSideInfoSize() const;
  [[nodiscard]] constexpr int GetFrameSize() const;
  [[nodiscard]] constexpr int GetPcmSize() const;
  [[nodiscard]] constexpr int GetTrackLength() const;
  [[nodiscard]] constexpr int GetFrameCounter() const;
  [[nodiscard]] constexpr int GetTrackFrames() const;
  [[nodiscard]] constexpr int GetFrameDuration() const;
  [[nodiscard]] constexpr int GetDuration() const;
  [[nodiscard]] constexpr int GetElapse() const;
  [[nodiscard]] std::string   GetVBRInfo() const;

 private:
  void ParseVBR(uint8_t b[], int off, int len);

  int VBRIHeader(uint8_t b[], int off, int len);

  int XingInfoHeader(uint8_t b[], int off, int len);

 public:
  void PrintHeaderInfo();
  void PrintVBRTag() const;
  void PrintProgress();
};
}  // namespace jmp123::decoder

#endif  // JMP123_HEADER_H
