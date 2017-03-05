#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "haar_wavelet.hh"

int main(int argc, char** argv)
{
  cv::Mat img = cv::imread(argv[1]);

  haar_wavelet haar;
  haar.set_image(img.clone());

  haar.compute(3);

  cv::imshow("Orig", img);
  cv::imshow("Haar", haar.get_haar_pyramid());

  cv::waitKey(0);

  return 0;
}
