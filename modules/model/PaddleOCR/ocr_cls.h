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

#include <opencv2/opencv.hpp>
#include "preprocess_op.h"
#include "utility.h"

namespace PaddleOCR
{

    class Classifier
    {
    public:
        explicit Classifier();
        double cls_thresh = 0.9;

        // Load Paddle inference model
        void LoadModel(const std::string &model_dir);

        void Run(std::vector<cv::Mat> img_list, std::vector<int> &cls_labels,
                 std::vector<float> &cls_scores, std::vector<double> &times);
        cv::dnn::Net predictor_; // 推理库实例

    private:
        bool use_gpu_ = false;
        int gpu_id_ = 0;
        int gpu_mem_ = 4000;
        int cpu_math_library_num_threads_ = 4;
        bool use_mkldnn_ = false;

        std::vector<float> mean_ = {0.5f, 0.5f, 0.5f};
        std::vector<float> scale_ = {1 / 0.5f, 1 / 0.5f, 1 / 0.5f};
        bool is_scale_ = true;
        bool use_tensorrt_ = false;
        std::string precision_ = "fp32";
        int cls_batch_num_ = 1;
        // pre-process
        ClsResizeImg resize_op_;
        Normalize normalize_op_;
        PermuteBatch permute_op_;

    }; // class Classifier

} // namespace PaddleOCR
