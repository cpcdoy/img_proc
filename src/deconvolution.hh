#pragma once

# include <opencv2/opencv.hpp>

# include <thread>

# include "util.hh"
# include "ui_utils.hh"

using namespace cv;

class deconvolution
{
  public:
    virtual void set_image(Mat img);
    virtual void set_channels(int c);
    virtual void set_params(Mat psf, double ang_deg = 0.0, int spread = 65, int snr = 25, float sigma = 3.0, int num_iterations = 100);
    virtual void set_iterations(int it = 100);
    virtual Mat apply_deconvolution();
    Mat generate_psf_gaussian(int radius);
    Mat generate_psf_defocus(int radius, int center = 65);

  protected:
    virtual void core_deconv(Mat m, Mat& res_channel, int tid);

    std::string name = "deconv";

    Mat img;
    Mat psf;
    
    int chans = 1;
    double angle_rad;
    int spread;
    int snr;
    float sigma;
    int num_iterations;

    double epsilon = 2.2204e-16;

    bool finished = false;
    int tracks[4]; // 4 = max channels of an image (RGBA)

    std::thread tracker;
};
