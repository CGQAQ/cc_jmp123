//
// Created by mjaso on 8/5/2020.
//

#include "bit_stream.h"

jmp123::decoder::BitStream::BitStream(int len, int extr)
    : bit_pos_(0), byte_pos_(0), end_pos_(0), max_off_(len) {
  bit_reservoir_ = std::make_unique<uint8_t[]>(len + extr);
}

int jmp123::decoder::BitStream::Append(uint8_t *b, int off, int len) {
  if (len + end_pos_ > max_off_) {
    // std::copy(bit_reservoir_ + byte_pos_, bit_reservoir_)
    std::memcpy(bit_reservoir_.get(), &bit_reservoir_[byte_pos_],
                end_pos_ - byte_pos_);
    end_pos_ -= byte_pos_;
    bit_pos_ = byte_pos_ = 0;
  }
  if (len + end_pos_ > max_off_) len = max_off_ - end_pos_;
  std::memcpy(&bit_reservoir_[end_pos_], &b[off], len);
  end_pos_ += len;
  return len;
}

void jmp123::decoder::BitStream::Feed(std::unique_ptr<uint8_t[]> b, int off) {
  bit_reservoir_ = std::move(b);
  byte_pos_ = off;
  bit_pos_ = 0;
}

int jmp123::decoder::BitStream::GetBits_17(int n) {
  int iret = bit_reservoir_[byte_pos_];
  iret <<= 8;
  iret |= bit_reservoir_[byte_pos_ + 1] & 0xff;
  iret <<= 8;
  iret |= bit_reservoir_[byte_pos_ + 2] & 0xff;
  iret <<= bit_pos_;
  iret &= 0xffffff;
  iret >>= 24 - n;
  bit_pos_ += n;
  byte_pos_ += bit_pos_ >> 3;
  bit_pos_ &= 0x7;
  return iret;
}

int jmp123::decoder::BitStream::GetBits_9(int n) {
  int iret = bit_reservoir_[byte_pos_];
  iret <<= 8;
  iret |= bit_reservoir_[byte_pos_ + 1] & 0xff;
  iret <<= bit_pos_;
  iret &= 0xffff;
  iret >>= 16 - n;
  bit_pos_ += n;
  byte_pos_ += bit_pos_ >> 3;
  bit_pos_ &= 0x7;
  return iret;
}

void jmp123::decoder::BitStream::SkipBytes(int n) {
  byte_pos_ += n;
  bit_pos_ = 0;
}

int jmp123::decoder::BitStream::Get_1_Bit() {
  int bit = bit_reservoir_[byte_pos_] << bit_pos_;
  bit >>= 7;
  bit &= 1;
  bit_pos_++;
  byte_pos_ += bit_pos_ >> 3;
  bit_pos_ &= 0x7;
  return bit;
}

void jmp123::decoder::BitStream::SkipBits(int n) {
  bit_pos_ += n;
  byte_pos_ += bit_pos_ >> 3;
  bit_pos_ &= 0x7;
}
