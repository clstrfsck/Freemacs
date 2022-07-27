/*
 * Copyright 2020 Martin Sandiford
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to: Free Software Foundation
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <iostream>

#include "gapbuffer.h"

#include "gtest/gtest.h"

namespace {
}

TEST(GapBuffer, gap_buffer_basic_construction) {
    GapBuffer gb;
    EXPECT_EQ(65536UL, gb.allocated());
    EXPECT_EQ(65536UL, gb.free());
    EXPECT_EQ(0UL, gb.size());
}

TEST(GapBuffer, gap_buffer_basic_insert) {
    GapBuffer gb;
    EXPECT_TRUE(gb.insert(0, "0123456789"));
    EXPECT_EQ(65536UL, gb.allocated());
    EXPECT_EQ(65526UL, gb.free());
    EXPECT_EQ(10UL, gb.size());
    std::string contents(gb.begin(), gb.end());
    EXPECT_EQ("0123456789", contents);
}

TEST(GapBuffer, gap_buffer_basic_erase) {
    GapBuffer gb;
    EXPECT_TRUE(gb.insert(0, "0123456789"));
    EXPECT_TRUE(gb.erase(0, 1));
    EXPECT_EQ(65536UL, gb.allocated());
    EXPECT_EQ(65527UL, gb.free());
    EXPECT_EQ(9UL, gb.size());
}

TEST(GapBuffer, gap_buffer_insert_at_end) {
    GapBuffer gb;
    EXPECT_TRUE(gb.insert(0, "0123456789"));
    EXPECT_TRUE(gb.insert(10, "ABCDEFGHIJ"));
    EXPECT_EQ(65536UL, gb.allocated());
    EXPECT_EQ(65516UL, gb.free());
    EXPECT_EQ(20UL, gb.size());
    std::string contents(gb.begin(), gb.end());
    EXPECT_EQ("0123456789ABCDEFGHIJ", contents);
}

TEST(GapBuffer, gap_buffer_insert_at_begin) {
    GapBuffer gb;
    EXPECT_TRUE(gb.insert(0, "0123456789"));
    EXPECT_TRUE(gb.insert(0, "ABCDEFGHIJ"));
    EXPECT_EQ(65536UL, gb.allocated());
    EXPECT_EQ(65516UL, gb.free());
    EXPECT_EQ(20UL, gb.size());
    std::string contents(gb.begin(), gb.end());
    EXPECT_EQ("ABCDEFGHIJ0123456789", contents);
}

TEST(GapBuffer, gap_buffer_insert_in_middle) {
    GapBuffer gb;
    EXPECT_TRUE(gb.insert(0, "0123456789"));
    EXPECT_TRUE(gb.insert(5, "ABCDEFGHIJ"));
    EXPECT_EQ(65536UL, gb.allocated());
    EXPECT_EQ(65516UL, gb.free());
    EXPECT_EQ(20UL, gb.size());
    std::string contents(gb.begin(), gb.end());
    EXPECT_EQ("01234ABCDEFGHIJ56789", contents);
}

TEST(GapBuffer, gap_buffer_insert_move_forward) {
    GapBuffer gb;
    EXPECT_TRUE(gb.insert(0, "56789"));
    EXPECT_TRUE(gb.insert(0, "01234"));
    EXPECT_TRUE(gb.insert(10, "ABCDEFGHIJ"));
    EXPECT_EQ(65536UL, gb.allocated());
    EXPECT_EQ(65516UL, gb.free());
    EXPECT_EQ(20UL, gb.size());
    std::string contents(gb.begin(), gb.end());
    EXPECT_EQ("0123456789ABCDEFGHIJ", contents);
}
