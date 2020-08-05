//
// Created by mjaso on 8/5/2020.
//

#ifndef JMP123_BIT_STREAM_H
#define JMP123_BIT_STREAM_H

#include <cstdint>
#include <memory>

namespace jmp123::decoder {
class BitStream {
 protected:
  int bit_pos_;
  int byte_pos_;
  std::unique_ptr<uint8_t[]> bit_reservoir_;

 private:
  int end_pos_;  // bit_reservoir_ 已填入字节数
  int max_off_;

 public:
  BitStream(int len, int extr);

  int Append(uint8_t b[], int off, int len);

  void Feed(std::unique_ptr<uint8_t[]> b, int off);

  int Get_1_Bit();

  int GetBits_17(int n);

  int GetBits_9(int n);

  [[nodiscard]] int GetBytePos() const { return byte_pos_; }

  [[nodiscard]] int GetSize() const { return end_pos_; }

  void SkipBytes(int n);

  void SkipBits(int n);
};
}  // namespace jmp123

#endif  // JMP123_BIT_STREAM_H
