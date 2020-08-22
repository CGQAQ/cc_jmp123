//
// Created by jason on 2020/8/9.
//

#ifndef JMP123_OUTPUT_H
#define JMP123_OUTPUT_H

#include <vector>

#include "RtAudio.h"
#include "decoder/audio_interface.h"

namespace jmp123::output {
class Output : public jmp123::decoder::IAudio {
  RtAudio dac_;

  std::vector<float> buffer_;

 public:
  Output() : dac_() {}

  bool Open(decoder::Header const &h, std::unique_ptr<std::string> ptr) override;
  int  Write(std::vector<uint8_t> const& b) override;
  void Start(bool b) override;
  void Drain() override;
  void close() override;
  void refreshMessage(std::string msg) override;

  static int Callback(void *outputBuffer, void *inputBuffer,
                      unsigned int nBufferFrames, double streamTime,
                      RtAudioStreamStatus status, void *userData);
};
}  // namespace jmp123::output

#endif  // JMP123_OUTPUT_H
