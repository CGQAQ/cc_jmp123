#include "playback.h"
#include "output/output.h"
#include <memory>

#define __LINUX_ALSA__

using namespace jmp123;
int main() {
  Playback p2(std::make_unique<output::Output>());
  //p2.Open("/home/jason/code/cc/cc_jmp123/test.mp3", "title");
  p2.Open("C:\\Users\\mjaso\\source\\repos\\cc_jmp123\\test.mp3", "title");
//  p2.Open("/home/jason/Desktop/test.mp3", "title");
  p2.Start(false);
}
