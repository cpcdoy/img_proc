#include "ui_utils.hh"

static void on_mouse_click(int event, int x, int y, int, void* imgptr)
{
  if (event == EVENT_LBUTTONDOWN)
  {
    Mat & img = (*(Mat*)imgptr);
    Point pt1 = Point(x + 65 / 2, y + 65 / 2);
    circle(img, pt1, 1, 255, -1, CV_AA, 1);
    imshow("PSF Generation", img);
  }
}

Mat create_drawing_window(int w, int h, std::string str)
{
  namedWindow(str.c_str(), 0);
  Mat psf = Mat(w, h, CV_64F, double(0));
  setMouseCallback(str.c_str(), on_mouse_click, &psf);
  imshow(str.c_str(), psf);
  waitKey(0);

  return psf;
}

struct cmd_params parse_cmd_params(int argc, char** argv)
{
  struct cmd_params params;
  for (int i = 1; i < argc; i++)
  {
    std::string str(argv[i]);
    if (str == "--draw")
      params.draw = true;
    else if (str == "--circle")
    {
      bool err = false;
      i++;
      if (i < argc && util::is_number(argv[i]))
        params.circle.x = std::stoi(argv[i]);
      else
        err = true;
      i++;
      if (i < argc && util::is_number(argv[i]))
        params.circle.y = std::stoi(argv[i]);
      else
        err = true;
      if (err)
      {
        params.error = true;
        std::cerr << "--circle command needs the following arguments:\nEx: --cirle 65 25\nwhere: -the 1st arg is the center (only on number because center = (x, x))\n       -the 2nd arg is the radius" << std::endl;
        break;
      }
    }
    else if (str == "--path")
    {
      i++;
      if (i < argc)
      {
        params.path = std::string(argv[i]);
        params.path_given = true;
      }
      else
      {
        params.error = true;
        std::cerr << "--path needs one argument" << std::endl;
        break;
      }
    }
    else if (str == "--window")
    {
      i++;
      if (i < argc && util::is_number(argv[i]))
      {
        params.window_size_given = true;
        params.window_size = std::stoi(argv[i]);
      }
      else
      {
        params.error = true;
        std::cerr << "window needs one argument" << std::endl;
        break;
      }
    }
    else if (str == "--psf")
    {
      i++;
      if (i < argc)
      {
        auto arg = std::string(argv[i]);
        if (arg == "defocus")
        {
          i++;
          if (i < argc && util::is_number(argv[i]))
          {
            params.defocus = true;
            params.defocus_radius = std::stoi(argv[i]);
          }
          else
          {
            params.error = true;
            std::cerr << "defocus needs one argument" << std::endl;
            break;
          }
        }
        else if (arg == "gaussian")
        {
          i++;
          if (i < argc && util::is_number(argv[i]))
          {
            params.gaussian = true;
            params.gaussian_sigma = std::stoi(argv[i]);
          }
          else
          {
            params.error = true;
            std::cerr << "gaussian needs one argument" << std::endl;
            break;
          }
        }
        else if (arg == "blind")
        {
          i++;
          if (i < argc && util::is_number(argv[i]))
          {
            params.blind_it = std::stoi(argv[i]);
            i++;
            if (i < argc && util::is_number(argv[i]))
            {
              params.blind = true;
              params.blind_prec = std::stoi(argv[i]);
            }
            else
            {
              params.error = true;
              std::cerr << "blind needs a second argument" << std::endl;
              break;
            }
          }
          else
          {
            params.error = true;
            std::cerr << "blind needs one argument" << std::endl;
            break;
          }
        }
        else
        {
          params.psf_path = std::string(argv[i]);
          params.psf_given = true;
        }
      }
      else
      {
        params.error = true;
        std::cerr << "--path needs one argument" << std::endl;
        break;
      }
    }
    else if (str == "--channels")
    {
      i++;
      if (i < argc && util::is_number(argv[i]))
        params.channels = std::stoi(argv[i]);
      else
        std::cerr << "--channels needs an integer" << std::endl;
    }
    else if (str == "--snr")
    {
      i++;
      if (i < argc && util::is_number(argv[i]))
      {
        params.snr = std::stoi(argv[i]);
        params.snr_given = true;
      }
      else
        std::cerr << "--snr needs an integer" << std::endl;
    }
    else if (str == "--it")
    {
      i++;
      if (i < argc && util::is_number(argv[i]))
        params.iterations = std::stoi(argv[i]);
      else
        std::cerr << "--it needs an integer" << std::endl;
    }
    else if (str == "--apply")
    {
      i++;
      if (i < argc)
      {
        params.apply = util::split(argv[i], ',');
        for (auto i = params.apply.begin(); i != params.apply.end(); i++)
          if ((*i) == "wiener")
            params.wiener = true;
          else if ((*i) == "lr")
            params.lr = true;
          else if ((*i) == "detect")
            params.detect = true;
      }
      else
        std::cerr << "--deconv needs a list" << std::endl;
    }
    else
    {
      params.error = true;
      std::cerr << "Unknown command " << str << std::endl;
      break;
    }
  }

  return params;
}
