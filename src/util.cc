#include "util.hh"

Mat grayscale(Mat img)
{
  Mat gs;
  cv::cvtColor(img, gs, CV_BGR2GRAY);

  return gs;
}
