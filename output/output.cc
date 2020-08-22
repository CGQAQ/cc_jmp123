//
// Created by jason on 2020/8/9.
//

#include "output.h"

#include <condition_variable>
#include <mutex>

namespace jmp123::output {

bool Output::Open(jmp123::decoder::Header h, std::unique_ptr<std::string> ptr) {
  RtAudio::StreamParameters parameters;
  parameters.deviceId     = dac_.getDefaultOutputDevice();
  parameters.nChannels    = h.GetChannelCount();
  parameters.firstChannel = 0;
  unsigned int sampleRate = h.GetSamplingRate();
  unsigned int bufferFrames =
      h.GetSamplingRate() / h.GetFrameDuration();  // 256 sample frames
  double                 data[2];
  RtAudio::StreamOptions options;
  options.flags = RTAUDIO_NONINTERLEAVED;

  try {
    dac_.openStream(&parameters, nullptr, RTAUDIO_FLOAT32, sampleRate,
                    &bufferFrames, &Output::Callback, &buffer_, &options,
                    nullptr);
  } catch (RtAudioError &e) {
    e.printMessage();
    return false;
  }

  return true;
}

static std::mutex              buffer_mutex;
static std::condition_variable buffer_condition;
static std::atomic_bool        start_;

int Output::Write(std::vector<uint8_t> const& b) {
  /*std::unique_lock lock(buffer_mutex);
  buffer_condition.wait(lock);*/

  buffer_ = std::vector<float>(b.size());
  std::copy(b.begin(), b.end(), buffer_.begin());
  return 0;
}
void Output::Start(bool b) { start_ = b; }
void Output::Drain() {}
void Output::close() {
  dac_.closeStream();
}
void Output::refreshMessage(std::string msg) {}
int  Output::Callback(void *outputBuffer, void *inputBuffer,
                     unsigned int nBufferFrames, double streamTime,
                     RtAudioStreamStatus status, void *userData) {
  if (!start_) return 0;

  buffer_condition.notify_one();
}
}  // namespace jmp123::output