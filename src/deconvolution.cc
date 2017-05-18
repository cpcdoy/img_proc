#include "deconvolution.hh"

void deconvolution::set_params(Mat psf, double ang_deg, int spread, int snr, float sigma, int num_iterations, int blind_it)
{
  this->psf[0] = psf;
  this->psf[1] = psf;
  this->psf[2] = psf;
  this->angle_rad = ang_deg / (180 / CV_PI);
  this->snr = pow(10, -0.1 * snr);
  this->spread = spread;
  this->sigma = sigma;
  this->num_iterations = num_iterations;
  this->blind_it = blind_it;
}

void deconvolution::set_snr(int snr)
{
  this->snr = snr;
}

void deconvolution::set_window_size(int window)
{
  this->window_size = window;
}

void deconvolution::set_image(Mat img)
{
  this->img = img;
  medianBlur(this->img, this->img, 3);
}

void deconvolution::set_blind_iterations(int it, int prec)
{
  this->blind_it = it;
  this->blind_prec = prec;
}

void deconvolution::set_channels(int c)
{
  int img_chans_max = img.channels();
  chans = c > 1 ? img_chans_max : 1;
}

void deconvolution::set_iterations(int it)
{
  this->num_iterations = it;
}

Mat deconvolution::generate_psf_defocus(int radius, int center)
{
  Mat psf = Mat(center, center, CV_64F, double(0));
  circle(psf, Point(center, center), radius, 255, -1, CV_AA, 1);

  this->psf[0] = psf;
  this->psf[1] = psf;
  this->psf[2] = psf;
  return psf;
}

Mat deconvolution::generate_psf_gaussian(int radius)
{
  auto kern = getGaussianKernel(radius, sigma);
  Mat k = kern * kern.t();
  flip(k, k, -1);
  //std::cout << "Kern = \n" << k << std::endl;
  this->psf[0] = k;
  this->psf[1] = k;
  this->psf[2] = k;
  return k;
}

Mat deconvolution::apply_deconvolution()
{
  Mat res;
  if (chans > 1)
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

    Mat channels[chans];
    split(img, channels);

    Mat results[chans];
    std::thread tids[chans];
    if (blind_prec)
      for (int i = 0; i < chans; i++)
        core_deconv(channels[i], results[i], i);
    else
    {
      for (int i = 0; i < chans; i++)
        tids[i] = std::thread(&deconvolution::core_deconv, this, channels[i], std::ref(results[i]), i);

      for (int i = 0; i < chans; i++)
      {
        tids[i].join();
        std::cout << "Thread " << i << " done." << std::endl;
      }
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

void deconvolution::core_deconv(Mat m, Mat& res, int tid)
{
  if (blind_prec && tid == 0)
  {
    Mat tmp_img;
    if (chans > 1)
      tmp_img = util::grayscale(img);
    else
      tmp_img = m.clone();
    Mat final_psf = psf[tid].clone();
    int tmp_it = num_iterations;
    num_iterations = blind_prec;
    for (int i = 0; i < blind_it; i++)
    {
      core_deconv_per_channel(tmp_img, res, 0); // PSF estimation

      int sub_win_w_1 = res.cols / 2 - window_size / 2;
      int sub_win_h_1 = res.rows / 2 - window_size / 2;
      auto t = res(Rect(sub_win_w_1, sub_win_h_1, window_size, window_size));
      psf[tid] = t.clone();

      imshow("psf " + std::to_string(i), psf[tid]);
      final_psf = psf[tid].clone();
      core_deconv_per_channel(tmp_img, res, tid); // Deconv with estimated PSF
      psf[tid] = res.clone(); // Reset PSF
      tmp_img = res.clone();
    }
    for (int i = 0; i < chans; i++)
      psf[i] = final_psf.clone();
    num_iterations = tmp_it;
  }
  core_deconv_per_channel(m, res, tid);
}

void deconvolution::core_deconv_per_channel(Mat m, Mat& res_channel, int tid)
{
  tracks[tid] = 0;
  m.convertTo(m, CV_64F);
  normalize(m, m, 0, 1, NORM_MINMAX);
  //generate_psf_defocus(18);

  psf[tid].convertTo(psf[tid], CV_64F);
  normalize(psf[tid], psf[tid], 0, 1, NORM_MINMAX);
  psf[tid] /= sum(psf[tid])[0];

  Mat f = m.clone();
  Mat psf_pad = Mat(m.rows, m.cols, CV_64F, double(0.0));

  Range colRange = Range(0, min(psf[tid].cols, psf_pad.cols));
  Range rowRange = Range(0, min(psf[tid].rows, psf_pad.rows));
  psf[tid](rowRange, colRange).copyTo(psf_pad(rowRange, colRange));

  Mat F;
  dft(m, F, DFT_COMPLEX_OUTPUT);

  Mat PSF_pad;
  dft(psf_pad, PSF_pad, DFT_COMPLEX_OUTPUT, psf[tid].rows);
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

  //snr = 1000;
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
