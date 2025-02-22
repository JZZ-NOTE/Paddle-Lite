// Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.
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

#include "lite/kernels/nnadapter/converter/converter.h"

namespace paddle {
namespace lite {
namespace kernels {
namespace nnadapter {

int ConvertArgMinMax(Converter* converter, OpInfo* op, Scope* scope) {
  // Input operand
  auto x_name = op->Input("X").front();
  auto input_operand = converter->GetMappedOperand(x_name);

  // Axis operand
  int axis = op->GetAttr<int64_t>("axis");
  auto axis_operand = converter->AddConstantOperand(axis);

  // Keepdim operand
  bool keepdim = op->GetAttr<bool>("keepdims");
  auto keepdim_operand =
      converter->AddConstantOperand(static_cast<int8_t>(keepdim));

  // Dtype operand
  int dtype = op->GetAttr<int>("dtype");
  // Default int64
  if (dtype < 0) {
    dtype = 3;
  }
  auto dtype_operand = converter->AddConstantOperand(
      static_cast<int32_t>(ConvertFluidDataTypeToNNPrecisionCode(dtype)));

  // Output operand
  auto out_name = op->Output("Out").front();
  auto output_operand = converter->AddOutputOperand(out_name);

  // Arg operation
  NNAdapterOperationType arg_operation_type;
  auto op_type = op->Type();
  if (op_type == "arg_min") {
    arg_operation_type = NNADAPTER_ARG_MIN;
  } else if (op_type == "arg_max") {
    arg_operation_type = NNADAPTER_ARG_MAX;
  } else {
    LOG(WARNING) << "Unsupported arg operation type: " << op_type;
    return UNSUPPORTED_FEATURE;
  }
  converter->AddOperation(
      arg_operation_type,
      {input_operand, axis_operand, keepdim_operand, dtype_operand},
      {output_operand});
  return NO_ERROR;
}

}  // namespace nnadapter
}  // namespace kernels
}  // namespace lite
}  // namespace paddle
