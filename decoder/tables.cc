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
}