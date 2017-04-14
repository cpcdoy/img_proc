#include <iostream>
#include <opencv2/opencv.hpp>
#include "haar_wavelet.hh"
#include "blur_detect.hh"
#include "deconvolution.hh"
#include "lucy_richardson.hh"
#include "ui_utils.hh"

int main(int argc, char** argv)
{
  cmd_params params = parse_cmd_params(argc, argv);
  if (params.error || !params.path_given || !(params.lr || params.wiener || params.detect))
  {
    std::cout << "--- Blur detection/deconvolution ---\n\nUsage: img_proc --path img_path [--circle center radius] [--draw] [--apply] [--it] [--channels] [--psf]"<< std::endl;
    return 1;
  }

  try
  {
    cv::Mat img = cv::imread(params.path);

    if (params.detect)
    {
      haar_wavelet haar;
      haar.set_image(img.clone());

      haar.compute(3);

      cv::imshow("Orig", img);
      //cv::imshow("Haar", haar.get_haar_pyramid());

      blur_detect blur;
      blur.set_image(img.clone());
      std::cout << "Blur extent : " << blur.get_blur_extent() << std::endl;
    }

    cv::imshow("Original", img);
    Mat psf;
    if (params.lr || params.wiener)
    {
      if (params.psf_given)
        psf = cv::imread(params.psf_path, CV_LOAD_IMAGE_GRAYSCALE);
      else if (params.draw)
        psf = create_drawing_window(65, 65, "PSF Generation");
    }

    if (params.wiener)
    {
      deconvolution deconv;
      deconv.set_image(img);
      deconv.set_channels(params.channels);
      deconv.set_params(psf);
      auto res = deconv.apply_deconvolution();
      imshow("Res Wiener", res);
    }

    if (params.lr)
    {
      lucy_richardson lr;
      lr.set_image(img);
      lr.set_channels(params.channels);
      lr.set_params(psf);
      lr.set_iterations(params.iterations);
      auto res = lr.apply_deconvolution();
      cv::imshow("Res LR", res);
    }

    cv::waitKey(0);
  }
  catch(cv::Exception& e)
  {
  }

  return 0;
}
