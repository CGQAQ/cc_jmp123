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

  PaStreamParameters parameters;
  parameters.channelCount = h.GetChannelCount();
  parameters.hostApiSpecificStreamInfo = nullptr;
  parameters.device = Pa_GetDefaultOutputDevice();
  parameters.sampleFormat = paInt16;
  parameters.suggestedLatency = Pa_GetDeviceInfo(parameters.device)->defaultHighOutputLatency;

  Pa_OpenStream(&pa_stream_, nullptr, &parameters, h.GetSamplingRate(),
                /*paFramesPerBufferUnspecified*//*h.GetSamplingRate() * parameters.suggestedLatency*/ 18432 / 2, paClipOff,
                nullptr, nullptr);

  return true;
}

std::vector<float> const &Output::GetBuffer() { return buffer_; }


static std::mutex              buffer_mutex;
static std::condition_variable buffer_condition;
static std::atomic_bool        start_;

int Output::Write(std::vector<int8_t> const &b) {
  Pa_WriteStream(pa_stream_, b.data(), b.size() / 2);
  return 0;
}
void Output::Start(bool b) {
  start_ = b;
  if (b) {
    Pa_StartStream(pa_stream_);
  } else {
    Pa_StopStream(pa_stream_);
  }
}
void Output::Drain() {}
void Output::close() {  }
void Output::refreshMessage(std::string msg) {}

//static int AudioCallback(void *outputBuffer, void *inputBuffer,
//                     unsigned int nBufferFrames, double streamTime,
//                     RtAudioStreamStatus status, void *userData) {
////  {
////    std::lock_guard lock{buffer_mutex};
////    if (!start_) return 0;
////
////    auto& output = *reinterpret_cast<Output *>(userData);
////
////    memcpy(outputBuffer, output.GetBuffer().data(), output.GetBuffer().size());
////  }
////
////  buffer_condition.notify_one();
//}
}  // namespace jmp123::output