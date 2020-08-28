#include "playback.h"
#include "output/output.h"
#include <memory>

using namespace jmp123;
int main() {
  Playback p2(std::make_unique<output::Output>());
  //p2.Open("/home/jason/code/cc/cc_jmp123/test.mp3", "title");

//  p2.Open("C:\\Users\\mjaso\\source\\repos\\cc_jmp123\\test.mp3", "title");
  p2.Open(R"(C:\Users\mjaso\Downloads\Jason Derulo - Take You Dancing.mp3)",
           "title");
//  p2.Open("/home/jason/Desktop/test.mp3", "t");
  p2.Start(false);
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
