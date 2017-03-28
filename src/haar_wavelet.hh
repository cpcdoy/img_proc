#pragma once

# include <opencv2/opencv.hpp>
# include "util.hh"

using namespace cv;

class haar_wavelet
{
  public:
    enum sub_band
    {
      LL,
      LH,
      HL,
      HH,
      ALL
    };

    void compute(int levels);

    void set_norm_factor(float n);
    void set_image(Mat img);
    Mat get_haar_pyramid();

  private:
    void haar(int w, int h);

    Mat img, tmp;

    float norm = 2;

    std::vector<Mat> edge_map;
    int levels;
};
