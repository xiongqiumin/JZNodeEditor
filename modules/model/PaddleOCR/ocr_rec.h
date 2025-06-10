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

#include "ocr_cls.h"
#include "preprocess_op.h"
#include "modules/model/Backend/Onnx/JZModelEngineOnnx.h"

namespace PaddleOCR
{

    class CRNNRecognizer
    {
    public:
        explicit CRNNRecognizer();

        // Load Paddle inference model
        void LoadLabel(const std::string &label_dir);
        void LoadModel(const std::string &model_dir);

        void Run(std::vector<cv::Mat> img_list, std::vector<std::string> &rec_texts,
                 std::vector<float> &rec_text_scores, std::vector<double> &times);
        JZModelEngineOnnx predictor_; // 推理库实例

    private:
        bool use_gpu_ = false;
        int gpu_id_ = 0;
        int gpu_mem_ = 4000;
        int cpu_math_library_num_threads_ = 4;
        bool use_mkldnn_ = false;

        std::vector<std::string> label_list_;

        std::vector<float> mean_ = {0.5f, 0.5f, 0.5f};
        std::vector<float> scale_ = {1 / 0.5f, 1 / 0.5f, 1 / 0.5f};
        bool is_scale_ = true;
        bool use_tensorrt_ = false;
        std::string precision_ = "fp32";
        int rec_batch_num_ = 6;
        int rec_img_h_ = 32;
        int rec_img_w_ = 320;
        std::vector<int> rec_image_shape_ = {3, rec_img_h_, rec_img_w_};
        // pre-process
        CrnnResizeImg resize_op_;
        Normalize normalize_op_;
        PermuteBatch permute_op_;

    }; // class CrnnRecognizer

} // namespace PaddleOCR
