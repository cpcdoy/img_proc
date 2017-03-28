#include "haar_wavelet.hh"

#include <cmath>
#include <iostream>

void haar_wavelet::compute(int levels)
{
  this->levels = levels;
  tmp.release();
  img = util::grayscale(img);

  int w = 1;
  int h = 1;

  for (; w < img.cols; w *= 2);
  for (; h < img.rows; h *= 2);

  w /= 2;
  h /= 2;

  Rect r(0, 0, w, h);

  img = img(r);

  for (int i = 0; i < levels; i++)
    haar(w / pow(2, i), h / pow(2, i));
}

void haar_wavelet::haar(int w, int h)
{
  int w_2 = w / 2;
  int h_2 = h / 2;

  tmp = img.clone();

  for (int i = 0; i < h; i++)
    for (int j = 0; j < w_2; j++)
    {
      auto a = tmp.at<char>(j * 2 + 1, i);
      auto b = tmp.at<char>(j * 2, i);

      img.at<char>(j + w_2, i) = (a - b) / norm;
      img.at<char>(j, i) = (a + b) / norm;
    }

  tmp = img.clone();

  for (int i = 0; i < h_2; i++)
    for (int j = 0; j < w; j++)
    {
      auto a = tmp.at<char>(j, i * 2 + 1);
      auto b = tmp.at<char>(j, i * 2);

      img.at<char>(j, h_2 + i) = (a - b) / norm;
      img.at<char>(j, i) = (a + b) / norm;
    }
}

void haar_wavelet::set_image(Mat img)
{
  this->img = img;
}

void haar_wavelet::set_norm_factor(float n)
{
  norm = n;
}

Mat haar_wavelet::get_haar_pyramid()
{
  return img;
}
