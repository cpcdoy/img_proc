#pragma once

# include <opencv2/opencv.hpp>
# include "util.hh"

using namespace cv;

class haar_wavelet
{
  public:
    void compute(int levels);

    void haar(int w, int h);

    void set_image(Mat img);
    Mat get_haar_pyramid();

  private:
    Mat img, tmp;
};
