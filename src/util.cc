#include "util.hh"

namespace util
{
  Mat grayscale(Mat img)
  {
    Mat gs;
    cv::cvtColor(img, gs, CV_BGR2GRAY);

    return gs;
  }

  bool is_number(std::string x)
  {
    std::regex e ("^-?\\d+");
    return std::regex_match(x,e);
  }
}
