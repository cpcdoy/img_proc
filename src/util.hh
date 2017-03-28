#pragma once

# include <opencv2/opencv.hpp>

using namespace cv;

namespace util
{
  Mat grayscale(Mat img);

  template<typename T>
    T max(T a, T b);


  template<typename T>
    T max(T a, T b)
    {
      return a > b ? a : b;
    }
}
