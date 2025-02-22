// Copyright (c) 2021 PaddlePaddle Authors. All Rights Reserved.
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

#include "lite/kernels/host/prior_box_compute.h"
#include <vector>
using pb_fp32 = paddle::lite::kernels::host::PriorBoxCompute<float,
                                                             TARGET(kHost),
                                                             PRECISION(kFloat)>;
REGISTER_LITE_KERNEL(prior_box, kHost, kFloat, kNCHW, pb_fp32, def)
    .BindInput("Input", {LiteType::GetTensorTy(TARGET(kHost))})
    .BindInput("Image", {LiteType::GetTensorTy(TARGET(kHost))})
    .BindOutput("Boxes", {LiteType::GetTensorTy(TARGET(kHost))})
    .BindOutput("Variances", {LiteType::GetTensorTy(TARGET(kHost))})
    .Finalize();
#ifdef ENABLE_ARM_FP16
using pb_fp16 = paddle::lite::kernels::host::PriorBoxCompute<float16_t,
                                                             TARGET(kARM),
                                                             PRECISION(kFP16)>;
REGISTER_LITE_KERNEL(prior_box, kARM, kFP16, kNCHW, pb_fp16, def)
    .BindInput("Input", {LiteType::GetTensorTy(TARGET(kARM), PRECISION(kFP16))})
    .BindInput("Image", {LiteType::GetTensorTy(TARGET(kARM), PRECISION(kFP16))})
    .BindOutput("Boxes",
                {LiteType::GetTensorTy(TARGET(kARM), PRECISION(kFP16))})
    .BindOutput("Variances",
                {LiteType::GetTensorTy(TARGET(kARM), PRECISION(kFP16))})
    .Finalize();
#endif  // ENABLE_ARM_FP16
