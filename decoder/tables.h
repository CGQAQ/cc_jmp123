//
//    JMP123 Porting into c++ 20
//    Copyright (C) 2020  Jason <m.jason.liu@outlook.com>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License along
//    with this program; if not, write to the Free Software Foundation, Inc.,
//    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//

#ifndef JMP123_TABLES_H
#define JMP123_TABLES_H

#include <array>
#include <vector>
namespace jmp123::decoder {

// Layer1也用到factor[]
// ISO/IEC 11172-3 Table 3-B.1
// scalefactor值为'0000 00'..'1111 11'(0..63),应该有64个值.在末尾补一个数0.0f
inline constexpr std::array kFactor{
    2.00000000000000f, 1.58740105196820f, 1.25992104989487f, 1.00000000000000f,
    0.79370052598410f, 0.62996052494744f, 0.50000000000000f, 0.39685026299205f,
    0.31498026247372f, 0.25000000000000f, 0.19842513149602f, 0.15749013123686f,
    0.12500000000000f, 0.09921256574801f, 0.07874506561843f, 0.06250000000000f,
    0.04960628287401f, 0.03937253280921f, 0.03125000000000f, 0.02480314143700f,
    0.01968626640461f, 0.01562500000000f, 0.01240157071850f, 0.00984313320230f,
    0.00781250000000f, 0.00620078535925f, 0.00492156660115f, 0.00390625000000f,
    0.00310039267963f, 0.00246078330058f, 0.00195312500000f, 0.00155019633981f,
    0.00123039165029f, 0.00097656250000f, 0.00077509816991f, 0.00061519582514f,
    0.00048828125000f, 0.00038754908495f, 0.00030759791257f, 0.00024414062500f,
    0.00019377454248f, 0.00015379895629f, 0.00012207031250f, 0.00009688727124f,
    0.00007689947814f, 0.00006103515625f, 0.00004844363562f, 0.00003844973907f,
    0.00003051757813f, 0.00002422181781f, 0.00001922486954f, 0.00001525878906f,
    0.00001211090890f, 0.00000961243477f, 0.00000762939453f, 0.00000605545445f,
    0.00000480621738f, 0.00000381469727f, 0.00000302772723f, 0.00000240310869f,
    0.00000190734863f, 0.00000151386361f, 0.00000120155435f, 0.0f,
};

inline constexpr std::array<std::array<std::array<uint8_t, 16>, 2>, 3>
    kAIDXTable{std::to_array<std::array<uint8_t, 16>, 2>(
                   {{0, 2, 2, 2, 2, 2, 2, 0, 0, 0, 1, 1, 1, 1, 1, 0},
                    {0, 2, 2, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0}}),
               std::to_array<std::array<uint8_t, 16>, 2>(
                   {{0, 2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                    {0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}),
               std::to_array<std::array<uint8_t, 16>, 2>(
                   {{0, 3, 3, 3, 3, 3, 3, 0, 0, 0, 1, 1, 1, 1, 1, 0},
                    {0, 3, 3, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0}})};

// cq_xxx: Layer II classes of quantization, ISO/IEC 11172-3 Table 3-B.4
inline constexpr std::array kCQ_Steps{3,    5,    7,     9,     15,   31,
                                      63,   127,  255,   511,   1023, 2047,
                                      4095, 8191, 16383, 32767, 65535};

inline constexpr std::array kCQ_C{
    1.3333333f, 1.6f,      1.1428571f, 1.77777778f, 1.0666667f,    1.0322581f,
    1.015873f,  1.007874f, 1.0039216f, 1.0019569f,  1.0009775f,    1.0004885f,
    1.0002442f, 1.000122f, 1.000061f,  1.0000305f,  1.00001525902f};

inline constexpr std::array kCQ_D{
    0.5f,           0.5f,           0.25f,          0.5f,
    0.125f,         0.0625f,        0.03125f,       0.015625f,
    0.0078125f,     0.00390625f,    0.001953125f,   0.0009765625f,
    0.00048828125f, 0.00024414063f, 0.00012207031f, 0.00006103516f,
    0.00003051758f};

inline constexpr std::array kCQ_Bits{5, 7,  3,  10, 4,  5,  6,  7, 8,
                                     9, 10, 11, 12, 13, 14, 15, 16};

inline constexpr std::array kBitAlloc_Offset{0, 3, 3, 1, 2, 3, 4, 5};

inline constexpr std::array kGroup{2, 3, 0, 4, 0, 0, 0, 0, 0,
                                   0, 0, 0, 0, 0, 0, 0, 0};

extern std::vector<std::vector<uint8_t>> const kNBal;

extern std::vector<std::vector<uint8_t>> const kSBQuant_Offset;

extern std::vector<std::vector<uint8_t>> const kOffsetTable;
}  // namespace jmp123::decoder
#endif  // JMP123_TABLES_H
