#pragma once

# include "deconvolution.hh"

class lucy_richardson : public deconvolution
{
  private:
    virtual void core_deconv_per_channel(Mat m, Mat& res_channel, int tid) override;
};
