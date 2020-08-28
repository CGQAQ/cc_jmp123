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

#include "tables.h"

namespace jmp123::decoder {

std::vector<std::vector<uint8_t>> const kNBal{
    // ISO/IEC 11172-3 Table 3-B.2a
    {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3,
     3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2},
    // ISO/IEC 11172-3 Table 3-B.2b
    {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3,
     3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2},
    // ISO/IEC 11172-3 Table 3-B.2c
    {4, 4, 3, 3, 3, 3, 3, 3},
    // ISO/IEC 11172-3 Table 3-B.2c
    {4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    // ISO/IEC 13818-3 Table B.1
    {4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2,
     2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
};

std::vector<std::vector<uint8_t>> const kSBQuant_Offset{
    // ISO/IEC 11172-3 Table 3-B.2a
    {7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 3, 3, 3,
     3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0},

    // ISO/IEC 11172-3 Table 3-B.2b
    {7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 3, 3, 3, 3,
     3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0},

    // ISO/IEC 11172-3 Table 3-B.2c
    {5, 5, 2, 2, 2, 2, 2, 2},

    // ISO/IEC 11172-3 Table 3-B.2d
    {5, 5, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},

    // ISO/IEC 13818-3 Table B.1
    {4, 4, 4, 4, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

std::vector<std::vector<uint8_t>> const kOffsetTable{
    {0, 1, 16},
    {0, 1, 2, 3, 4, 5, 16},
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14},
    {0, 1, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 16},
    {0, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}};

std::vector<std::vector<float>> kIMDCTWin = {
    {0.0322824f,  0.1072064f,  0.2014143f,  0.3256164f,  0.5f,
     0.7677747f,  1.2412229f,  2.3319514f,  7.7441506f,  -8.4512568f,
     -3.0390580f, -1.9483297f, -1.4748814f, -1.2071068f, -1.0327232f,
     -0.9085211f, -0.8143131f, -0.7393892f, -0.6775254f, -0.6248445f,
     -0.5787917f, -0.5376016f, -0.5f,       -0.4650284f, -0.4319343f,
     -0.4000996f, -0.3689899f, -0.3381170f, -0.3070072f, -0.2751725f,
     -0.2420785f, -0.2071068f, -0.1695052f, -0.1283151f, -0.0822624f,
     -0.0295815f},
    {0.0322824f,  0.1072064f,  0.2014143f,  0.3256164f,  0.5f,
     0.7677747f,  1.2412229f,  2.3319514f,  7.7441506f,  -8.4512568f,
     -3.0390580f, -1.9483297f, -1.4748814f, -1.2071068f, -1.0327232f,
     -0.9085211f, -0.8143131f, -0.7393892f, -0.6781709f, -0.6302362f,
     -0.5928445f, -0.5636910f, -0.5411961f, -0.5242646f, -0.5077583f,
     -0.4659258f, -0.3970546f, -0.3046707f, -0.1929928f, -0.0668476f,
     -0.0f,       -0.0f,       -0.0f,       -0.0f,       -0.0f,
     -0.0f},
    {/* block_type = 2 */},
    {0.0f,        0.0f,        0.0f,        0.0f,        0.0f,
     0.0f,        0.3015303f,  1.4659259f,  6.9781060f,  -9.0940447f,
     -3.5390582f, -2.2903500f, -1.6627548f, -1.3065630f, -1.0828403f,
     -0.9305795f, -0.8213398f, -0.7400936f, -0.6775254f, -0.6248445f,
     -0.5787917f, -0.5376016f, -0.5f,       -0.4650284f, -0.4319343f,
     -0.4000996f, -0.3689899f, -0.3381170f, -0.3070072f, -0.2751725f,
     -0.2420785f, -0.2071068f, -0.1695052f, -0.1283151f, -0.0822624f,
     -0.0295815f}};
}  // namespace jmp123::decoder