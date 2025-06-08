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
#include "postprocess_op.h"

namespace PaddleOCR
{

    class DBDetector
    {
    public:
        DBDetector();

        // Load Paddle inference model
        void LoadModel(const std::string &model_dir);

        // Run predictor
        void Run(cv::Mat &img, std::vector<std::vector<std::vector<int>>> &boxes,
                 std::vector<double> &times);
        cv::dnn::Net predictor_; // 推理库实例

    private:
        bool use_gpu_ = false;
        int gpu_id_ = 0;
        int gpu_mem_ = 4000;
        int cpu_math_library_num_threads_ = 4;
        bool use_mkldnn_ = false;

        std::string limit_type_ = "max";
        int limit_side_len_ = 960;

        double det_db_thresh_ = 0.3;
        double det_db_box_thresh_ = 0.5;
        double det_db_unclip_ratio_ = 2.0;
        std::string det_db_score_mode_ = "slow";
        bool use_dilation_ = false;

        bool visualize_ = true;
        bool use_tensorrt_ = false;
        std::string precision_ = "fp32";

        std::vector<float> mean_ = {0.485f, 0.456f, 0.406f};
        std::vector<float> scale_ = {1 / 0.229f, 1 / 0.224f, 1 / 0.225f};
        bool is_scale_ = true;

        // pre-process
        ResizeImgType0 resize_op_;
        Normalize normalize_op_;
        Permute permute_op_;

        // post-process
        DBPostProcessor post_processor_;
    };

} // namespace PaddleOCR