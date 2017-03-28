#include <iostream>
#include <opencv2/opencv.hpp>
#include "haar_wavelet.hh"
#include "blur_detect.hh"

int main(int argc, char** argv)
{
  cv::Mat img = cv::imread(argv[1]);

  haar_wavelet haar;
  haar.set_image(img.clone());

  haar.compute(3);

  cv::imshow("Orig", img);
  cv::imshow("Haar", haar.get_haar_pyramid());

  blur_detect blur;
  blur.set_image(img.clone());
  std::cout << "Blur extent : " << blur.get_blur_extent() * 100 << "%" << std::endl;

  cv::waitKey(0);

  return 0;
}
