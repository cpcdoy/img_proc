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
    virtual void set_params(Mat psf, double ang_deg = 0.0, int spread = 65, int snr = 25, float sigma = 3.0, int num_iterations = 50, int blind_it = 0);
    virtual void set_iterations(int it = 100);
    virtual void set_blind_iterations(int it = 5, int prec = 5);
    virtual void set_snr(int snr = 10000);
    virtual Mat apply_deconvolution();
    virtual void set_window_size(int window);
    Mat generate_psf_gaussian(int radius);
    Mat generate_psf_defocus(int radius, int center = 65);

  protected:
    virtual void core_deconv_per_channel(Mat m, Mat& res_channel, int tid);
    virtual void core_deconv(Mat m, Mat& res_channel, int tid);

    std::string name = "deconv";

    Mat img;
    Mat psf[3];

    int chans = 1;
    double angle_rad;
    int spread;
    int snr;
    float sigma;
    int num_iterations;
    int blind_it = 0;
    int blind_prec = 0; // Kernel estimation precision (in terms of iteration with the acc LR method)
    int window_size = 65;

    double epsilon = 2.2204e-16;

    bool finished = false;
    int tracks[3]; // 3 = max channels of an image without an alpha channel (RGB)

    std::thread tracker;
};
