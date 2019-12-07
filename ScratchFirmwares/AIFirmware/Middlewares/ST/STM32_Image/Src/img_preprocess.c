/**
  ******************************************************************************
  * @file    img_preprocess.c
  * @author  MCD Application Team
  * @brief   Library of functions for image preprocessing before NN inference
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "img_preprocess.h"

/** @addtogroup Middlewares
  * @{
  */

/** @addtogroup STM32_Image
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/

/**
  * @brief  Performs pixel conversion from 8-bits integer to float simple precision with asymmetric normalization, i.e. in the range [0,+1]
  * @param  pSrc     Pointer to source buffer
  * @param  pDst     Pointer to destination buffer
  * @param  pixels   Number of pixels
  * @retval void     None
  */
void RGB24_to_Float_Asym(void *pSrc, void *pDst, uint32_t pixels)
{
  struct rgb
  {
    uint8_t r, g, b;
  };
  struct rgbf
  {
    float r, g, b;
  };
  struct rgb *pivot = (struct rgb *) pSrc;
  struct rgbf *dest = (struct rgbf *) pDst;
  for (int i = 0; i < pixels; i++)
  {
    dest[i].r = (((float)(pivot[i].b)) / 255.0F);
    dest[i].g = (((float)(pivot[i].g)) / 255.0F);
    dest[i].b = (((float)(pivot[i].r)) / 255.0F);
  }

  /*==> NN input data in the range [0 , +1]*/
}

/**
  * @brief  Performs pixel conversion from 8-bits integer to float simple precision with symmetric normalization, i.e. in the range [-1,+1]
  * @param  pSrc     Pointer to source buffer
  * @param  pDst     Pointer to destination buffer
  * @param  pixels   Number of pixels
  * @retval void     None
  */
void RGB24_to_Float_Sym(void *pSrc, void *pDst, uint32_t pixels)
{
  struct rgb
  {
    uint8_t r, g, b;
  };
  struct rgbf
  {
    float r, g, b;
  };
  struct rgb *pivot = (struct rgb *) pSrc;
  struct rgbf *dest = (struct rgbf *) pDst;
  for (int i = 0; i < pixels; i++)
  {
    dest[i].r = ((float)pivot[i].b / 127.5F) - 1.0F;
    dest[i].g = ((float)pivot[i].g / 127.5F) - 1.0F;
    dest[i].b = ((float)pivot[i].r / 127.5F) - 1.0F;
  }

  /*==> NN input data in the range [-1 , +1]*/
}

/**
  * @brief  Performs pixel conversion from 8-bits integer to 8-bits quantized format expected by NN input with normalization in the range [0,+1]
  * @param  pSrc            Pointer to source buffer
  * @param  pDst            Pointer to destination buffer
  * @param  pixels          Number of pixels
  * @param  q_input_shift   Shift to be applied for normalization in the range [0,+1]
  * @retval void     None
  */
void RGB24_to_8FXP(void *pSrc, void *pDst, uint32_t pixels, uint32_t q_input_shift)
{
  struct rgb
  {
    uint8_t r, g, b;
  };

  struct rgb_fxp
  {
    uint8_t r, g, b;
  };

  struct rgbf
  {
    float r, g, b;
  };

  struct rgb *pivot = (struct rgb *) pSrc;
  struct rgb_fxp *dest = (struct rgb_fxp *) pDst;

  uint16_t tmp_fxp_16_1, tmp_fxp_16_2;
  for (int i = 0; i < pixels; i++)
  {
    tmp_fxp_16_1 = pivot[i].r + (1 << q_input_shift);
    tmp_fxp_16_2 = pivot[i].b + (1 << q_input_shift);
    dest[i].r = __USAT(tmp_fxp_16_2 >> (1 + q_input_shift), 8);
    dest[i].b = __USAT(tmp_fxp_16_1 >> (1 + q_input_shift), 8);
    tmp_fxp_16_1 = pivot[i].g + (1 << q_input_shift);
    dest[i].g = __USAT(tmp_fxp_16_1 >> (1 + q_input_shift), 8);
  }
}


/**
  * @brief  Performs image (or selected Region Of Interest) resizing using bilinear interpolation
  * @param  srcImage     Pointer to source image buffer
  * @param  srcW         Source image width
  * @param  srcH         Source image height
  * @param  pixelSize    Number of bytes per pixel
  * @param  roiX         Region Of Interest x starting location 
  * @param  roiY         Region Of Interest y starting location
  * @param  roiW         Region Of Interest width
  * @param  roiH         Region Of Interest height
  * @param  dstImage     Pointer to destination image buffer
  * @param  dstW         Destination image width
  * @param  dstH         Destination image height
  * @retval void         None
  */
void ImageResize(uint8_t *srcImage, uint32_t srcW, uint32_t srcH,
                 uint32_t pixelSize, uint32_t roiX, uint32_t roiY,
                 uint32_t roiW, uint32_t roiH,  uint8_t *dstImage,
                 uint32_t dstW, uint32_t dstH)
{
  int32_t srcStride;
  float widthRatio;
  float heightRatio;

  int32_t maxWidth;
  int32_t maxHeight;

  float srcX, srcY, dX1, dY1, dX2, dY2;
  int32_t dstX1, srcY1, dstX2, srcY2;

  uint8_t *tmp1, *tmp2;
  uint8_t *p1, *p2, *p3, *p4;

  int32_t offset1;
  int32_t offset2;

  srcStride = pixelSize * srcW;

  widthRatio = ((roiW ? roiW : srcW) / (float) dstW);
  heightRatio = ((roiH ? roiH : srcH) / (float) dstH);

  /* Get horizontal and vertical limits. */
  maxWidth = (roiW ? roiW : srcW) - 1;
  maxHeight = (roiH ? roiH : srcH) - 1;

  for (int32_t y = 0; y < dstH; y++)
  {
    /* Get Y from source. */
    srcY = ((float) y * heightRatio) + roiY;
    srcY1 = (int32_t) srcY;
    srcY2 = (srcY1 == maxHeight) ? srcY1 : srcY1 + 1;
    dY1 = srcY - (float) srcY1;
    dY2 = 1.0f - dY1;

    /* Calculates the pointers to the two needed lines of the source. */
    tmp1 = srcImage + srcY1 * srcStride;
    tmp2 = srcImage + srcY2 * srcStride;

    for (int32_t x = 0; x < dstW; x++)
    {
      /* Get X from source. */
      srcX = x * widthRatio + roiX;
      dstX1 = (int32_t) srcX;
      dstX2 = (dstX1 == maxWidth) ? dstX1 : dstX1 + 1;
      dX1 = srcX - /*(float32)*/dstX1;
      dX2 = 1.0f - dX1;

      /* Calculates the four points (p1,p2, p3, p4) of the source. */
      offset1 = dstX1 * pixelSize;
      offset2 = dstX2 * pixelSize;
      p1 = tmp1 + offset1;
      p2 = tmp1 + offset2;
      p3 = tmp2 + offset1;
      p4 = tmp2 + offset2;
      /* For each channel, interpolate the four points. */
      for (int32_t ch = 0; ch < pixelSize; ch++, dstImage++, p1++, p2++, p3++, p4++)
      {
        *dstImage = (uint8_t)(dY2 * (dX2 * (*p1) + dX1 * (*p2)) + dY1 * (dX2 * (*p3) + dX1 * (*p4)));
      }
    }
  }
}

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
