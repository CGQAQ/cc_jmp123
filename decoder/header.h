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
  [[nodiscard]] int         Offset() const;
  bool                      SyncFrame(uint8_t* b, int off, int end_pos);
  [[nodiscard]] bool        IsMS() const;
  [[nodiscard]] bool        IsIntensityStereo() const;
  [[nodiscard]] int         GetBitrate() const;
  [[nodiscard]] int         GetBitrateIndex() const;
  [[nodiscard]] int         GetChannelCount() const;
  [[nodiscard]] int         GetMode() const;
  [[nodiscard]] int         GetModeExtension() const;
  [[nodiscard]] int         GetVersion() const;
  [[nodiscard]] int         GetLayer() const;
  [[nodiscard]] int         GetSamplingFrequency() const;
  [[nodiscard]] int         GetSamplingRate() const;
  [[nodiscard]] int         GetMainDataSize() const;
  [[nodiscard]] int         GetSideInfoSize() const;
  [[nodiscard]] int         GetFrameSize() const;
  [[nodiscard]] int         GetPcmSize() const;
  [[nodiscard]] int         GetTrackLength() const;
  [[nodiscard]] int         GetFrameCounter() const;
  [[nodiscard]] int         GetTrackFrames() const;
  [[nodiscard]] int         GetFrameDuration() const;
  [[nodiscard]] int         GetDuration() const;
  [[nodiscard]] int         GetElapse() const;
  [[nodiscard]] std::string GetVBRInfo() const;

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
