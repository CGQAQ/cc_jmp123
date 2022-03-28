//
// Created by jason on 2020/8/9.
//

#ifndef JMP123_OUTPUT_H
#define JMP123_OUTPUT_H

#include <vector>

#include "decoder/audio_interface.h"
#include "portaudio.h"

namespace jmp123::output {

class Output : public jmp123::decoder::IAudio {
  PaStream* pa_stream_{};

  std::vector<float> buffer_;

 public:
  Output() {
    Pa_Initialize();
  }
  ~Output() override {
    if (pa_stream_ != nullptr){
      Pa_CloseStream(pa_stream_);
      pa_stream_ = nullptr;
    }
    Pa_Terminate();
  }

  bool Open(decoder::Header const &h, std::unique_ptr<std::string> ptr) override;
  std::vector<float> const& GetBuffer();
  int  Write(std::vector<uint8_t> const& b) override;
  void Start(bool b) override;
  void Drain() override;
  void close() override;
  void refreshMessage(std::string msg) override;
};

//static int AudioCallback(void *outputBuffer, void *inputBuffer,
//                    unsigned int nBufferFrames, double streamTime,
//                    RtAudioStreamStatus status, void *userData);
}  // namespace jmp123::output

#endif  // JMP123_OUTPUT_H
