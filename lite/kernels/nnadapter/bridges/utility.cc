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

#include "lite/kernels/nnadapter/bridges/utility.h"
#include <cmath>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "lite/core/op_lite.h"
#include "lite/core/tensor.h"

namespace paddle {
namespace lite {
namespace subgraph {
namespace nnadapter {

bool HasInput(const OpInfo* op_info,
              const Scope* scope,
              const std::string& arg_name) {
  return op_info->HasInput(arg_name) && op_info->Input(arg_name).size() > 0 &&
         scope->FindVar(op_info->Input(arg_name).front());
}

bool HasOutput(const OpInfo* op_info,
               const Scope* scope,
               const std::string& arg_name) {
  return op_info->HasOutput(arg_name) && op_info->Output(arg_name).size() > 0 &&
         scope->FindVar(op_info->Output(arg_name).front());
}

bool IsPerChannelScales(const std::vector<float>& scales) {
  const float threshold = 1e-5f;
  size_t size = scales.size();
  CHECK_GT(size, 0) << "The size of scales should be greater than 0.";
  auto ref_scale = scales[0];
  for (size_t i = 1; i < size; i++) {
    auto cur_scale = scales[i];
    if (std::fabs(cur_scale - ref_scale) > threshold) {
      return true;
    }
  }
  return false;
}

bool IsPrecisionCompatible(const NNAdapterOperandType* target,
                           const PrecisionType reference) {
  bool compatiable = false;
  switch (target->precision) {
    case NNADAPTER_TENSOR_BOOL8:
      compatiable = reference == PRECISION(kBool);
      break;
    case NNADAPTER_TENSOR_INT8:
    case NNADAPTER_TENSOR_QUANT_INT8_SYMM_PER_LAYER:
    case NNADAPTER_TENSOR_QUANT_INT8_SYMM_PER_CHANNEL:
      compatiable = reference == PRECISION(kInt8);
      break;
    case NNADAPTER_TENSOR_INT16:
      compatiable = reference == PRECISION(kInt16);
      break;
    case NNADAPTER_TENSOR_INT32:
    case NNADAPTER_TENSOR_QUANT_INT32_SYMM_PER_LAYER:
    case NNADAPTER_TENSOR_QUANT_INT32_SYMM_PER_CHANNEL:
      compatiable = reference == PRECISION(kInt32);
      break;
    case NNADAPTER_TENSOR_INT64:
      compatiable = reference == PRECISION(kInt64);
      break;
    case NNADAPTER_TENSOR_FLOAT16:
      compatiable = reference == PRECISION(kFP16);
      break;
    case NNADAPTER_TENSOR_FLOAT32:
      compatiable = reference == PRECISION(kFloat);
      break;
    case NNADAPTER_TENSOR_FLOAT64:
      compatiable = reference == PRECISION(kFP64);
      break;
    default:
      break;
  }
  return compatiable;
}

bool IsLayoutCompatible(const NNAdapterOperandType* target,
                        const DataLayoutType& reference) {
  bool compatiable = false;
  switch (target->layout) {
    case NNADAPTER_NCHW:
      compatiable = reference == DATALAYOUT(kNCHW);
      break;
    case NNADAPTER_NHWC:
      compatiable = reference == DATALAYOUT(kNHWC);
      break;
    default:
      break;
  }
  return true;
}

bool IsDimensionCompatible(const NNAdapterOperandType* target,
                           const DDim& reference) {
  bool compatiable = target->dimensions.count == reference.size();
  if (compatiable) {
    for (size_t i = 0; i < target->dimensions.count; i++) {
      // -1 mean Any for dynamic shape
      if (target->dimensions.data[i] != -1 &&
          target->dimensions.data[i] != reference[i]) {
        compatiable = false;
        break;
      }
    }
  }
  return compatiable;
}

std::vector<int32_t> ConvertDimensions(const DDim& input_dimensions) {
  std::vector<int32_t> output_dimensions(input_dimensions.size());
  for (size_t i = 0; i < input_dimensions.size(); i++) {
    output_dimensions[i] = static_cast<int32_t>(input_dimensions[i]);
  }
  return output_dimensions;
}

std::vector<int32_t> ConvertDimensions(
    const std::vector<int64_t>& input_dimensions) {
  std::vector<int32_t> output_dimensions(input_dimensions.size());
  for (size_t i = 0; i < input_dimensions.size(); i++) {
    output_dimensions[i] = static_cast<int32_t>(input_dimensions[i]);
  }
  return output_dimensions;
}

void ConvertDimensions(const DDim& input_dimensions,
                       int32_t* output_dimensions,
                       uint32_t* output_dimension_count) {
  CHECK(output_dimensions);
  CHECK(output_dimension_count);
  *output_dimension_count = input_dimensions.size();
  for (size_t i = 0; i < input_dimensions.size(); i++) {
    output_dimensions[i] = static_cast<int32_t>(input_dimensions[i]);
  }
}

void ConvertDimensions(const std::vector<int64_t>& input_dimensions,
                       int32_t* output_dimensions,
                       uint32_t* output_dimension_count) {
  CHECK(output_dimensions);
  CHECK(output_dimension_count);
  *output_dimension_count = input_dimensions.size();
  for (size_t i = 0; i < input_dimensions.size(); i++) {
    output_dimensions[i] = static_cast<int32_t>(input_dimensions[i]);
  }
}

DDim ConvertDimensions(int32_t* input_dimensions,
                       uint32_t input_dimension_count) {
  CHECK(input_dimensions);
  std::vector<int64_t> output_dimensions(input_dimension_count);
  for (int i = 0; i < input_dimension_count; i++) {
    output_dimensions[i] = static_cast<int64_t>(input_dimensions[i]);
  }
  return DDim(output_dimensions);
}

PrecisionType ConvertPrecision(NNAdapterOperandPrecisionCode input_precision) {
  PrecisionType output_precision = PRECISION(kUnk);
  switch (input_precision) {
    case NNADAPTER_TENSOR_BOOL8:
      output_precision = PRECISION(kBool);
      break;
    case NNADAPTER_TENSOR_INT8:
    case NNADAPTER_TENSOR_QUANT_INT8_SYMM_PER_LAYER:
    case NNADAPTER_TENSOR_QUANT_INT8_SYMM_PER_CHANNEL:
    case NNADAPTER_TENSOR_QUANT_UINT8_ASYMM_PER_LAYER:
      output_precision = PRECISION(kInt8);
      break;
    case NNADAPTER_TENSOR_INT16:
      output_precision = PRECISION(kInt16);
      break;
    case NNADAPTER_TENSOR_INT32:
    case NNADAPTER_TENSOR_QUANT_INT32_SYMM_PER_LAYER:
    case NNADAPTER_TENSOR_QUANT_INT32_SYMM_PER_CHANNEL:
    case NNADAPTER_TENSOR_QUANT_UINT32_ASYMM_PER_LAYER:
      output_precision = PRECISION(kInt32);
      break;
    case NNADAPTER_TENSOR_INT64:
      output_precision = PRECISION(kInt64);
      break;
    case NNADAPTER_TENSOR_FLOAT16:
      output_precision = PRECISION(kFP16);
      break;
    case NNADAPTER_TENSOR_FLOAT32:
      output_precision = PRECISION(kFloat);
      break;
    case NNADAPTER_TENSOR_FLOAT64:
      output_precision = PRECISION(kFP64);
      break;
    default:
      LOG(FATAL) << "Failed to convert the NNAdapter operand precision code("
                 << static_cast<int>(input_precision) << ") to PrecisionType !";
      break;
  }
  return output_precision;
}

int PrecisionLength(NNAdapterOperandPrecisionCode precision) {
  switch (precision) {
    case NNADAPTER_BOOL8:
    case NNADAPTER_INT8:
    case NNADAPTER_UINT8:
    case NNADAPTER_TENSOR_BOOL8:
    case NNADAPTER_TENSOR_INT8:
    case NNADAPTER_TENSOR_UINT8:
    case NNADAPTER_TENSOR_QUANT_INT8_SYMM_PER_LAYER:
    case NNADAPTER_TENSOR_QUANT_INT8_SYMM_PER_CHANNEL:
    case NNADAPTER_TENSOR_QUANT_UINT8_ASYMM_PER_LAYER:
      return 1;
    case NNADAPTER_INT16:
    case NNADAPTER_UINT16:
    case NNADAPTER_FLOAT16:
    case NNADAPTER_TENSOR_INT16:
    case NNADAPTER_TENSOR_UINT16:
    case NNADAPTER_TENSOR_FLOAT16:
      return 2;
    case NNADAPTER_INT32:
    case NNADAPTER_UINT32:
    case NNADAPTER_FLOAT32:
    case NNADAPTER_TENSOR_INT32:
    case NNADAPTER_TENSOR_UINT32:
    case NNADAPTER_TENSOR_FLOAT32:
    case NNADAPTER_TENSOR_QUANT_INT32_SYMM_PER_LAYER:
    case NNADAPTER_TENSOR_QUANT_INT32_SYMM_PER_CHANNEL:
    case NNADAPTER_TENSOR_QUANT_UINT32_ASYMM_PER_LAYER:
      return 4;
    case NNADAPTER_INT64:
    case NNADAPTER_UINT64:
    case NNADAPTER_FLOAT64:
    case NNADAPTER_TENSOR_INT64:
    case NNADAPTER_TENSOR_UINT64:
    case NNADAPTER_TENSOR_FLOAT64:
      return 8;
    default:
      LOG(ERROR) << "Failed to get the length of type("
                 << static_cast<int>(precision) << ").";
      break;
  }
  return 0;
}

NNAdapterOperandPrecisionCode FluidDataType2NNAdapterTensorPrecisionCode(
    int dtype_code) {
  NNAdapterOperandPrecisionCode precision_code = NNADAPTER_TENSOR_FLOAT32;
  switch (dtype_code) {
    case 0:  // BOOL = 0;
      precision_code = NNADAPTER_TENSOR_BOOL8;
      break;
    case 1:  // INT16 = 1
      precision_code = NNADAPTER_TENSOR_INT16;
      break;
    case 2:  // INT32 = 2
      precision_code = NNADAPTER_TENSOR_INT32;
      break;
    case 3:  // INT64 = 3
      precision_code = NNADAPTER_TENSOR_INT64;
      break;
    case 4:  // FP16 = 4
      precision_code = NNADAPTER_TENSOR_FLOAT16;
      break;
    case 5:  // FP32 = 5
      precision_code = NNADAPTER_TENSOR_FLOAT32;
      break;
    case 6:  // FP64 = 6
      precision_code = NNADAPTER_TENSOR_FLOAT64;
      break;
    case 20:  // UINT8 = 20
      precision_code = NNADAPTER_TENSOR_UINT8;
      break;
    case 21:  // INT8 = 21
      precision_code = NNADAPTER_TENSOR_INT8;
      break;
    default:
      LOG(FATAL) << "Unsupported data type: " << dtype_code;
      break;
  }
  return precision_code;
}

NNAdapterOperandPrecisionCode FluidDataType2NNAdapterScalarPrecisionCode(
    int dtype_code) {
  NNAdapterOperandPrecisionCode precision_code = NNADAPTER_FLOAT32;
  switch (dtype_code) {
    case 0:                              // BOOL = 0;
      precision_code = NNADAPTER_BOOL8;  // NNADAPTER_BOOL8 = 0
      break;
    case 1:                              // INT16 = 1
      precision_code = NNADAPTER_INT16;  // NNADAPTER_INT16 = 3
      break;
    case 2:                              // INT32 = 2
      precision_code = NNADAPTER_INT32;  // NNADAPTER_INT32 = 6
      break;
    case 3:                              // INT64 = 3
      precision_code = NNADAPTER_INT64;  // NNADAPTER_INT64 = 7
      break;
    case 4:                                // FP16 = 4
      precision_code = NNADAPTER_FLOAT16;  // NNADAPTER_FLOAT16 = 9
      break;
    case 5:                                // FP32 = 5
      precision_code = NNADAPTER_FLOAT32;  // NNADAPTER_FLOAT32 = 10
      break;
    case 6:                                // FP64 = 6
      precision_code = NNADAPTER_FLOAT64;  // NNADAPTER_FLOAT64 = 11
      break;
    case 20:                             // UINT8 = 20
      precision_code = NNADAPTER_UINT8;  // NNADAPTER_UINT8 = 2
      break;
    case 21:                            // INT8 = 21
      precision_code = NNADAPTER_INT8;  // NNADAPTER_INT8 = 1
      break;
    default:
      LOG(FATAL) << "Unsupported data type: " << dtype_code;
      break;
  }
  return precision_code;
}

NNAdapterOperandPrecisionCode Precision2NNAdapterTensorPrecisionCode(
    PrecisionType precision) {
  NNAdapterOperandPrecisionCode precision_code = NNADAPTER_TENSOR_FLOAT32;
  switch (precision) {
    case PRECISION(kFloat):
      precision_code = NNADAPTER_TENSOR_FLOAT32;
      break;
    case PRECISION(kInt8):
      precision_code = NNADAPTER_TENSOR_INT8;
      break;
    case PRECISION(kInt32):
      precision_code = NNADAPTER_TENSOR_INT32;
      break;
    case PRECISION(kFP16):
      precision_code = NNADAPTER_TENSOR_FLOAT16;
      break;
    case PRECISION(kBool):
      precision_code = NNADAPTER_TENSOR_BOOL8;
      break;
    case PRECISION(kInt64):
      precision_code = NNADAPTER_TENSOR_INT64;
      break;
    case PRECISION(kInt16):
      precision_code = NNADAPTER_TENSOR_INT16;
      break;
    case PRECISION(kUInt8):
      precision_code = NNADAPTER_TENSOR_UINT16;
      break;
    case PRECISION(kFP64):
      precision_code = NNADAPTER_TENSOR_FLOAT64;
      break;
    default:
      LOG(FATAL) << "Unsupported data type: "
                 << lite_api::PrecisionToStr(precision);
      break;
  }
  return precision_code;
}

NNAdapterOperandPrecisionCode Precision2NNAdapterScalarPrecisionCode(
    PrecisionType precision) {
  NNAdapterOperandPrecisionCode precision_code = NNADAPTER_FLOAT32;
  switch (precision) {
    case PRECISION(kFloat):
      precision_code = NNADAPTER_FLOAT32;
      break;
    case PRECISION(kInt8):
      precision_code = NNADAPTER_INT8;
      break;
    case PRECISION(kInt32):
      precision_code = NNADAPTER_INT32;
      break;
    case PRECISION(kFP16):
      precision_code = NNADAPTER_FLOAT16;
      break;
    case PRECISION(kBool):
      precision_code = NNADAPTER_BOOL8;
      break;
    case PRECISION(kInt64):
      precision_code = NNADAPTER_INT64;
      break;
    case PRECISION(kInt16):
      precision_code = NNADAPTER_INT16;
      break;
    case PRECISION(kUInt8):
      precision_code = NNADAPTER_UINT16;
      break;
    case PRECISION(kFP64):
      precision_code = NNADAPTER_FLOAT64;
      break;
    default:
      LOG(FATAL) << "Unsupported data type: "
                 << lite_api::PrecisionToStr(precision);
      break;
  }
  return precision_code;
}

NNAdapterPadModeCode PadMode2NNAdapterPadModeCode(std::string mode) {
  if (mode == "constant") {
    return NNADAPTER_PAD_MODE_CONSTANT;
  }
  if (mode == "reflect") {
    return NNADAPTER_PAD_MODE_REFLECT;
  }
  if (mode == "replicate") {
    return NNADAPTER_PAD_MODE_REPLICATE;
  }
  if (mode == "edge") {
    return NNADAPTER_PAD_MODE_EDGE;
  }
  LOG(FATAL) << "Unsupported mode type: " << mode;
  return NNADAPTER_PAD_MODE_NONE;
}

NNAdapterAutoPadCode PaddingAlgorithm2AutoPadCode(
    const std::string& padding_algorithm) {
  NNAdapterAutoPadCode auto_pad_code;
  if (padding_algorithm == "EXPLICIT" || padding_algorithm.empty()) {
    auto_pad_code = NNADAPTER_AUTO_PAD_NONE;
  } else if (padding_algorithm == "SAME") {
    auto_pad_code = NNADAPTER_AUTO_PAD_SAME;
  } else if (padding_algorithm == "VALID") {
    auto_pad_code = NNADAPTER_AUTO_PAD_VALID;
  } else {
    LOG(FATAL) << "Unsupported padding algorithm: " << padding_algorithm;
  }
  return auto_pad_code;
}

}  // namespace nnadapter
}  // namespace subgraph
}  // namespace lite
}  // namespace paddle
