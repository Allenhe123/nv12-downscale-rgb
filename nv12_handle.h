#ifndef NV12_HANDLE_H
#define NV12_HANDLE_H

#include "libyuv.h"
#include "basic_types.h"



// NV12 downscale impl
int NV12Scale(uint8 *psrc_buf, int psrc_w, int psrc_h, uint8 *pdst_buf, int pdst_w, int pdst_h, libyuv::FilterModeEnum pfmode);

int NV12Scale_optimize(uint8 *psrc_buf,
                       int psrc_w, int psrc_h,
                       float offset, float rate,
                       uint8 *pdst_buf, int pdst_w, int pdst_h,
                       libyuv::FilterModeEnum pfmode);

// NV12 downscale whole image
void downscale();

// NV12 downscale as 3 parts 3:1:1
void downscale_part();

// NV12 downscale
void downscale_part_color_convert_thread();

// NV12 to RGB
void nv12_to_rgb();

#endif // NV12_HANDLE_H

