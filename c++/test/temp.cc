/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Adaptor.hh"
#include "Compression.hh"
#include "OrcTest.hh"
#include "RLE.hh"
#include "wrap/gtest-wrapper.h"

#include <iostream>
#include <vector>

namespace orc {

std::vector<int64_t> decodeRLEv2(const unsigned char *bytes,
                                 unsigned long l,
                                 size_t n,
                                 size_t count,
                                 const char* notNull = nullptr,
                                 bool isSigned = true,
                                 uint64_t blockSize = 0) {
  std::unique_ptr<RleDecoder> rle = createRleDecoder(
      std::unique_ptr<SeekableInputStream>(
            new SeekableArrayInputStream(bytes,l, blockSize)),
        isSigned,RleVersion_2, *getDefaultPool());
  std::vector<int64_t> results;
  for (size_t i = 0; i < count; i+=n) {
    size_t remaining = count - i;
    size_t nread = std::min(n, remaining);
    std::vector<int64_t> data(nread);
    rle->next(data.data(), nread, notNull);
    if (notNull) {
      notNull += nread;
    }
    results.insert(results.end(), data.begin(), data.end());
  }

  return results;
}

void checkResults(const std::vector<int64_t> &e, const std::vector<int64_t> &a,
                  size_t n, const char* notNull = nullptr) {
  EXPECT_EQ(e.size(), a.size()) << "vectors differ in size";
  for (size_t i = 0; i < e.size(); ++i) {
    if (!notNull || notNull[i]) {
      EXPECT_EQ(e[i], a[i]) << "Output wrong at " << i << ", n=" << n;
    }
  }
}

TEST(RLEv2, basicDelta0) {
  const size_t count = 20;
  std::vector<int64_t> values;
  for (size_t i = 0; i < count; ++i) {
    values.push_back(static_cast<int64_t>(i));
  }

  const unsigned char bytes[] = {0xc0,0x13,0x00,0x02};
  unsigned long l = sizeof(bytes) / sizeof(char);
  // Read 1 at a time, then 3 at a time, etc.
  checkResults(values, decodeRLEv2(bytes, l, 1, count), 1);
  checkResults(values, decodeRLEv2(bytes, l, 3, count), 3);
  checkResults(values, decodeRLEv2(bytes, l, 7, count), 7);
  checkResults(values, decodeRLEv2(bytes, l, count, count), count);
};

TEST(RLEv2, basicDelta1) {
  std::vector<int64_t> values(5);
  values[0] = -500;
  values[1] = -400;
  values[2] = -350;
  values[3] = -325;
  values[4] = -310;

  const unsigned char bytes[] = {0xce,0x04,0xe7,0x07,0xc8,0x01,0x32,0x19,0x0f};
  unsigned long l = sizeof(bytes) / sizeof(char);
  // Read 1 at a time, then 3 at a time, etc.
  checkResults(values, decodeRLEv2(bytes, l, 1, values.size()), 1);
  checkResults(values, decodeRLEv2(bytes, l, 3, values.size()), 3);
  checkResults(values, decodeRLEv2(bytes, l, 7, values.size()), 7);
  checkResults(values, decodeRLEv2(bytes, l, values.size(), values.size()),
               values.size());
};

TEST(RLEv2, basicDelta2) {
  std::vector<int64_t> values(5);
  values[0] = -500;
  values[1] = -600;
  values[2] = -650;
  values[3] = -675;
  values[4] = -710;

  const unsigned char bytes[] = {0xce,0x04,0xe7,0x07,0xc7,0x01,0x32,0x19,0x23};
  unsigned long l = sizeof(bytes) / sizeof(char);
  // Read 1 at a time, then 3 at a time, etc.
  checkResults(values, decodeRLEv2(bytes, l, 1, values.size()), 1);
  checkResults(values, decodeRLEv2(bytes, l, 3, values.size()), 3);
  checkResults(values, decodeRLEv2(bytes, l, 7, values.size()), 7);
  checkResults(values, decodeRLEv2(bytes, l, values.size(), values.size()),
               values.size());
};

TEST(RLEv2, basicDelta3) {
  std::vector<int64_t> values(5);
  values[0] = 500;
  values[1] = 400;
  values[2] = 350;
  values[3] = 325;
  values[4] = 310;

  const unsigned char bytes[] = {0xce,0x04,0xe8,0x07,0xc7,0x01,0x32,0x19,0x0f};
  unsigned long l = sizeof(bytes) / sizeof(char);
  // Read 1 at a time, then 3 at a time, etc.
  checkResults(values, decodeRLEv2(bytes, l, 1, values.size()), 1);
  checkResults(values, decodeRLEv2(bytes, l, 3, values.size()), 3);
  checkResults(values, decodeRLEv2(bytes, l, 7, values.size()), 7);
  checkResults(values, decodeRLEv2(bytes, l, values.size(), values.size()),
               values.size());
};

TEST(RLEv2, basicDelta4) {
  std::vector<int64_t> values(5);
  values[0] = 500;
  values[1] = 600;
  values[2] = 650;
  values[3] = 675;
  values[4] = 710;

  const unsigned char bytes[] = {0xce,0x04,0xe8,0x07,0xc8,0x01,0x32,0x19,0x23};
  unsigned long l = sizeof(bytes) / sizeof(char);
  // Read 1 at a time, then 3 at a time, etc.
  checkResults(values, decodeRLEv2(bytes, l, 1, values.size()), 1);
  checkResults(values, decodeRLEv2(bytes, l, 3, values.size()), 3);
  checkResults(values, decodeRLEv2(bytes, l, 7, values.size()), 7);
  checkResults(values, decodeRLEv2(bytes, l, values.size(), values.size()),
               values.size());
};

TEST(RLEv2, delta0Width) {
  const unsigned char buffer[] = {0x4e, 0x2, 0x0, 0x1, 0x2, 0xc0, 0x2, 0x42,
				  0x0};
  std::unique_ptr<RleDecoder> decoder =
    createRleDecoder(std::unique_ptr<SeekableInputStream>
                     (new SeekableArrayInputStream
                      (buffer, ARRAY_SIZE(buffer))),
                     false, RleVersion_2, *getDefaultPool());
  int64_t values[6];
  decoder->next(values, 6, 0);
  EXPECT_EQ(0, values[0]);
  EXPECT_EQ(1, values[1]);
  EXPECT_EQ(2, values[2]);
  EXPECT_EQ(0x42, values[3]);
  EXPECT_EQ(0x42, values[4]);
  EXPECT_EQ(0x42, values[5]);
}

TEST(RLEv2, basicDelta0WithNulls) {
  std::vector<int64_t> values;
  std::vector<char> notNull;
  for (size_t i = 0; i < 20; ++i) {
    values.push_back(static_cast<int64_t>(i));
    notNull.push_back(true);
    // throw in a null every third value
    bool addNull = (i % 3 == 0);
    if (addNull) {
      values.push_back(-1);
      notNull.push_back(false);
    }
  }

  const unsigned char bytes[] = {0xc0,0x13,0x00,0x02};
  unsigned long l = sizeof(bytes) / sizeof(char);
  const size_t count = values.size();
  // Read 1 at a time, then 3 at a time, etc.
  checkResults(values, decodeRLEv2(bytes, l, 1, count, notNull.data()),
               1, notNull.data());
  checkResults(values, decodeRLEv2(bytes, l, 3, count, notNull.data()),
               3, notNull.data());
  checkResults(values, decodeRLEv2(bytes, l, 7, count, notNull.data()),
               7, notNull.data());
  checkResults(values, decodeRLEv2(bytes, l, count, count, notNull.data()),
               count, notNull.data());
};

TEST(RLEv2, shortRepeats) {
  const size_t runLength = 7;
  const size_t nVals = 10;
  const size_t count = nVals * runLength;
  std::vector<int64_t> values;
  for (size_t i = 0; i < nVals; ++i) {
    for (size_t j = 0; j < runLength; ++j) {
      values.push_back(static_cast<int64_t>(i));
    }
  }

  const unsigned char bytes[] = {0x04,0x00,0x04,0x02,0x04,0x04,0x04,
                                 0x06,0x04,0x08,0x04,0x0a,0x04,0x0c,
                                 0x04,0x0e,0x04,0x10,0x04,0x12};
  unsigned long l = sizeof(bytes) / sizeof(char);
  // Read 1 at a time, then 3 at a time, etc.
  checkResults(values, decodeRLEv2(bytes, l, 1, count), 1);
  checkResults(values, decodeRLEv2(bytes, l, 3, count), 3);
  checkResults(values, decodeRLEv2(bytes, l, 7, count), 7);
  checkResults(values, decodeRLEv2(bytes, l, count, count), count);
};

TEST(RLEv2, multiByteShortRepeats) {
  const size_t runLength = 7;
  const size_t nVals = 3;
  const size_t count = nVals * runLength;
  std::vector<int64_t> values;
  for (size_t i = 0; i < nVals; ++i) {
    for (size_t j = 0; j < runLength; ++j) {
      values.push_back(static_cast<int64_t>(i) + (1LL << 62));
    }
  }

  const unsigned char bytes[] = {0x3c,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                 0x3c,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x02,
                                 0x3c,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x04};
  unsigned long l = sizeof(bytes) / sizeof(char);
  // Read 1 at a time, then 3 at a time, etc.
  checkResults(values, decodeRLEv2(bytes, l, 1, count), 1);
  checkResults(values, decodeRLEv2(bytes, l, 3, count), 3);
  checkResults(values, decodeRLEv2(bytes, l, 7, count), 7);
  checkResults(values, decodeRLEv2(bytes, l, count, count), count);
};

TEST(RLEv2, 0to2Repeat1Direct) {
  const unsigned char buffer[] = {0x46, 0x02, 0x02, 0x40};
  std::unique_ptr<RleDecoder> rle =
      createRleDecoder(std::unique_ptr<SeekableInputStream>
		       (new SeekableArrayInputStream(buffer,
						     ARRAY_SIZE(buffer))),
		       true, RleVersion_2, *getDefaultPool());
  std::vector<int64_t> data(3);
  rle->next(data.data(), 3, nullptr);

  for (size_t i = 0; i < data.size(); ++i) {
    EXPECT_EQ(i, data[i]) << "Output wrong at " << i;
  }
};

TEST(RLEv2, bitSize1Direct) {
  // 0,1 repeated 20 times (unsigned ints)
  size_t count = 40;
  std::vector<int64_t> values;
  for (size_t i = 0; i < count; ++i) {
    values.push_back(i % 2);
  }
  const unsigned char bytes[] = {0x40, 0x27, 0x55, 0x55, 0x55, 0x55, 0x55};
  unsigned long l = sizeof(bytes) / sizeof(char);
  for (uint32_t blkSize = 1; blkSize <= l; ++blkSize) {
    // Read 1 at a time, then 3 at a time, etc.
    checkResults(values, decodeRLEv2(bytes, l, 1, count, nullptr, false, blkSize), 1);
    checkResults(values, decodeRLEv2(bytes, l, 3, count, nullptr, false, blkSize), 3);
    checkResults(values, decodeRLEv2(bytes, l, 7, count, nullptr, false, blkSize), 7);
    checkResults(values, decodeRLEv2(bytes, l, count, count, nullptr, false, blkSize), count);
  }

  values.clear();
  // 0,1,1 repeated 10 times (unsigned ints)
  for (size_t i = 0; i < 10; ++i) {
    values.push_back(0);
    values.push_back(1);
    values.push_back(1);
  }
  // bytes2: 0100,0000,0001,1101,0110,1101,1011,0110,1101,1011,0110,1100
  // First byte: 01 for encoding, 00000 for bitSize index (0), 0 for first bit of len - 1.
  // Second byte: 0001,1101 for len - 1 = 29.
  // Following bits repeating 011 ten times. Last byte padding with 00.
  const unsigned char bytes2[] = {0x40, 0x1D, 0x6D, 0xB6, 0xDB, 0x6C};
  l = sizeof(bytes2) / sizeof(char);
  count = 30;
  for (uint32_t blkSize = 1; blkSize <= l; ++blkSize) {
    // Read 1 at a time, then 3 at a time, etc.
    checkResults(values, decodeRLEv2(bytes2, l, 1, count, nullptr, false, blkSize), 1);
    checkResults(values, decodeRLEv2(bytes2, l, 3, count, nullptr, false, blkSize), 3);
    checkResults(values, decodeRLEv2(bytes2, l, 7, count, nullptr, false, blkSize), 7);
    checkResults(values, decodeRLEv2(bytes2, l, count, count, nullptr, false, blkSize), count);
  }
}

TEST(RLEv2, bitSize2Direct) {
  // 0,1 repeated 10 times (signed ints)
  const size_t count = 20;
  std::vector<int64_t> values;
  for (size_t i = 0; i < count; ++i) {
    values.push_back(i % 2);
  }

  const unsigned char bytes[] = {0x42, 0x13, 0x22, 0x22, 0x22, 0x22, 0x22};
  unsigned long l = sizeof(bytes) / sizeof(char);
  for (uint32_t blkSize = 1; blkSize <= l; ++blkSize) {
    // Read 1 at a time, then 3 at a time, etc.
    checkResults(values, decodeRLEv2(bytes, l, 1, count, nullptr, true, blkSize), 1);
    checkResults(values, decodeRLEv2(bytes, l, 3, count, nullptr, true, blkSize), 3);
    checkResults(values, decodeRLEv2(bytes, l, 7, count, nullptr, true, blkSize), 7);
    checkResults(values, decodeRLEv2(bytes, l, count, count, nullptr, true, blkSize), count);
  }
}

TEST(RLEv2, bitSize4Direct) {
  // 0,2 repeated 10 times (signed ints)
  size_t count = 20;
  std::vector<int64_t> values;
  for (size_t i = 0; i < count; ++i) {
     values.push_back((i%2)*2);
  }

  const unsigned char bytes[] = {0x46,0x13,0x04,0x04,0x04,0x04,
                                 0x04,0x04,0x04,0x04,0x04,0x04};
  unsigned long l = sizeof(bytes) / sizeof(char);
  for (uint32_t blkSize = 1; blkSize <= l; ++blkSize) {
    // Read 1 at a time, then 3 at a time, etc.
    checkResults(values, decodeRLEv2(bytes, l, 1, count, nullptr, true, blkSize), 1);
    checkResults(values, decodeRLEv2(bytes, l, 3, count, nullptr, true, blkSize), 3);
    checkResults(values, decodeRLEv2(bytes, l, 7, count, nullptr, true, blkSize), 7);
    checkResults(values, decodeRLEv2(bytes, l, count, count, nullptr, true, blkSize), count);
  }

  // -2,0,2,4 repeated 5 times (signed ints)
  count = 20;
  values.clear();
  for (size_t i = 0; i < count; ++i) {
    values.push_back((i % 4) * 2 - 2);
  }

  const unsigned char bytes2[] = {0x46,0x13,0x30,0x48,0x30,0x48,
                                  0x30,0x48,0x30,0x48,0x30,0x48};
  l = sizeof(bytes) / sizeof(char);
  for (uint32_t blkSize = 1; blkSize <= l; ++blkSize) {
    // Read 1 at a time, then 3 at a time, etc.
    checkResults(values, decodeRLEv2(bytes2, l, 1, count, nullptr, true, blkSize), 1);
    checkResults(values, decodeRLEv2(bytes2, l, 3, count, nullptr, true, blkSize), 3);
    checkResults(values, decodeRLEv2(bytes2, l, 7, count, nullptr, true, blkSize), 7);
    checkResults(values, decodeRLEv2(bytes2, l, count, count, nullptr, true, blkSize), count);
  }
}

TEST(RLEv2, bitSize8Direct) {
  const size_t count = 20;
  std::vector<int64_t> values;
  for (int i = 0; i < count; ++i) {
    values.push_back(i);
  }

  const unsigned char bytes[] = {0x4E,0x13,0x00,0x02,0x04,0x06,0x08,0x0A,0x0C,0x0E,0x10,
                                 0x12,0x14,0x16,0x18,0x1A,0x1C,0x1E,0x20,0x22,0x24,0x26};
  unsigned long l = sizeof(bytes) / sizeof(char);
  for (uint32_t blkSize = 1; blkSize <= l; ++blkSize) {
    // Read 1 at a time, then 3 at a time, etc.
    checkResults(values, decodeRLEv2(bytes, l, 1, count, nullptr, true, blkSize), 1);
    checkResults(values, decodeRLEv2(bytes, l, 3, count, nullptr, true, blkSize), 3);
    checkResults(values, decodeRLEv2(bytes, l, 7, count, nullptr, true, blkSize), 7);
    checkResults(values, decodeRLEv2(bytes, l, count, count, nullptr, true, blkSize), count);
  }
}

TEST(RLEv2, bitSize16Direct) {
  const size_t count = 20;
  std::vector<int64_t> values;
  for (int i = 0; i < count; ++i) {
    values.push_back((i << 8) + i);
  }

  const unsigned char bytes[] = {0x5E,0x13,
                                 0x00,0x00,0x02,0x02,0x04,0x04,0x06,0x06,
                                 0x08,0x08,0x0A,0x0A,0x0C,0x0C,0x0E,0x0E,
                                 0x10,0x10,0x12,0x12,0x14,0x14,0x16,0x16,
                                 0x18,0x18,0x1A,0x1A,0x1C,0x1C,0x1E,0x1E,
                                 0x20,0x20,0x22,0x22,0x24,0x24,0x26,0x26};
  unsigned long l = sizeof(bytes) / sizeof(char);
  for (uint32_t blkSize = 1; blkSize <= l; ++blkSize) {
    // Read 1 at a time, then 3 at a time, etc.
    checkResults(values, decodeRLEv2(bytes, l, 1, count, nullptr, true, blkSize), 3);
    checkResults(values, decodeRLEv2(bytes, l, 3, count, nullptr, true, blkSize), 3);
    checkResults(values, decodeRLEv2(bytes, l, 7, count, nullptr, true, blkSize), 7);
    checkResults(values, decodeRLEv2(bytes, l, count, count, nullptr, true, blkSize), count);
  }
}

TEST(RLEv2, bitSize24Direct) {
  const size_t count = 20;
  std::vector<int64_t> values;
  for (int64_t i = 0; i < count; ++i) {
    values.push_back((i << 16) + (i << 8) + i);
  }

  const unsigned char bytes[] = {0x6E,0x13,
                                 0x00,0x00,0x00,0x02,0x02,0x02,0x04,0x04,0x04,
                                 0x06,0x06,0x06,0x08,0x08,0x08,0x0A,0x0A,0x0A,
                                 0x0C,0x0C,0x0C,0x0E,0x0E,0x0E,0x10,0x10,0x10,
                                 0x12,0x12,0x12,0x14,0x14,0x14,0x16,0x16,0x16,
                                 0x18,0x18,0x18,0x1A,0x1A,0x1A,0x1C,0x1C,0x1C,
                                 0x1E,0x1E,0x1E,0x20,0x20,0x20,0x22,0x22,0x22,
                                 0x24,0x24,0x24,0x26,0x26,0x26};
  unsigned long l = sizeof(bytes) / sizeof(char);
  for (uint32_t blkSize = 1; blkSize <= l; ++blkSize) {
    // Read 1 at a time, then 3 at a time, etc.
    checkResults(values, decodeRLEv2(bytes, l, 1, count, nullptr, true, blkSize), 3);
    checkResults(values, decodeRLEv2(bytes, l, 3, count, nullptr, true, blkSize), 3);
    checkResults(values, decodeRLEv2(bytes, l, 7, count, nullptr, true, blkSize), 7);
    checkResults(values, decodeRLEv2(bytes, l, count, count, nullptr, true, blkSize), count);
  }
}

TEST(RLEv2, bitSize32Direct) {
  const size_t count = 20;
  std::vector<int64_t> values;
  for (int64_t i = 0; i < count; ++i) {
    values.push_back((i << 24) + (i << 16) + (i << 8) + i);
  }

  const unsigned char bytes[] = {0x76,0x13,
                                 0x00,0x00,0x00,0x00,0x02,0x02,0x02,0x02,
                                 0x04,0x04,0x04,0x04,0x06,0x06,0x06,0x06,
                                 0x08,0x08,0x08,0x08,0x0A,0x0A,0x0A,0x0A,
                                 0x0C,0x0C,0x0C,0x0C,0x0E,0x0E,0x0E,0x0E,
                                 0x10,0x10,0x10,0x10,0x12,0x12,0x12,0x12,
                                 0x14,0x14,0x14,0x14,0x16,0x16,0x16,0x16,
                                 0x18,0x18,0x18,0x18,0x1A,0x1A,0x1A,0x1A,
                                 0x1C,0x1C,0x1C,0x1C,0x1E,0x1E,0x1E,0x1E,
                                 0x20,0x20,0x20,0x20,0x22,0x22,0x22,0x22,
                                 0x24,0x24,0x24,0x24,0x26,0x26,0x26,0x26};
  unsigned long l = sizeof(bytes) / sizeof(char);
  for (uint32_t blkSize = 1; blkSize <= l; ++blkSize) {
    // Read 1 at a time, then 3 at a time, etc.
    checkResults(values, decodeRLEv2(bytes, l, 1, count, nullptr, true, blkSize), 3);
    checkResults(values, decodeRLEv2(bytes, l, 3, count, nullptr, true, blkSize), 3);
    checkResults(values, decodeRLEv2(bytes, l, 7, count, nullptr, true, blkSize), 7);
    checkResults(values, decodeRLEv2(bytes, l, count, count, nullptr, true, blkSize), count);
  }
}

TEST(RLEv2, bitSize40Direct) {
  const size_t count = 20;
  std::vector<int64_t> values;
  for (int64_t i = 0; i < count; ++i) {
    values.push_back((i << 32) + (i << 24) + (i << 16) + (i << 8) + i);
  }

  const unsigned char bytes[] = {0x78,0x13,
                                 0x00,0x00,0x00,0x00,0x00,
                                 0x02,0x02,0x02,0x02,0x02,
                                 0x04,0x04,0x04,0x04,0x04,
                                 0x06,0x06,0x06,0x06,0x06,
                                 0x08,0x08,0x08,0x08,0x08,
                                 0x0A,0x0A,0x0A,0x0A,0x0A,
                                 0x0C,0x0C,0x0C,0x0C,0x0C,
                                 0x0E,0x0E,0x0E,0x0E,0x0E,
                                 0x10,0x10,0x10,0x10,0x10,
                                 0x12,0x12,0x12,0x12,0x12,
                                 0x14,0x14,0x14,0x14,0x14,
                                 0x16,0x16,0x16,0x16,0x16,
                                 0x18,0x18,0x18,0x18,0x18,
                                 0x1A,0x1A,0x1A,0x1A,0x1A,
                                 0x1C,0x1C,0x1C,0x1C,0x1C,
                                 0x1E,0x1E,0x1E,0x1E,0x1E,
                                 0x20,0x20,0x20,0x20,0x20,
                                 0x22,0x22,0x22,0x22,0x22,
                                 0x24,0x24,0x24,0x24,0x24,
                                 0x26,0x26,0x26,0x26,0x26};
  unsigned long l = sizeof(bytes) / sizeof(char);
  for (uint32_t blkSize = 1; blkSize <= l; ++blkSize) {
    // Read 1 at a time, then 3 at a time, etc.
    checkResults(values, decodeRLEv2(bytes, l, 1, count, nullptr, true, blkSize), 3);
    checkResults(values, decodeRLEv2(bytes, l, 3, count, nullptr, true, blkSize), 3);
    checkResults(values, decodeRLEv2(bytes, l, 7, count, nullptr, true, blkSize), 7);
    checkResults(values, decodeRLEv2(bytes, l, count, count, nullptr, true, blkSize), count);
  }
}

TEST(RLEv2, bitSize48Direct) {
  const size_t count = 20;
  std::vector<int64_t> values;
  for (int64_t i = 0; i < count; ++i) {
    values.push_back((i << 40) + (i << 32) + (i << 24) + (i << 16) + (i << 8) + i);
  }

  const unsigned char bytes[] = {0x7A,0x13,
                                 0x00,0x00,0x00,0x00,0x00,0x00,
                                 0x02,0x02,0x02,0x02,0x02,0x02,
                                 0x04,0x04,0x04,0x04,0x04,0x04,
                                 0x06,0x06,0x06,0x06,0x06,0x06,
                                 0x08,0x08,0x08,0x08,0x08,0x08,
                                 0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,
                                 0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,
                                 0x0E,0x0E,0x0E,0x0E,0x0E,0x0E,
                                 0x10,0x10,0x10,0x10,0x10,0x10,
                                 0x12,0x12,0x12,0x12,0x12,0x12,
                                 0x14,0x14,0x14,0x14,0x14,0x14,
                                 0x16,0x16,0x16,0x16,0x16,0x16,
                                 0x18,0x18,0x18,0x18,0x18,0x18,
                                 0x1A,0x1A,0x1A,0x1A,0x1A,0x1A,
                                 0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,
                                 0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,
                                 0x20,0x20,0x20,0x20,0x20,0x20,
                                 0x22,0x22,0x22,0x22,0x22,0x22,
                                 0x24,0x24,0x24,0x24,0x24,0x24,
                                 0x26,0x26,0x26,0x26,0x26,0x26};
  unsigned long l = sizeof(bytes) / sizeof(char);
  for (uint32_t blkSize = 1; blkSize <= l; ++blkSize) {
    // Read 1 at a time, then 3 at a time, etc.
    checkResults(values, decodeRLEv2(bytes, l, 1, count, nullptr, true, blkSize), 3);
    checkResults(values, decodeRLEv2(bytes, l, 3, count, nullptr, true, blkSize), 3);
    checkResults(values, decodeRLEv2(bytes, l, 7, count, nullptr, true, blkSize), 7);
    checkResults(values, decodeRLEv2(bytes, l, count, count, nullptr, true, blkSize), count);
  }
}

TEST(RLEv2, bitSize56Direct) {
  // 0,2 repeated 10 times (signed ints)
  const size_t count = 20;
  std::vector<int64_t> values;
  for (int64_t i = 0; i < count; ++i) {
    values.push_back((i << 48) + (i << 40) + (i << 32) + (i << 24) + (i << 16) + (i << 8) + i);
  }

  const unsigned char bytes[] = {0x7C,0x13,
                                 0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                 0x02,0x02,0x02,0x02,0x02,0x02,0x02,
                                 0x04,0x04,0x04,0x04,0x04,0x04,0x04,
                                 0x06,0x06,0x06,0x06,0x06,0x06,0x06,
                                 0x08,0x08,0x08,0x08,0x08,0x08,0x08,
                                 0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,
                                 0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,
                                 0x0E,0x0E,0x0E,0x0E,0x0E,0x0E,0x0E,
                                 0x10,0x10,0x10,0x10,0x10,0x10,0x10,
                                 0x12,0x12,0x12,0x12,0x12,0x12,0x12,
                                 0x14,0x14,0x14,0x14,0x14,0x14,0x14,
                                 0x16,0x16,0x16,0x16,0x16,0x16,0x16,
                                 0x18,0x18,0x18,0x18,0x18,0x18,0x18,
                                 0x1A,0x1A,0x1A,0x1A,0x1A,0x1A,0x1A,
                                 0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,
                                 0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,
                                 0x20,0x20,0x20,0x20,0x20,0x20,0x20,
                                 0x22,0x22,0x22,0x22,0x22,0x22,0x22,
                                 0x24,0x24,0x24,0x24,0x24,0x24,0x24,
                                 0x26,0x26,0x26,0x26,0x26,0x26,0x26};
  unsigned long l = sizeof(bytes) / sizeof(char);
  for (uint32_t blkSize = 1; blkSize <= l; ++blkSize) {
    // Read 1 at a time, then 3 at a time, etc.
    checkResults(values, decodeRLEv2(bytes, l, 1, count, nullptr, true, blkSize), 3);
    checkResults(values, decodeRLEv2(bytes, l, 3, count, nullptr, true, blkSize), 3);
    checkResults(values, decodeRLEv2(bytes, l, 7, count, nullptr, true, blkSize), 7);
    checkResults(values, decodeRLEv2(bytes, l, count, count, nullptr, true, blkSize), count);
  }
}

TEST(RLEv2, bitSize64Direct) {
  const size_t count = 20;
  std::vector<int64_t> values;
  for (int64_t i = 0; i < count; ++i) {
    values.push_back((i << 56) + (i << 48) + (i << 40) + (i << 32) + (i << 24) + (i << 16) + (i << 8) + i);
  }

  const unsigned char bytes[] = {0x7E,0x13,
                                 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                 0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,
                                 0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
                                 0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,
                                 0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
                                 0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,
                                 0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,
                                 0x0E,0x0E,0x0E,0x0E,0x0E,0x0E,0x0E,0x0E,
                                 0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
                                 0x12,0x12,0x12,0x12,0x12,0x12,0x12,0x12,
                                 0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x14,
                                 0x16,0x16,0x16,0x16,0x16,0x16,0x16,0x16,
                                 0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
                                 0x1A,0x1A,0x1A,0x1A,0x1A,0x1A,0x1A,0x1A,
                                 0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,
                                 0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,
                                 0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
                                 0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,
                                 0x24,0x24,0x24,0x24,0x24,0x24,0x24,0x24,
                                 0x26,0x26,0x26,0x26,0x26,0x26,0x26,0x26};
  unsigned long l = sizeof(bytes) / sizeof(char);
  for (uint32_t blkSize = 1; blkSize <= l; ++blkSize) {
    // Read 1 at a time, then 3 at a time, etc.
    checkResults(values, decodeRLEv2(bytes, l, 1, count, nullptr, true, blkSize), 3);
    checkResults(values, decodeRLEv2(bytes, l, 3, count, nullptr, true, blkSize), 3);
    checkResults(values, decodeRLEv2(bytes, l, 7, count, nullptr, true, blkSize), 7);
    checkResults(values, decodeRLEv2(bytes, l, count, count, nullptr, true, blkSize), count);
  }
}

TEST(RLEv2, multipleRunsDirect) {
  std::vector<int64_t> values;
  // 0,1 repeated 10 times (signed ints)
  for (size_t i = 0; i < 20; ++i) {
     values.push_back(i%2);
  }
  // 0,2 repeated 10 times (signed ints)
  for (size_t i = 0; i < 20; ++i) {
     values.push_back((i%2)*2);
  }

  const unsigned char bytes[] = {0x42,0x13,0x22,0x22,0x22,0x22,0x22,
                                 0x46,0x13,0x04,0x04,0x04,0x04,0x04,
                                 0x04,0x04,0x04,0x04,0x04};
  unsigned long l = sizeof(bytes) / sizeof(char);

  size_t count = values.size();
  for (uint32_t blkSize = 1; blkSize <= l; ++blkSize) {
    // Read 1 at a time, then 3 at a time, etc.
    checkResults(values, decodeRLEv2(bytes, l, 1, count, nullptr, true, blkSize), 3);
    checkResults(values, decodeRLEv2(bytes, l, 3, count, nullptr, true, blkSize), 3);
    checkResults(values, decodeRLEv2(bytes, l, 7, count, nullptr, true, blkSize), 7);
    checkResults(values, decodeRLEv2(bytes, l, count, count, nullptr, true, blkSize), count);
  }
}