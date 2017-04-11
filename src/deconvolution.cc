#include "deconvolution.hh"

void deconvolution::set_params(Mat psf, double ang_deg, int spread, int snr, float sigma, int num_iterations)
{
  this->psf = psf;
  this->angle_rad = ang_deg / (180 / CV_PI);
  this->snr = pow(10, -0.1 * snr);
  this->spread = spread;
  this->sigma = sigma;
  this->num_iterations = num_iterations;
}

void deconvolution::set_image(Mat img)
{
  this->img = img;
}

void deconvolution::set_channels(int c)
{
  int img_chans_max = img.channels();
  chans = c > img_chans_max ? img_chans_max : c;
}

void deconvolution::set_iterations(int it)
{
  this->num_iterations = it;
}

Mat deconvolution::generate_psf_defocus(int radius, int center)
{
  Mat psf = Mat(center, center, CV_64F, double(0));
  circle(psf, Point(center, center), radius, 255, -1, CV_AA, 1);

  this->psf = psf;
  return psf;
}

Mat deconvolution::generate_psf_gaussian(int radius)
{
  auto kern = getGaussianKernel(radius, sigma);
  Mat k = kern * kern.t();
  flip(k, k, -1);
  std::cout << "Kern = \n" << k << std::endl;
  this->psf = k;
  return k;
}

Mat deconvolution::apply_deconvolution()
{
  img = util::grayscale(img);
  img.convertTo(img, CV_32FC1, 1 / 255.0);

  Mat IMG_complex = img.clone();

  dft(img, IMG_complex, DFT_COMPLEX_OUTPUT);

  psf /= sum(psf)[0];
  Mat psf_pad =  Mat(img.rows, img.cols, CV_64F, double(0));

  int w = psf.cols;
  int h = psf.rows;

  Mat PSF;
  dft(psf_pad, PSF, DFT_COMPLEX_OUTPUT, h);
  Mat PSF_2;
  //PSF_2 = sum(pow(PSF, 2, PSF_2));

  //imshow("psf", generate_psf_gaussian(spread));

  return img;
}
