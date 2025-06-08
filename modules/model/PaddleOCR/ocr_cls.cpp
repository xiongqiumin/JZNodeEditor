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

#include "ocr_cls.h"

using namespace cv;

namespace PaddleOCR
{

    Classifier::Classifier()
    {

    }

    void Classifier::Run(std::vector<cv::Mat> img_list,
                         std::vector<int> &cls_labels,
                         std::vector<float> &cls_scores,
                         std::vector<double> &times)
    {
        std::chrono::duration<float> preprocess_diff = std::chrono::duration<float>::zero();
        std::chrono::duration<float> inference_diff = std::chrono::duration<float>::zero();
        std::chrono::duration<float> postprocess_diff = std::chrono::duration<float>::zero();
        
        int img_num = img_list.size();
        cls_labels.resize(img_num);
        cls_scores.resize(img_num);
        
        std::vector<int> cls_image_shape = {3, 48, 192};
        for (int beg_img_no = 0; beg_img_no < img_num;
             beg_img_no += this->cls_batch_num_)
        {
            auto preprocess_start = std::chrono::steady_clock::now();
            int end_img_no = std::min(img_num, beg_img_no + this->cls_batch_num_);
            int batch_num = end_img_no - beg_img_no;
            // preprocess
            std::vector<cv::Mat> norm_img_batch;
            for (int ino = beg_img_no; ino < end_img_no; ino++)
            {
                cv::Mat srcimg;
                img_list[ino].copyTo(srcimg);
                cv::Mat resize_img;
                this->resize_op_.Run(srcimg, resize_img, this->use_tensorrt_,
                                     cls_image_shape);

                this->normalize_op_.Run(&resize_img, this->mean_, this->scale_,
                                        this->is_scale_);
                if (resize_img.cols < cls_image_shape[2])
                {
                    cv::copyMakeBorder(resize_img, resize_img, 0, 0, 0,
                                       cls_image_shape[2] - resize_img.cols,
                                       cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
                }
                norm_img_batch.push_back(resize_img);
            }
            std::vector<float> input(batch_num * cls_image_shape[0] *
                                         cls_image_shape[1] * cls_image_shape[2],
                                     0.0f);
            this->permute_op_.Run(norm_img_batch, input.data());
            auto preprocess_end = std::chrono::steady_clock::now();
            preprocess_diff += preprocess_end - preprocess_start;

            // inference.
            auto inference_start = std::chrono::steady_clock::now();
            
            std::vector<int> blob_shape = { batch_num, 3, 48, 192 };
            Mat blob(blob_shape,CV_32F, input.data());

            std::vector<Mat> outputs;
            predictor_.setInput(blob);
            predictor_.forward(outputs, predictor_.getUnconnectedOutLayersNames());

            auto inference_end = std::chrono::steady_clock::now();
            inference_diff += inference_end - inference_start;

            std::vector<int> predict_shape;
            for (int i = 0; i < outputs[0].dims; ++i)
                predict_shape.push_back(outputs[0].size[i]);
            float *predict_batch = (float *)outputs[0].data;

            // postprocess
            auto postprocess_start = std::chrono::steady_clock::now();
            for (int batch_idx = 0; batch_idx < predict_shape[0]; batch_idx++)
            {
                int label = int(
                    Utility::argmax(&predict_batch[batch_idx * predict_shape[1]],
                                    &predict_batch[(batch_idx + 1) * predict_shape[1]]));
                float score = float(*std::max_element(
                    &predict_batch[batch_idx * predict_shape[1]],
                    &predict_batch[(batch_idx + 1) * predict_shape[1]]));
                cls_labels[beg_img_no + batch_idx] = label;
                cls_scores[beg_img_no + batch_idx] = score;
            }
            auto postprocess_end = std::chrono::steady_clock::now();
            postprocess_diff += postprocess_end - postprocess_start;
        }
        times.push_back(double(preprocess_diff.count() * 1000));
        times.push_back(double(inference_diff.count() * 1000));
        times.push_back(double(postprocess_diff.count() * 1000));
    }

    void Classifier::LoadModel(const std::string &model_dir)
    {
        predictor_ = cv::dnn::readNet(model_dir);
    }
} // namespace PaddleOCR
