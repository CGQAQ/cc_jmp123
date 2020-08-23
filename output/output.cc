//
// Created by jason on 2020/8/9.
//

#include "output.h"

#include <atomic>
#include <condition_variable>
#include <mutex>

namespace jmp123::output {

bool Output::Open(jmp123::decoder::Header const &h,
                  std::unique_ptr<std::string>   ptr) {
  RtAudio::StreamParameters parameters;
  parameters.deviceId                 = dac_.getDefaultOutputDevice();
  parameters.nChannels                = h.GetChannelCount();
  parameters.firstChannel             = 0;
  unsigned int           sampleRate   = h.GetSamplingRate();
  unsigned int           bufferFrames = 1152;  // 256 sample frames
  double                 data[2];
  RtAudio::StreamOptions options;
  options.flags = RTAUDIO_NONINTERLEAVED;

  try {
    dac_.openStream(&parameters, nullptr, RTAUDIO_FLOAT32, sampleRate,
                    &bufferFrames, &AudioCallback, this, &options,
                    nullptr);
  } catch (RtAudioError &e) {
    e.printMessage();
    return false;
  }

  auto a = dac_.getCurrentApi();
  auto b = dac_.getDeviceCount();

  return true;
}

std::vector<float> const &Output::GetBuffer() { return buffer_; }


static std::mutex              buffer_mutex;
static std::condition_variable buffer_condition;
static std::atomic_bool        start_;

int Output::Write(std::vector<uint8_t> const &b) {
  std::unique_lock lock(buffer_mutex);
  buffer_condition.wait(lock);

  buffer_ = std::vector<float>(b.size());
  std::copy(b.begin(), b.end(), buffer_.begin());
  return 0;
}
void Output::Start(bool b) {
  start_ = b;
  if (b) {
    dac_.startStream();
  } else {
    dac_.stopStream();
  }
}
void Output::Drain() {}
void Output::close() { dac_.closeStream(); }
void Output::refreshMessage(std::string msg) {}

static int AudioCallback(void *outputBuffer, void *inputBuffer,
                     unsigned int nBufferFrames, double streamTime,
                     RtAudioStreamStatus status, void *userData) {
  {
    std::lock_guard lock{buffer_mutex};
    if (!start_) return 0;

    auto& output = *reinterpret_cast<Output *>(userData);

    memcpy(outputBuffer, output.GetBuffer().data(), output.GetBuffer().size());
  }

  buffer_condition.notify_one();
}
}  // namespace jmp123::output