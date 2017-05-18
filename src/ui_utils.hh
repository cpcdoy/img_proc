#pragma once

# include <string>
# include <cctype>

# include "util.hh"

# include <opencv2/opencv.hpp>

using namespace cv;

Mat create_drawing_window(int w, int h, std::string str);

struct cmd_params
{
  bool draw = false;
  bool cirle = false;
  Point circle;
  bool line = false;
  Point line_beg, line_end;

  std::string path;
  bool path_given = false;

  int channels = 1;

  int iterations = 100;

  std::string psf_path;
  bool psf_given = false;

  std::vector<std::string> apply;
  bool wiener = false;
  bool lr = false;
  bool detect = false;

  bool defocus = false;
  int defocus_radius = 0;

  bool gaussian = false;
  float gaussian_sigma = 0.0f;

  bool blind = false;
  int blind_it = 0;
  int blind_prec = 0;

  int window_size = 65;
  bool window_size_given = false;

  int snr = 1000;
  bool snr_given = false;

  bool error = false;
};

struct cmd_params parse_cmd_params(int argc, char** argv);
