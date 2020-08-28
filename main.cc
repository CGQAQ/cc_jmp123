#include "playback.h"
#include "output/output.h"
#include <memory>
#include <fmt/format.h>

#include "config.h"

using namespace jmp123;
int main() {
  Playback p2(std::make_unique<output::Output>());
  p2.Open(fmt::format("{}/{}", PROJECT_PATH, "test.mp3"), "title");
  p2.Start(true);
}

//int main()
//{
//  Pa_Initialize();
//
//  PaStreamParameters parameters;
//  parameters.channelCount = 2;
//  parameters.hostApiSpecificStreamInfo = nullptr;
//  parameters.device = Pa_GetDefaultOutputDevice();
//  parameters.sampleFormat = paInt16;
//  parameters.suggestedLatency = Pa_GetDeviceInfo(parameters.device)->defaultHighOutputLatency;
//
//  PaStream* p;
//  Pa_OpenStream(&p, nullptr, &parameters, 48000,
//                parameters.suggestedLatency * 48000, paNoFlag, nullptr,
//                nullptr);
//
//  Pa_StartStream(p);
//
//  using namespace std;
//  ifstream f{ R"(C:\Users\mjaso\Desktop\j.out)", ios_base::binary };
//  char* buf = (char*)malloc(18432);
//  while (!f.eof())
//  {
//    f.read(buf, 18432);
//    Pa_WriteStream(p, buf, 18432/4);
//  }
//
//  std::cout << "Hello World!\n";
//}
