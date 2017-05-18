#include "lucy_richardson.hh"

void lucy_richardson::core_deconv_per_channel(Mat m, Mat& res_channel, int tid)
{
  m.convertTo(m, CV_64F);
  normalize(m, m, 0, 1, NORM_MINMAX);
  /*sigma = 5.0;
  int winSize = 8 * sigma + 1;
  generate_psf_gaussian(winSize);
  generate_psf_defocus(winSize);*/

  Mat res;
  Mat f_k_lambda = m.clone();
  Mat psi_f_k = m.clone();
  Mat f_k = m.clone();
  Mat g = m.clone(); 
  Mat blurred_on_reblurred = m.clone();
  Mat reblurred = m.clone();

  Mat T1, T2, tmpMat1, tmpMat2;
  T1 = Mat(m.rows,m.cols, CV_64F, 0.0);
  T2 = Mat(m.rows,m.cols, CV_64F, 0.0);

  double lambda = 0;
  for(int j = 0; j < num_iterations; j++)
  {
    if (j>1)
    {
      multiply(T1, T2, tmpMat1);
      multiply(T2, T2, tmpMat2);
      lambda=sum(tmpMat1)[0] / (sum(tmpMat2)[0]+epsilon);
    }

    f_k_lambda = psi_f_k + lambda * (psi_f_k-f_k);
    f_k_lambda.setTo(0, f_k_lambda < 0);

    filter2D(f_k_lambda, reblurred, -1, psf[tid]);
    reblurred.setTo(epsilon, reblurred <= 0);

    divide(g, reblurred, blurred_on_reblurred);
    blurred_on_reblurred = blurred_on_reblurred + epsilon;

    filter2D(blurred_on_reblurred, blurred_on_reblurred, -1, psf[tid]);

    pow(blurred_on_reblurred, 2, blurred_on_reblurred);

    f_k = psi_f_k.clone();
    multiply(f_k_lambda, blurred_on_reblurred, psi_f_k);

    T2 = T1.clone();
    T1 = psi_f_k - f_k_lambda;

    tracks[tid] = j;
  }

  res = psi_f_k.clone();
  normalize(res, res, 0, 1, NORM_MINMAX);

  res_channel = res;
}
