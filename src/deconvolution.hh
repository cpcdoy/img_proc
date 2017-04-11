#pragma once

# include <opencv2/opencv.hpp>

# include "util.hh"

using namespace cv;

class deconvolution
{
  public:
    virtual void set_image(Mat img);
    virtual void set_channels(int c);
    virtual void set_params(Mat psf, double ang_deg = 0.0, int d = 65, int snr = 25, float sigma = 3.0, int num_iterations = 100);
    virtual void set_iterations(int it = 100);
    virtual Mat apply_deconvolution();

  protected:
    Mat generate_psf_gaussian(int radius);
    Mat generate_psf_defocus(int radius, int center = 65);
    Mat img;
    Mat psf;
    
    int chans = 1;
    double angle_rad;
    int spread;
    int snr;
    float sigma;
    int num_iterations;

    double epsilon = 2.2204e-16;
};
