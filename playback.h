//
// Created by jason on 2020/8/9.
//

#ifndef JMP123_PLAYBACK_H
#define JMP123_PLAYBACK_H

#include <decoder/audio_interface.h>

#include <filesystem>
#include <fstream>

#include "decoder/header.h"
#include "decoder/layer123.h"
#include "decoder/layer_1.h"
#include "decoder/layer_2.h"
#include "decoder/layer_3.h"

namespace jmp123 {
class Playback {
  constexpr static int BUFLEN = 8192;
  std::vector<uint8_t> buf{};
  bool                 eof{false}, paused{false};
  //  RandomRead instream;
  std::ifstream instream;
  //  ID3Tag id3tag;
  int                              off{}, maxOff{};
  decoder::Header                  header;
  std::unique_ptr<decoder::IAudio> audio;
  float                            currentVolume = 0.0f;

  decoder::LayerI_II_III* layer{};

 public:
  Playback(std::unique_ptr<decoder::IAudio> audio)
      : audio(std::move(audio)), header(), buf(BUFLEN) {
    // id3tag = new ID3Tag();
  }

  ~Playback() { delete layer; }

  bool Open(std::string name, std::string title) {
    maxOff = 8192;
    off    = 0;
    paused = eof = false;

    instream  = std::ifstream(name, std::ifstream::binary);
    auto size = std::filesystem::file_size(name);
    header.Initialize(size, 0);
    NextHeader();

    audio->Open(header, nullptr);

    if (eof) return false;
    return true;
  }

  bool Start(bool verbose) {
    using namespace decoder;
    layer      = nullptr;
    int frames = 0;
    paused     = false;

    audio.get()->Start(true);

    switch (header.GetLayer()) {
      case 1:
        layer = new LayerI(std::move(header), std::move(audio));
        break;
      case 2:
        layer = new LayerII(std::move(header), std::move(audio));
        break;
      case 3:
        layer = new LayerIII(std::move(header), std::move(audio));
        break;
      default:
        return false;
    }

    while (!eof) {
      // 1. 解码一帧并输出(播放)
      off = layer->DecodeFrame(buf, off);
      //      auto a = layer->DecodeFrame(buf, off);
      if (verbose && (++frames & 0x7) == 0) header.PrintProgress();

      // 2. 定位到下一帧并解码帧头
      NextHeader();

      // 3. 检测并处理暂停
      if (paused) {
        while (paused && !eof)
          ;
      }
    }
    if (verbose) {
      header.PrintProgress();
      std::cout << std::endl;
    }
    return true;
  }

  //====================================================================

  void NextHeader() {
    int len = 0, chunk = 0;
    while (!eof && !header.SyncFrame(buf, off, maxOff)) {
      // buf内帧同步失败或数据不足一帧，刷新缓冲区buf
      off = header.Offset();
      len = maxOff - off;
      //      System.arraycopy(buf, off, buf, 0, len);
      memmove(buf.data(), buf.data() + off, len);
      instream.read(reinterpret_cast<char*>(buf.data() + len), off);
      maxOff = len + instream.gcount();
      off    = 0;
      if (maxOff <= len || (chunk += BUFLEN) > 0x10000) eof = true;
    }
    off = header.Offset();
  }
};
}  // namespace jmp123

#endif  // JMP123_PLAYBACK_H
