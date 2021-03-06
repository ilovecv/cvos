//----------------------------------------------------------------------------
// Inputs:
// @param: A: uvf
// @param: B: uvb 
// @param: m_change: pixel-wise weights of how much to change 
//  (currently: 1 - residual)
// @param: residual: residual image / occlusion term (discounts flow from 
//  occluded pixels to be counted into local window)
// @param: W: window size (determines sigma_d) - cuz I need constant size 
//  sliding window I think for histogram version (max 50, but % determined 
//  on image size)
// @param: sigma_r: think I used to determine this based on range of flow 
//  within a window 
//----------------------------------------------------------------------------

#include <math.h>
#include <matrix.h>
#include <mex.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <vector>
#include "cvos_common.h"
using namespace std;
// Aout = bflt_mex(A, B, mask_change_, mask_ignore_, w_, sigma_d, sigma_r)

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
  // ----------------------------------------------------------------------    
  // double MAXG = 100.0;
  // double MINW = 20.0;
  // // double opts_w = 2.0;
  // double opts_TOOMUCHMASKTHRESH = 0.95;
  // values for lookup table for exponential.
  int EXP_NUM_ = 2000;
  double EXP_MAX_ = 5.0f;
  // ----------------------------------------------------------------------  
  // declare variables
  double* uvf_;
  
  double* uvf = (double*)mxGetPr( prhs[0] );
  double* uvb = (double*)mxGetPr( prhs[1] );
  double* m_change = (double*)mxGetPr( prhs[2] );
  double* resf = (double*)mxGetPr( prhs[3] );
  // double* resb = (double*)mxGetPr( prhs[4] );
   
  double w_ = (double)mxGetScalar( prhs[5] );
  // double sigma_uvf = (double)mxGetScalar( prhs[6] );
  double sigma_uvb = (double)mxGetScalar( prhs[7] );
  double sigma_loc = (double)mxGetScalar( prhs[8] );

  mwSize* sz = (mwSize*)mxGetDimensions( prhs[0] );
  int ndims = mxGetNumberOfDimensions( prhs[0] );
  if (ndims < 2) {
    mexPrintf("First argument size is unexpected. Quitting.\n");
    plhs[0] = mxCreateDoubleMatrix(1, 1, mxREAL);
    uvf_ = (double*)mxGetPr(plhs[0]);
    return;
  }
  int rows = sz[0];
  int cols = sz[1];
  int chan = sz[2];

  // T: just assume 2 channel matrix is passed for now.
  plhs[0] = mxCreateNumericArray(ndims, sz, mxDOUBLE_CLASS, mxREAL);
  uvf_ = (double*)mxGetPr(plhs[0]); 
  
  // double sigma_uvf_sq_inv = 1.0/(2.0*sigma_uvf*sigma_uvf);
  double sigma_uvb_sq_inv = 1.0/(2.0*sigma_uvb*sigma_uvb);
  // double sigma_loc_sq_inv = 1.0/(2.0*sigma_loc*sigma_loc);
  // double sigma_img_sq_inv = 1.0/(2.0*sigma_img*sigma_img);
  
  memcpy(uvf_, uvf, sizeof(double)*rows*cols*chan);
  
  // create an exponential function lookup table
  double* exp_ = new double[EXP_NUM_];
  for (int i = 0; i < EXP_NUM_; ++i) { 
    double val = double(i)/double(EXP_NUM_)*EXP_MAX_;
    exp_[i] = exp(-val);
  }

  int w = w_;
  double sigma_loc_square_2 = 2.0*sigma_loc*sigma_loc;
  int ww = min((int) ceil(sqrt(EXP_MAX_*sigma_loc_square_2)), int(w));
  double gstep = double(w) / double(ww);

  double sigma_loc_sq_inv = 1.0/double(sigma_loc_square_2);
      
          
  int idx, idx1, idx2, widx, widx1, widx2;
  for (int x = 0; x < cols; ++x) { 
    for (int y = 0; y < rows; ++y) {
      idx = linear_index(y, x, rows);
      idx1 = linear_index(y, x, 0, rows, cols);
      idx2 = linear_index(y, x, 1, rows, cols);

      double m_o = m_change[ idx ];
      // double br_o = resb[ idx ];
      double fr_o = resf[ idx ];
      double bu_o = uvb[ idx1 ];
      double bv_o = uvb[ idx2 ];
      // double fu_o = uvf[ idx1 ];
      // double fv_o = uvf[ idx2 ];
      // double r0 = residual[ idx ];

      // if ((m_o > 0.1f) && ((fr_o - br_o) > 0.1f)) {
      if ((m_o > 0.1f)) {
        // // get local window bounds
        // int y_min = max(y-w, 0);
        // int y_max = min(y+w, rows-1);
        // int x_min = max(x-w, 0);
        // int x_max = min(x+w, cols-1);

        // get local window bounds
        // int ww = min(max(int(2*floor(sqrt(bx_o*bx_o + by_o*by_o))), 10), w);
        // double sigma_loc = 2.0f;
        // double sigma_loc = min(max(2.0 * sqrt(fu_o*fu_o + fv_o*fv_o), 2.0), double(w));

        // mexPrintf("ww: %d   sigma_d: %0.4f\n", ww, sigma_d);

        int y_min = max(y-ww, 0);
        int y_max = min(y+ww, rows-1);
        int x_min = max(x-ww, 0);
        int x_max = min(x+ww, cols-1);

        // double energy_uvf_local = 0.0f;
        // double energy_uvb_local = 0.0f;
        // double var_uvf_local = 0.0f;
        // double var_uvb_local = 0.0f;

        double val_x = 0.0;
        double val_y = 0.0;
        double filter_norm = 1e-6;
        for (int cc=x_min; cc <= x_max; ++cc) { 
          for (int rr=y_min; rr <= y_max; ++rr) { 
            widx = linear_index(rr, cc, rows);
            widx1 = linear_index(rr, cc, 0, rows, cols);
            widx2 = linear_index(rr, cc, 1, rows, cols);

            // double m = m_change[ widx ];
            // double br = resb[ widx ];
            double fr = resf[ widx ];
            double bu = uvb [ widx1 ];
            double bv = uvb [ widx2 ];
            // double fu = uvf [ widx1 ];
            // double fv = uvf [ widx2 ];

            // weight based on flow magnitude difference
            double uvb_dist_raw = ((bu-bu_o)*(bu-bu_o)+(bv-bv_o)*(bv-bv_o));
            // double uvf_dist_raw = ((fu-fu_o)*(fu-fu_o)+(fv-fv_o)*(fv-fv_o));
            double uvb_dist = uvb_dist_raw*sigma_uvb_sq_inv;
            // double uvf_dist = uvf_dist_raw*sigma_uvf_sq_inv;

            // weight based on geometric distance
            double loc_dist = ((cc-x)*(cc-x)+(rr-y)*(rr-y))*sigma_loc_sq_inv;
            // so slow:
            //double g = exp(-(flow_dist + geometric_dist));
            // double d = uvb_dist + uvf_dist + loc_dist;
            double d = uvb_dist + loc_dist; // 20140807 works
            int exp_idx = min( int( ((d * gstep)/EXP_MAX_) *EXP_NUM_), EXP_NUM_-1);
            double g = exp_[exp_idx] * (1.0 - fr);

            // mexPrintf("dist: %f  eid: %d   g: %f\n", d, exp_idx, g);

            // energy_uvf_local += uvf_dist_raw;
            // energy_uvb_local += uvb_dist_raw;

            filter_norm += g;
            val_x += uvf[widx1]*g;
            val_y += uvf[widx2]*g;
          }
        }
        val_x /= filter_norm;
        val_y /= filter_norm;

        //if (energy_uvb_local > energy_uvf_local) {
          // double aa = (1.0 - m0) * (1.0 - m0);
          // uvf_[idx1] = uvf[idx1] * aa + val_x * (1.0 - aa);
          // uvf_[idx2] = uvf[idx2] * aa + val_y * (1.0 - aa);
          // uvf_[idx1] = uvf[idx1] * (1.0 - m0) + val_x * m0;
          // uvf_[idx2] = uvf[idx2] * (1.0 - m0) + val_y * m0;
          // mexPrintf("changed (x=%d,y=%d)\n", x, y);
          uvf_[idx1] = val_x;
          uvf_[idx2] = val_y;
          // uvf_[idx1] = 99.0;
          // uvf_[idx2] = 99.0;
        // }
      }
    }
  }
  // fin.
  delete exp_;
}