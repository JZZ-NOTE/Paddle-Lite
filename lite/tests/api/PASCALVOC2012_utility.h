// Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include "lite/utils/io.h"
#include "lite/utils/log/cp_logging.h"
#include "lite/utils/string.h"

namespace paddle {
namespace lite {

template <class T = float>
std::vector<std::vector<T>> ReadRawData(
    const std::string& raw_data_dir,
    const std::vector<int>& input_shape = {1, 3, 300, 300},
    int iteration = 1) {
  std::vector<std::vector<T>> raw_data;

  int image_size = 1;
  for (size_t i = 1; i < input_shape.size(); i++) {
    image_size *= input_shape[i];
  }
  int input_size = image_size * input_shape[0];

  for (int i = 0; i < iteration; i++) {
    std::vector<T> one_iter_raw_data;
    one_iter_raw_data.resize(input_size);
    T* data = &(one_iter_raw_data.at(0));
    for (int j = 0; j < input_shape[0]; j++) {
      std::string raw_data_file_dir =
          raw_data_dir + std::string("/") +
          std::to_string(i * input_shape[0] + j + 1);
      std::ifstream fin(raw_data_file_dir, std::ios::in | std::ios::binary);
      CHECK(fin.is_open()) << "failed to open file " << raw_data_file_dir;
      fin.seekg(0, std::ios::end);
      int file_size = fin.tellg();
      fin.seekg(0, std::ios::beg);
      CHECK_EQ(static_cast<size_t>(file_size),
               static_cast<size_t>(image_size) * sizeof(T) / sizeof(char));
      fin.read(reinterpret_cast<char*>(data), file_size);
      fin.close();
      data += image_size;
    }
    raw_data.emplace_back(one_iter_raw_data);
  }

  return raw_data;
}

}  // namespace lite
}  // namespace paddle
