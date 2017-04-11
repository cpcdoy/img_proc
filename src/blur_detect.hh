#pragma once 

# include "haar_wavelet.hh"
# include "util.hh"

# include <vector>

class blur_detect
{
  public:
    void set_image(Mat i);
    float get_blur_extent();

  private:
    void build_edge_map();
    void partition_edge_map();
    float compute_blur_extent();

    std::vector<Mat> edge_map;
    std::vector<Mat> emax;
    haar_wavelet haar;

    int levels = 3;
    int threshold = 35;
    // Min_zero very close to zero
    float min_zero = 0.2;
};
