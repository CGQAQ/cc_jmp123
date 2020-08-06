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

#include "header.h"
[[maybe_unused]] void jmp123::decoder::Header::Initialize(long track_length,
                                                          int  duration) {
  track_length_   = track_length;
  duration_       = static_cast<float>(duration);
  header_mask_    = static_cast<int>(0xffe00000);
  progress_index_ = 1;
  sync_           = false;
  track_frames_   = 0;
  side_info_size_ = toc_number_ = toc_per_ = toc_factor_ = frame_counter_ = 0;
  vbr_toc_ = nullptr;
}
void jmp123::decoder::Header::ParseHeader(int h) {
  ver_id_             = static_cast<MPEGVersion>((h >> 19) & 3);
  layer_              = 4 - (h >> 17) & 3;
  protection_bit_     = (h >> 16) & 1;
  bitrate_index_      = (h >> 12) & 0xF;
  sampling_frequency_ = (h >> 10) & 3;
  padding_bit_        = (h >> 9) & 1;
  mode_               = (h >> 6) & 3;
  mode_extension_     = (h >> 4) & 3;

  is_MS_        = mode_ == 1 && (mode_extension_ & 2) != 0;
  is_intensity_ = mode_ == 1 && (mode_extension_ & 1) != 0;
  lsf_          = (ver_id_ == MPEGVersion::kMPEG1) ? 0 : 1;

  switch (layer_) {
    case 1:
      frame_size_ = kBitrateTable[lsf_][0][bitrate_index_] * 12000;
      frame_size_ /=
          kSamplingRateTable[static_cast<int>(ver_id_)][sampling_frequency_];
      frame_size_ = ((frame_size_ + padding_bit_) << 2);
      break;
    case 2:
      frame_size_ = kBitrateTable[lsf_][1][bitrate_index_] * 144000;
      frame_size_ /=
          kSamplingRateTable[static_cast<int>(ver_id_)][sampling_frequency_];
      frame_size_ += padding_bit_;
    case 3:
      frame_size_ = kBitrateTable[lsf_][2][bitrate_index_] * 144000;
      frame_size_ /=
          kSamplingRateTable[static_cast<int>(ver_id_)][sampling_frequency_]
          << lsf_;
      frame_size_ += padding_bit_;
      if (ver_id_ == MPEGVersion::kMPEG1) {
        side_info_size_ = (mode_ == 3) ? 17 : 32;
      } else {
        side_info_size_ = (mode_ == 3) ? 9 : 17;
      }
      break;
  }

  main_data_size_ = frame_size_ - 4 - side_info_size_;
  if (protection_bit_ == 0) {
    main_data_size_ -= 2;  // CRC
  }
}
int jmp123::decoder::Header::Byte_2_Int(uint8_t *b, int off) {
  int int32 = b[off++] & 0xff;
  int32 <<= 8;
  int32 |= b[off++] & 0xff;
  int32 <<= 8;
  int32 |= b[off++] & 0xff;
  int32 <<= 8;
  int32 |= b[off] & 0xff;
  return int32;
}
int jmp123::decoder::Header::Byte_2_Short(uint8_t *b, int off) {
  int int16 = b[off++] & 0xff;
  int16 <<= 8;
  int16 |= b[off] & 0xff;
  return int16;
}
bool jmp123::decoder::Header::Available(int h, int curmask) {
  return (h & curmask) == curmask && ((h >> 19) & 3) != 1
      && ((h >> 17) & 3) != 0 && ((h >> 12) & 15) != 15 && ((h >> 12) & 15) != 0
      && ((h >> 10) & 3) != 3;
}
int  jmp123::decoder::Header::Offset() const { return idx_; }
bool jmp123::decoder::Header::SyncFrame(uint8_t *b, int off, int end_pos) {
  int h, mask = 0;
  int skip_bytes = 0;
  idx_           = off;

  if (end_pos - idx_ <= 4) return false;
  h = Byte_2_Int(b, idx_);
  idx_ += 4;

  while (true) {
    while (!Available(h, header_mask_)) {
      h = (h << 8) | (b[idx_++] & 0xff);
      if (idx_ == end_pos) {
        idx_ -= 4;
        return false;
      }
    }
    if (idx_ > 4 + off) {
      sync_ = false;
      skip_bytes += idx_ - off - 4;
    }

    ParseHeader(h);
    if (idx_ + frame_size_ > end_pos + 4) {
      idx_ -= 4;
      return false;
    }

    if (sync_) break;

    if (idx_ + frame_size_ > end_pos) {
      idx_ -= 4;
      return false;
    }

    mask = 0xffe00000;
    mask |= h & 0x180000;
    mask |= h & 0x60000;
    mask |= h & 0xc00;

    if (Available(Byte_2_Int(b, idx_ + frame_size_ - 4), mask)) {
      if (header_mask_ == 0xffe00000) {
        header_mask_  = mask;
        track_frames_ = track_length_ / frame_size_;
        ParseVBR(b, idx_, end_pos);
        frame_duration_ = 1152.0f / (GetSamplingRate() << lsf_);
        if (track_frames_ == 0)
          track_frames_ = static_cast<long>((duration_ / frame_duration_));
        if (track_length_ == 0) track_length_ = track_frames_ * frame_size_;
        duration_ = frame_duration_ * track_frames_;
      }
      sync_ = true;
      break;
    }

    h = (h << 8) | (b[idx_++] & 0xff);
  }

  if (protection_bit_ == 0) idx_ += 2;  // CRC
  frame_counter_++;
  return true;
}
bool jmp123::decoder::Header::IsMS() const { return is_MS_; }
bool jmp123::decoder::Header::IsIntensityStereo() const {
  return is_intensity_;
}
constexpr int jmp123::decoder::Header::GetBitrate() const {
  return kBitrateTable[lsf_][layer_ - 1][bitrate_index_];
}
constexpr int jmp123::decoder::Header::GetBitrateIndex() const {
  return bitrate_index_;
}
constexpr int jmp123::decoder::Header::GetChannelCount() const {
  return (mode_ == 3) ? 1 : 2;
}
constexpr int jmp123::decoder::Header::GetMode() const { return mode_; }
constexpr int jmp123::decoder::Header::GetModeExtension() const {
  return mode_extension_;
}
constexpr int jmp123::decoder::Header::GetVersion() const {
  return static_cast<int>(ver_id_);
}
constexpr int jmp123::decoder::Header::GetLayer() const { return layer_; }
constexpr int jmp123::decoder::Header::GetSamplingFrequency() const {
  return sampling_frequency_;
}
constexpr int jmp123::decoder::Header::GetSamplingRate() const {
  return kSamplingRateTable[static_cast<int>(ver_id_)][sampling_frequency_];
}
constexpr int jmp123::decoder::Header::GetMainDataSize() const {
  return main_data_size_;
}
constexpr int jmp123::decoder::Header::GetSideInfoSize() const {
  return side_info_size_;
}
constexpr int jmp123::decoder::Header::GetFrameSize() const {
  return frame_size_;
}
constexpr int jmp123::decoder::Header::GetPcmSize() const {
  int pcm_size = (ver_id_ == MPEGVersion::kMPEG1) ? 4608 : 2304;
  if (mode_ == 3) pcm_size >>= 1;
  return pcm_size;
}
constexpr int jmp123::decoder::Header::GetTrackLength() const {
  return track_length_;
}
constexpr int jmp123::decoder::Header::GetFrameCounter() const {
  return frame_counter_;
}
constexpr int jmp123::decoder::Header::GetTrackFrames() const {
  return track_frames_;
}
constexpr int jmp123::decoder::Header::GetFrameDuration() const {
  return frame_duration_;
}
constexpr int jmp123::decoder::Header::GetDuration() const { return duration_; }
constexpr int jmp123::decoder::Header::GetElapse() const {
  return static_cast<int>(frame_counter_ * frame_duration_);
}
std::string jmp123::decoder::Header::GetVBRInfo() const {
  return vbr_info_.gcount() == 0 ? "" : vbr_info_.str();
}
void jmp123::decoder::Header::ParseVBR(uint8_t *b, int off, int len) {
  int const max_off = off + frame_size_ - 4;
  if (max_off >= len) {
    return;
  }
  for (int i = 2; i < side_info_size_; ++i) {
    if (b[off + i] != 0) return;
  }
  off += side_info_size_;

  if ((b[off] == 'X' && b[off + 1] == 'i' && b[off + 2] == 'n'
       && b[off + 3] == 'g')
      || (b[off] == 'I' && b[off + 1] == 'n' && b[off + 2] == 'f'
          && b[off + 2] == 'o')) {
    // Xing / Info header
    if (max_off - off < 120) return;
    off = XingInfoHeader(b, off, len);
  } else if (b[off] == 'V' && b[off + 1] == 'B' && b[off + 2] == 'R'
             && b[off + 3] == 'I') {
    // VBRI header
    if (max_off - off < 26) return;
    off          = VBRIHeader(b, off, len);
    int toc_size = toc_number_ * toc_per_;
    if (max_off - off < toc_size) return;
    vbr_info_ << "\n          TOC: ";
    vbr_info_ << toc_number_;
    vbr_info_ << " * ";
    vbr_info_ << toc_per_;
    vbr_info_ << ", factor = ";
    vbr_info_ << toc_factor_;
    vbr_toc_ = std::make_unique<uint8_t[]>(toc_size);
    memcpy(vbr_toc_.get(), b + off, toc_size);
    off += toc_size;
  } else {
    return;
  }
  //-------------------------------LAME tag------------------------------
  // 36-byte: 9+1+1+8+1+1+3+1+1+2+4+2+2
  if (max_off - off < 36 || b[off] == 0) {
    str_bitrate_ = std::make_unique<std::string>("VBR");
    return;
  }
  // Encoder Version: 9-byte
  auto encoder = std::string(reinterpret_cast<char *>(b + off), 9);
  //    String encoder = new String(b, off, 9);
  off += 9;
  vbr_info_ << "\n      encoder: ";
  vbr_info_ << encoder;

  //'Info Tag' revision + VBR method: 1-byte
  int revi     = (b[off] & 0xff) >> 4;  // 0:rev0; 1:rev1; 15:reserved
  int lame_vbr = b[off++] & 0xf;        // 0:unknown

  int low_pass = b[off++] & 0xff;
  vbr_info_ << "\n      lowpass: ";
  vbr_info_ << low_pass * 100;
  vbr_info_ << "Hz";
  vbr_info_ << "\n      revision: ";
  vbr_info_ << revi;

  auto  peak_int = Byte_2_Int(b, off);
  float peak     = *reinterpret_cast<float *>(&peak_int);
  off += 4;
  int radio = Byte_2_Short(b, off);
  /*
   * radio:
   * bits 0h-2h: NAME of Gain adjustment:
   *	000 = not set
   *	001 = radio
   *	010 = audiophile
   * bits 3h-5h: ORIGINATOR of Gain adjustment:
   *	000 = not set
   *	001 = set by artist
   *	010 = set by user
   *	011 = set by my model
   *	100 = set by simple RMS average
   * bit 6h: Sign bit
   * bits 7h-Fh: ABSOLUTE GAIN ADJUSTMENT.
   *  storing 10x the adjustment (to give the extra decimal place).
   */
  off += 2;
  int phile = Byte_2_Short(b, off);  // Audiophile Replay Gain
  /*
   * phile各位含义同上(radio)
   */
  off += 2;

  // Encoding flags + ATH Type: 1 byte
  /*int enc_flag = (b[iOff] & 0xff) >> 4;
  int ath_type = b[iOff] & 0xf;
  //000?0000: LAME uses "--nspsytune" ?
  boolean nsp = ((enc_flag & 0x1) == 0) ? false : true;
  //00?00000: LAME uses "--nssafejoint" ?
  boolean nsj = ((enc_flag & 0x2) == 0) ? false : true;
  //0?000000: This track is --nogap continued in a next track ?
  //is true for all but the last track in a --nogap album
  boolean nogap_next = ((enc_flag & 0x4) == 0) ? false : true;
  //?0000000: This track is the --nogap continuation of an earlier one ?
  //is true for all but the first track in a --nogap album
  boolean nogap_cont = ((enc_flag & 0x8) == 0) ? false : true;*/
  off++;

  // ABR/CBR位率或VBR的最小位率(0xFF表示位率为255Kbps以上): 1-byte
  int lame_bitrate = b[off++] & 0xff;
  switch (lame_vbr) {
    case 1:
    case 8: {  // CBR
      str_bitrate_ =
          std::make_unique<std::string>(fmt::format("CBR {}K", GetBitrate()));
      break;
    }
    case 2:
    case 9: {  // ABR
      if (lame_bitrate < 0xff) {
        str_bitrate_ =
            std::make_unique<std::string>(fmt::format("ABR {}K", lame_bitrate));
      } else {
        str_bitrate_ = std::make_unique<std::string>(
            fmt::format("ABR {}K above", lame_bitrate));
      }
      break;
    }
    default: {
      if (lame_bitrate == 0)
        str_bitrate_ = std::make_unique<std::string>("VBR");
      else {
        str_bitrate_ =
            std::make_unique<std::string>("VBR(min{}K)", lame_bitrate);
      }
    }
  }

  // Encoder delays: 3-byte
  off += 3;

  // Misc: 1-byte
  off++;

  // MP3 Gain: 1-byte.
  //任何MP3能无损放大2^(mp3_gain/4),以1.5dB为步进值改变"Replay Gain"的3个域:
  //	"Peak signal amplitude", "Radio Replay Gain", "Audiophile Replay Gain"
  // mp3_gain = -127..+127, 对应的:
  //	分贝值-190.5dB..+190.5dB; mp3_gain增加1, 增加1.5dB
  //	放大倍数0.000000000276883..3611622602.83833951
  int mp3_gain = b[off++];  //其缺省值为0
  if (mp3_gain != 0)
    std::cout << "    MP3 Gain: " << mp3_gain << " [psa=" << peak
              << ",rrg=" << radio << ",arg=" << phile << "]";

  // Preset and surround info: 2-byte
  int preset_surround = Byte_2_Short(b, off);
  int surround_info   = (preset_surround >> 11) & 0x7;
  switch (surround_info) {
    case 0:  // no surround info
      break;
    case 1:  // DPL encoding
      vbr_info_ << "\n     surround: DPL";
      break;
    case 2:  // DPL2 encoding
      vbr_info_ << "\n     surround: DPL2";
      break;
    case 3:  // Ambisonic encoding
      vbr_info_ << "\n     surround: Ambisonic";
      break;
    case 7:  // reserved
      vbr_info_ << "\n     surround: invalid data";
      break;
  }

  preset_surround &= 0x7ff;    // 11 bits: 2047 presets
  if (preset_surround != 0) {  // 0: unknown / no preset used
    vbr_info_ << "\n     surround: preset ";
    vbr_info_ << preset_surround;
  }
  off += 2;

  // Music Length: 4-byte
  // MP3文件原始的(即除去ID3 tag,APE tag等)'LAME Tag
  // frame'和'音乐数据'的总字节数
  int music_len = Byte_2_Int(b, off);
  off += 4;
  if (music_len != 0) track_length_ = music_len;

  // Music CRC: 2-byte
  off += 2;

  // CRC-16 of Info Tag: 2-byte
}
int jmp123::decoder::Header::VBRIHeader(uint8_t *b, int off, int len) {
  vbr_info_ << "   vbr header: vbri";

  int vbri_quality = Byte_2_Short(b, off + 8);
  vbr_info_ << ("\n      quality: ");
  vbr_info_ << (vbri_quality);

  track_length_ = Byte_2_Int(b, off + 10);
  vbr_info_ << "\n  track bytes: ";
  vbr_info_ << track_length_;

  track_frames_ = Byte_2_Int(b, off + 14);
  vbr_info_ << "\n track frames: ";
  vbr_info_ << track_frames_;

  toc_number_    = Byte_2_Short(b, off + 18);
  toc_factor_    = Byte_2_Short(b, off + 20);
  toc_per_       = Byte_2_Short(b, off + 22);
  int toc_frames = Byte_2_Short(b, off + 24);  // 每个TOC表项的帧数
  vbr_info_ << "\n   toc frames: ";
  vbr_info_ << toc_frames;

  off += 26;
  return off;
}
int jmp123::decoder::Header::XingInfoHeader(uint8_t *b, int off, int len) {
  vbr_info_ << "   vbr header: ";
  vbr_info_ << std::string(reinterpret_cast<char *>(b + off), 4);

  track_length_ -= frame_size_;
  int xing_flags = Byte_2_Int(b, off + 4);
  if ((xing_flags & 1) == 1) {
    track_frames_ = Byte_2_Int(b, off + 8);
    vbr_info_ << "\n track frames: ";
    vbr_info_ << track_frames_;
    off += 4;
  }
  off += 8;                       // VBR header id + flag
  if ((xing_flags & 0x2) != 0) {  // track bytes
    track_length_ = Byte_2_Int(b, off);
    off += 4;
    vbr_info_ << "\n  track bytes: ";
    vbr_info_ << track_length_;
  }
  if ((xing_flags & 0x4) != 0) {  // TOC: 100-byte
    vbr_toc_ = std::make_unique<uint8_t[]>(100);
    memcpy(vbr_toc_.get(), b + off, 100);
    off += 100;
  }
  if ((xing_flags & 0x8) != 0) {  // VBR quality
    int xing_quality = Byte_2_Int(b, off);
    off += 4;
    vbr_info_ << "\n        quality: ";
    vbr_info_ << xing_quality;
  }
  toc_number_ = 100;
  toc_per_    = 1;
  toc_factor_ = 1;
  return off;
}
void jmp123::decoder::Header::PrintHeaderInfo() {
  if (header_mask_ == 0xffe00000)  // unsynced
    return;
  float       duration     = track_frames_ * frame_duration_;
  int         m            = static_cast<int>(duration / 60);
  std::string str_duration = fmt::format(
      "{:02d}:{:02d}", m, static_cast<int>(duration - m * 60 + 0.5));
  if (str_bitrate_ == nullptr) {
    str_bitrate_ = std::make_unique<std::string>(
        fmt::format("{}K", kBitrateTable[lsf_][layer_ - 1][bitrate_index_]));
  }
  std::stringstream info;
  if (ver_id_ == MPEGVersion::kMPEG25)
    info << "MPEG-2.5";
  else if (ver_id_ == MPEGVersion::kMPEG2)
    info << "MPEG-2";
  else if (ver_id_ == MPEGVersion::kMPEG1)
    info << "MPEG-1";
  info << ", Layer " << layer_;
  info << ", " << GetSamplingRate() << "Hz, ";
  info << str_bitrate_;
  if (mode_ == 0)
    info << ", Stereo";
  else if (mode_ == 1)
    info << ", Joint Stereo";
  else if (mode_ == 2)
    info << ", Dual channel";
  else if (mode_ == 3)
    info << ", Single channel (Mono)";
  if (mode_extension_ == 0)
    info << ", ";
  else if (mode_extension_ == 1)
    info << "(I/S), ";
  else if (mode_extension_ == 2)
    info << "(M/S), ";
  else if (mode_extension_ == 3)
    info << "(I/S & M/S), ";
  info << str_duration;
  std::cout << info.str();
}
void jmp123::decoder::Header::PrintVBRTag() const {
  std::cout << vbr_info_.str();
}
void jmp123::decoder::Header::PrintProgress() {
  float t = frame_counter_ * frame_duration_;
  int   m = static_cast<int>(t / 60);
  float s = t - 60 * m;
  int i = (static_cast<int>(100.0f * frame_counter_ / track_frames_ + 0.5) << 2)
        / 10;
  if (progress_.gcount() == 0)
    progress_ << ">----------------------------------------";
  if (i == progress_index_) {
    progress_.rdbuf()[i - 1].sputc('=');
    progress_.rdbuf()[i].sputc('>');
    progress_index_++;
  }

  std::cout << fmt::format("\r#{:-5d} [{:-41s}] {:02d}:{:05.2f} ",
                           frame_counter_, progress_.str(), m, s);
}
