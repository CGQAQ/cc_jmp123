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

/*
 * dewin: D[i] * 32767 (i=0..511), 然后重新排序
 * D[]: Coefficients Di of the synthesis window. ISO/IEC 11172-3 ANNEX_B Table
 * 3-B.3
 */
inline constexpr std::array<std::array<float, 16>, 32> kDewin =
    std::to_array<std::array<float, 16>, 32>(
        {// [32][16]
         {0.0f, -14.5f, 106.5f, -229.5f, 1018.5f, -2576.5f, 3287.0f, -18744.5f,
          37519.0f, 18744.5f, 3287.0f, 2576.5f, 1018.5f, 229.5f, 106.5f, 14.5f},
         {-0.5f, -15.5f, 109.0f, -259.5f, 1000.0f, -2758.5f, 2979.5f, -19668.0f,
          37496.0f, 17820.0f, 3567.0f, 2394.0f, 1031.5f, 200.5f, 104.0f, 13.0f},
         {-0.5f, -17.5f, 111.0f, -290.5f, 976.0f, -2939.5f, 2644.0f, -20588.0f,
          37428.0f, 16895.5f, 3820.0f, 2212.5f, 1040.0f, 173.5f, 101.0f, 12.0f},
         {-0.5f, -19.0f, 112.5f, -322.5f, 946.5f, -3118.5f, 2280.5f, -21503.0f,
          37315.0f, 15973.5f, 4046.0f, 2031.5f, 1043.5f, 147.0f, 98.0f, 10.5f},
         {-0.5f, -20.5f, 113.5f, -355.5f, 911.0f, -3294.5f, 1888.0f, -22410.5f,
          37156.5f, 15056.0f, 4246.0f, 1852.5f, 1042.5f, 122.0f, 95.0f, 9.5f},
         {-0.5f, -22.5f, 114.0f, -389.5f, 869.5f, -3467.5f, 1467.5f, -23308.5f,
          36954.0f, 14144.5f, 4420.0f, 1675.5f, 1037.5f, 98.5f, 91.5f, 8.5f},
         {-0.5f, -24.5f, 114.0f, -424.0f, 822.0f, -3635.5f, 1018.5f, -24195.0f,
          36707.5f, 13241.0f, 4569.5f, 1502.0f, 1028.5f, 76.5f, 88.0f, 8.0f},
         {-1.0f, -26.5f, 113.5f, -459.5f, 767.5f, -3798.5f, 541.0f, -25068.5f,
          36417.5f, 12347.0f, 4694.5f, 1331.5f, 1016.0f, 55.5f, 84.5f, 7.0f},
         {-1.0f, -29.0f, 112.0f, -495.5f, 707.0f, -3955.0f, 35.0f, -25926.5f,
          36084.5f, 11464.5f, 4796.0f, 1165.0f, 1000.5f, 36.0f, 80.5f, 6.5f},
         {-1.0f, -31.5f, 110.5f, -532.0f, 640.0f, -4104.5f, -499.0f, -26767.0f,
          35710.0f, 10594.5f, 4875.0f, 1003.0f, 981.0f, 18.0f, 77.0f, 5.5f},
         {-1.0f, -34.0f, 107.5f, -568.5f, 565.5f, -4245.5f, -1061.0f, -27589.0f,
          35295.0f, 9739.0f, 4931.5f, 846.0f, 959.5f, 1.0f, 73.5f, 5.0f},
         {-1.5f, -36.5f, 104.0f, -605.0f, 485.0f, -4377.5f, -1650.0f, -28389.0f,
          34839.5f, 8899.5f, 4967.5f, 694.0f, 935.0f, -14.5f, 69.5f, 4.5f},
         {-1.5f, -39.5f, 100.0f, -641.5f, 397.0f, -4499.0f, -2266.5f, -29166.5f,
          34346.f, 8077.5f, 4983.f, 547.5f, 908.5f, -28.5f, 66.f, 4.f},
         {-2.f, -42.5f, 94.5f, -678.f, 302.5f, -4609.5f, -2909.f, -29919.f,
          33814.5f, 7274.f, 4979.5f, 407.f, 879.5f, -41.5f, 62.5f, 3.5f},
         {-2.f, -45.5f, 88.5f, -714.f, 201.f, -4708.f, -3577.f, -30644.5f,
          33247.f, 6490.f, 4958.f, 272.5f, 849.f, -53.f, 58.5f, 3.5f},
         {-2.5f, -48.5f, 81.5f, -749.f, 92.5f, -4792.5f, -4270.f, -31342.f,
          32645.f, 5727.5f, 4919.f, 144.f, 817.f, -63.5f, 55.5f, 3.f},
         {-2.5f, -52.f, 73.f, -783.5f, -22.5f, -4863.5f, -4987.5f, -32009.5f,
          32009.5f, 4987.5f, 4863.5f, 22.5f, 783.5f, -73.f, 52.f, 2.5f},
         {-3.f, -55.5f, 63.5f, -817.f, -144.f, -4919.f, -5727.5f, -32645.f,
          31342.f, 4270.f, 4792.5f, -92.5f, 749.f, -81.5f, 48.5f, 2.5f},
         {-3.5f, -58.5f, 53.f, -849.f, -272.5f, -4958.f, -6490.f, -33247.f,
          30644.5f, 3577.f, 4708.f, -201.f, 714.f, -88.5f, 45.5f, 2.f},
         {-3.5f, -62.5f, 41.5f, -879.5f, -407.f, -4979.5f, -7274.f, -33814.5f,
          29919.f, 2909.f, 4609.5f, -302.5f, 678.f, -94.5f, 42.5f, 2.f},
         {-4.f, -66.f, 28.5f, -908.5f, -547.5f, -4983.f, -8077.5f, -34346.f,
          29166.5f, 2266.5f, 4499.f, -397.f, 641.5f, -100.f, 39.5f, 1.5f},
         {-4.5f, -69.5f, 14.5f, -935.f, -694.f, -4967.5f, -8899.5f, -34839.5f,
          28389.f, 1650.f, 4377.5f, -485.f, 605.f, -104.f, 36.5f, 1.5f},
         {-5.f, -73.5f, -1.f, -959.5f, -846.f, -4931.5f, -9739.0f, -35295.0f,
          27589.0f, 1061.0f, 4245.5f, -565.5f, 568.5f, -107.5f, 34.f, 1.0f},
         {-5.5f, -77.f, -18.f, -981.f, -1003.f, -4875.f, -10594.5f, -35710.0f,
          26767.0f, 499.0f, 4104.5f, -640.f, 532.f, -110.5f, 31.5f, 1.f},
         {-6.5f, -80.5f, -36.f, -1000.5f, -1165.0f, -4796.0f, -11464.5f,
          -36084.5f, 25926.5f, -35.0f, 3955.0f, -707.0f, 495.5f, -112.0f, 29.0f,
          1.0f},
         {-7.0f, -84.5f, -55.5f, -1016.0f, -1331.5f, -4694.5f, -12347.0f,
          -36417.5f, 25068.5f, -541.0f, 3798.5f, -767.5f, 459.5f, -113.5f,
          26.5f, 1.0f},
         {-8.0f, -88.0f, -76.5f, -1028.5f, -1502.0f, -4569.5f, -13241.0f,
          -36707.5f, 24195.0f, -1018.5f, 3635.5f, -822.0f, 424.0f, -114.0f,
          24.5f, 0.5f},
         {-8.5f, -91.5f, -98.5f, -1037.5f, -1675.5f, -4420.0f, -14144.5f,
          -36954.0f, 23308.5f, -1467.5f, 3467.5f, -869.5f, 389.5f, -114.0f,
          22.5f, 0.5f},
         {-9.5f, -95.0f, -122.0f, -1042.5f, -1852.5f, -4246.0f, -15056.0f,
          -37156.5f, 22410.5f, -1888.0f, 3294.5f, -911.0f, 355.5f, -113.5f,
          20.5f, 0.5f},
         {-10.5f, -98.0f, -147.0f, -1043.5f, -2031.5f, -4046.0f, -15973.5f,
          -37315.0f, 21503.0f, -2280.5f, 3118.5f, -946.5f, 322.5f, -112.5f,
          19.0f, 0.5f},
         {-12.0f, -101.0f, -173.5f, -1040.0f, -2212.5f, -3820.0f, -16895.5f,
          -37428.0f, 20588.0f, -2644.0f, 2939.5f, -976.0f, 290.5f, -111.0f,
          17.5f, 0.5f},
         {-13.0f, -104.0f, -200.5f, -1031.5f, -2394.0f, -3567.0f, -17820.0f,
          -37496.0f, 19668.0f, -2979.5f, 2758.5f, -1000.0f, 259.5f, -109.0f,
          15.5f, 0.5f}});

extern std::vector<std::vector<uint8_t>> const kNBal;

extern std::vector<std::vector<uint8_t>> const kSBQuant_Offset;

extern std::vector<std::vector<uint8_t>> const kOffsetTable;
}  // namespace jmp123::decoder
#endif  // JMP123_TABLES_H
