#pragma once

# include "deconvolution.hh"

class lucy_richardson : public deconvolution
{
  public:
    virtual Mat apply_deconvolution() override;
  private:
    Mat core_deconv(Mat m);
};
