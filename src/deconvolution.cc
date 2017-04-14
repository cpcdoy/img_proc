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
  medianBlur(this->img, this->img, 3);
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
  //std::cout << "Kern = \n" << k << std::endl;
  this->psf = k;
  return k;
}

Mat deconvolution::apply_deconvolution()
{
  tracker = std::thread([&]()
  {
      while (!finished)
      {
        std::cout << "Applying " << name << " deconvolution...\n";
        std::cout << "Spawned " << chans << " threads:\n";
        for (int i = 0; i < chans; i++)
          std::cout << "-- Thread " << i << ": " << tracks[i] << " / " << num_iterations << "\n";
        std::cout << std::endl << "\x1B[2J\x1B[H";
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
      }
  });

  Mat res;
  imshow("PSF", psf);
  if (chans > 1)
  {
    Mat channels[chans];
    split(img, channels);

    Mat results[chans];
    std::thread tids[chans];
    for (int i = 0; i < chans; i++)
      tids[i] = std::thread(&deconvolution::core_deconv, this, channels[i], std::ref(results[i]), i);

    for (int i = 0; i < chans; i++)
    {
      tids[i].join();
      std::cout << "Thread " << i << " done." << std::endl;
    }
    finished = true;
    tracker.join();

    merge(results, chans, res);
  }
  else if (chans == 1)
  {
    img = util::grayscale(img);
    core_deconv(img, res, 0);
  }

  return res;
}

void deconvolution::core_deconv(Mat m, Mat& res_channel, int tid)
{
  tracks[tid] = 0;
  m.convertTo(m, CV_64F);
  normalize(m, m, 0, 1, NORM_MINMAX);
  //generate_psf_defocus(18);

  psf.convertTo(psf, CV_64F);
  normalize(psf, psf, 0, 1, NORM_MINMAX);
  psf /= sum(psf)[0];

  Mat f = m.clone();
  Mat psf_pad = Mat(m.rows, m.cols, CV_64F, double(0.0));

  Range colRange = Range(0, min(psf.cols, psf_pad.cols));
  Range rowRange = Range(0, min(psf.rows, psf_pad.rows));
  psf(rowRange, colRange).copyTo(psf_pad(rowRange, colRange));

  Mat F;
  dft(m, F, DFT_COMPLEX_OUTPUT);

  Mat PSF_pad;
  dft(psf_pad, PSF_pad, DFT_COMPLEX_OUTPUT, psf.rows);
  /*split(PSF_pad, planes);
    imshow("psf pad", planes[0]);
    imshow("psf pad2", planes[1]);

    Mat PSF_pad_abs_2;
    PSF_pad = abs(PSF_pad);
    pow(PSF_pad, 2, PSF_pad_abs_2);
    imshow("psf pad abs 2", PSF_pad_abs_2);

    int SNR = 25;

    Mat iPSF = PSF_pad / (PSF_pad_abs_2 + SNR);
    Mat RES;
    mulSpectrums(F, iPSF, RES, DFT_COMPLEX_OUTPUT);*/

  int width = m.cols;
  int height= m.rows;

  Mat_<Vec2f> src = PSF_pad;
  Mat_<Vec2f> dst = F;

  snr = 10000;
  float eps = 0.0001f;
  float power, factor, tmp;
  float inv_snr = 1.f / (snr + 0.00001f);
  Mat PSF_pad_2;
  pow(src, 2, PSF_pad_2);

  Mat planes[2];
  split(PSF_pad_2, planes);
  PSF_pad_2 = planes[0] + planes[1];

  tracks[tid] = 50;
  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++)
    {
      power = PSF_pad_2.at<float>(y, x);
      factor = (1.f / power)*(1.f-inv_snr/(power*power + inv_snr));

      tmp = dst(y,x)[0];
      dst(y,x)[0] = (src(y,x)[0] * tmp + src(y,x)[1] * dst(y,x)[1]) * factor;
      dst(y,x)[1] = (src(y,x)[0] * dst(y,x)[1] - src(y,x)[1] * tmp) * factor; 
    }
  }

  Mat res;
  idft(dst, res, DFT_SCALE | DFT_REAL_OUTPUT);
  res = abs(res);
  normalize(res, res, 0, 1, NORM_MINMAX); 

  tracks[tid] = 100;
  res_channel = res;
}
