#include "lucy_richardson.hh"

void lucy_richardson::core_deconv(Mat m, Mat& res_channel, int tid)
{
  m.convertTo(m, CV_64F);
  normalize(m, m, 0, 1, NORM_MINMAX);
  int winSize = 8 * sigma + 1;
  //generate_psf_gaussian(winSize);
  //generate_psf_defocus(winSize);

  Mat res;
  Mat Y = m.clone();
  Mat J1 = m.clone();
  Mat J2 = m.clone();
  Mat wI = m.clone(); 
  Mat imR = m.clone();
  Mat reBlurred = m.clone();	

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

    Y = J1 + lambda * (J1-J2);
    Y.setTo(0, Y < 0);

    filter2D(Y, reBlurred, -1, psf);
    //GaussianBlur(Y, reBlurred, Size(winSize,winSize), sigma, sigma);
    reBlurred.setTo(epsilon, reBlurred <= 0);

    divide(wI, reBlurred, imR);
    imR = imR + epsilon;

    filter2D(imR, imR, -1, psf);
    //GaussianBlur(imR, imR, Size(winSize,winSize), sigma, sigma);

    J2 = J1.clone();
    multiply(Y, imR, J1);

    T2 = T1.clone();
    T1 = J1 - Y;

    tracks[tid] = j;
  }

  res = J1.clone();
  normalize(res, res, 0, 1, NORM_MINMAX); 

  res_channel = res;
}
