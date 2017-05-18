#include "blur_detect.hh"

void blur_detect::set_image(Mat i)
{
  img = i.clone();

  haar.set_image(i.clone());
}

void blur_detect::build_edge_map()
{
  haar.compute(levels);

  auto haar_pyramid = haar.get_haar_pyramid();

  // Sub-bands mapping
  //  ______________________________
  // |LL3|HL3|HL2    |HL1           |
  // |___|___|       |              |
  // |LH3|HH3|       |              |
  // |___|___|_______|              |
  // |LH2    |HH2    |              |
  // |       |       |              |
  // |       |       |              |
  // |_______|_______|______________|
  // |LH1            |HH1           |
  // |               |              |
  // |               |              |
  // |               |              |
  // |               |              |
  // |               |              |
  // |               |              |
  // |_______________|______________|
  //
  // LH_i => min (0, h / 2^i), max (w / 2^i, h / 2^(i - 1))
  // HL_i => min (w / 2^i, 0), max (w / 2^(i - 1), h / 2^i)
  // HH_i => min (w / 2^i, h / 2^i), max (w / 2^(i - 1), h / 2^(i - 1))
  for (int i = levels; i > 0; i--)
  {
    int sub_band_max_w = haar_pyramid.cols >> i;
    int sub_band_max_h = haar_pyramid.rows >> i;
    edge_map.push_back(Mat(sub_band_max_w, sub_band_max_h, CV_8UC1));

    for (int l = 0; l < sub_band_max_h; l++)
      for (int k = 0; k < sub_band_max_w; k++)
      {
        char lh = haar_pyramid.at<char>(k, sub_band_max_h + l + 1);
        int lh_2 = lh * lh;

        char hl = haar_pyramid.at<char>(sub_band_max_w + k, l + 1);
        int hl_2 = hl * lh;

        char hh = haar_pyramid.at<char>(sub_band_max_w + k + 1, sub_band_max_h + l + 1);
        int hh_2 = hh * hh;

        edge_map[levels - i].at<char>(k, l) = sqrt(lh_2 + hl_2 + hh_2);
      }
  }

  for (int i = 0; i < levels; i++)
    cv::imshow("edge map " + i, edge_map[i]);
}

/*void blur_detect::build_edge_map()
  {
  haar.compute(levels);

  auto haar_pyramid = haar.get_haar_pyramid();
  int w = haar_pyramid.cols;
  int h = haar_pyramid.rows;

  for (int i = 1; i <= levels; i++)
  edge_map.push_back(Mat(w >> i, h >> i, CV_8UC1));

  for (int i = 1; i <= levels; i++)
  for (int l = 0; l < h >> i; l++)
  for (int k = 0; k < w >> i; k++)
  {
  int lh = haar_pyramid.data[(l + (h >> i)) * w + (k >> i)];
  int hl = haar_pyramid.data[(l * w + (k >> i) + (w >> i))];
  int hh = haar_pyramid.data[((l + (h >> i)) * w + k + (w >> i))];

  edge_map[i - 1].data[k + l * (w >> i)] = sqrt(lh * lh + hl * hl + hh * hh);
  }
  }*/

void blur_detect::partition_edge_map()
{
  for (int i = levels; i > 0; i--)
  {
    int sub_band_max_w = edge_map[levels - i].cols;
    int sub_band_max_h = edge_map[levels - i].rows;
    int sub_band_window_size = pow(2, i);
    emax.push_back(Mat(sub_band_max_w, sub_band_max_h, CV_8UC1));
    //std::cout << sub_band_window_size << " " << sub_band_max_w << std::endl;

    for (int l = 0; l < sub_band_max_h; l++)
      for (int k = 0; k < sub_band_max_w; k++)
      {
        char max = 0;
        for (int y = -sub_band_window_size / 2; y < sub_band_window_size / 2; y++)
          for (int x = -sub_band_window_size / 2; x < sub_band_window_size / 2; x++)
          {
            int window_coord_x = k + x;
            int window_coord_y = l + y;

            if (!(window_coord_x < 0 || window_coord_y < 0 || window_coord_x >= sub_band_max_w || window_coord_y >= sub_band_max_h))
              max = util::max(max, edge_map[levels - i].at<char>(window_coord_x, window_coord_y));
          }
        emax[levels - i].at<char>(k, l) = max;
      }
  }

  for (int i = 0; i < levels; i++)
    cv::imshow("emax " + i, emax[i]);
}

float blur_detect::compute_blur_extent()
{
  int n_edge, n_da, n_rg, n_brg;
  n_edge = n_da = n_rg = n_brg = 0;

  auto haar_pyramid = haar.get_haar_pyramid();

  for (int l = 0; l < haar_pyramid.rows; l += 2)
    for (int k = 0; k < haar_pyramid.cols; k += 2)
    {
      char edges[levels];
      for (int i = 0; i < levels; i++)
      {
        int emax_w = emax[i].cols;
        int emax_h = emax[i].rows;

        int x = k >> (i + 1);
        int y = l >> (i + 1);

        if (x >= emax_w)
          x = emax_w - 1;
        if (y >= emax_h)
          y = emax_h - 1;

        edges[i] = emax[i].data[x + y * emax_w];
      }
      // Rule 1
      if (edges[0] > threshold || edges[1] > threshold || edges[2] > threshold)
      {
        n_edge++;

        // Rule 2
        if (edges[0] > edges[1] && edges[1] > edges[2])
          n_da++;
        // Rule 3/4
        else if ((edges[0] < edges[1] && edges[1] < edges[2]) || (edges[1] > edges[0] && edges[1] > edges[2]))
        {
          n_rg++;
          // Rule 5
          if (edges[0] < threshold)
            n_brg++;
        }
      }
    }

  const float epsilon = 5.960465e-8;
  float per = n_da / (n_edge + epsilon);
  float extent = (n_brg + epsilon) / (n_rg + epsilon);
  std::cout << "Per = " << per << std::endl;
  std::cout << "n_da = " << n_da << std::endl;
  std::cout << "n_edge = " << n_edge << std::endl;
  std::cout << "n_da ratio = " << per << std::endl;
  std::cout << "n_brg = " << n_brg << std::endl;
  std::cout << "n_rg = " << n_rg << std::endl;
  std::cout << (per > min_zero || extent > 0.6 ? "sharp" : "blurred") << std::endl;
  if (n_rg != 0)
    return (n_brg + epsilon) / (n_rg + epsilon);
  return 0.0f;
}

float blur_detect::laplacian_of_gauss()
{
  Mat gray, dst, abs_dst;

  GaussianBlur(img, img, Size(3,3), 0, 0, BORDER_DEFAULT);
  gray = util::grayscale(img);
  imshow("resulttt2", gray);

  Laplacian(gray, dst, CV_16S, 3, 1, 0, BORDER_DEFAULT);
  convertScaleAbs(dst, abs_dst);
  imshow("resulttt", abs_dst);

  cv::pow(abs_dst, 2, abs_dst);
  return cv::sum(abs_dst)[0] / (abs_dst.cols * abs_dst.rows);
}

float blur_detect::get_blur_extent()
{
  build_edge_map();
  partition_edge_map();
  std::cout << "LoG = " << laplacian_of_gauss() << std::endl;
  return compute_blur_extent();
}
