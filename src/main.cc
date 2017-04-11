#include <iostream>
#include <opencv2/opencv.hpp>
#include "haar_wavelet.hh"
#include "blur_detect.hh"
#include "deconvolution.hh"
#include "lucy_richardson.hh"


static void on_mouse_click(int event, int x, int y, int, void* imgptr)
{
  if (event == EVENT_LBUTTONDOWN)
  {
    Mat & img = (*(Mat*)imgptr);
    Point pt1 = Point(x + 30, y + 30);
    circle(img, pt1, 1, 255, -1, CV_AA, 1);
    imshow("PSF Generation", img);
  }
}

int main(int argc, char** argv)
{
  if (argc == 1)
  {
    std::cout << "--- Blur detection/deconvolution ---\n\nUsage: img_proc img_path"<< std::endl;
    return 0;
  }

  try
  {
    cv::Mat img = cv::imread(argv[1]);

    /*haar_wavelet haar;
      haar.set_image(img.clone());

      haar.compute(3);

      cv::imshow("Orig", img);
    //cv::imshow("Haar", haar.get_haar_pyramid());

    blur_detect blur;
    blur.set_image(img.clone());
    std::cout << "Blur extent : " << blur.get_blur_extent() << std::endl;*/

    deconvolution deconv;
    deconv.set_image(img);
    deconv.set_params(cv::imread("psf.jpg"), 25, 25, 25);

    lucy_richardson lr;
    lr.set_image(img);
    lr.set_channels(3);

    /*namedWindow("PSF Generation", 0);
    Mat psf = Mat(65, 65, CV_64F, double(0));
    setMouseCallback("PSF Generation", on_mouse_click, &psf);
    imshow("PSF Generation", psf);
    waitKey(0);*/

    lr.set_params(Mat(), 25, 25, 25);
    lr.set_iterations(100);
    cv::imshow("Original", img);
    cv::imshow("lr res", lr.apply_deconvolution());

    auto res = deconv.apply_deconvolution();
    //cv::imshow("Deconv", res);

    cv::waitKey(0);
  }
  catch(cv::Exception& e)
  {
    std::cerr << "The image format doesn't seem to be correct. Try using JPEG, PNG or any known format." << std::endl;
    std::cerr << "\nError details: " << e.what() << std::endl;
  }

  return 0;
}
