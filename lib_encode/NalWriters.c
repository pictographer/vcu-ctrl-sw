/******************************************************************************
*
* Copyright (C) 2017 Allegro DVT2.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX OR ALLEGRO DVT2 BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of  Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
*
* Except as contained in this notice, the name of Allegro DVT2 shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Allegro DVT2.
*
******************************************************************************/

#include "NalWriters.h"
#include "lib_bitstream/RbspEncod.h"

static void audWrite(IRbspWriter* writer, AL_TBitStreamLite* bitstream, void const* param)
{
  writer->WriteAUD(bitstream, (int)(uintptr_t)param);
}

AL_NalUnit AL_CreateAud(int nut, AL_ESliceType eSliceType)
{
  AL_NalUnit nal = { &audWrite, (void*)(uintptr_t)eSliceType, nut, 0 };
  return nal;
}

static void spsWrite(IRbspWriter* writer, AL_TBitStreamLite* bitstream, void const* param)
{
  writer->WriteSPS(bitstream, param);
}

AL_NalUnit AL_CreateSps(int nut, AL_TSps* sps)
{
  AL_NalUnit nal = { &spsWrite, sps, nut, 1 };
  return nal;
}

static void ppsWrite(IRbspWriter* writer, AL_TBitStreamLite* bitstream, void const* param)
{
  writer->WritePPS(bitstream, param);
}

AL_NalUnit AL_CreatePps(int nut, AL_TPps* pps)
{
  AL_NalUnit nal = { &ppsWrite, pps, nut, 1 };
  return nal;
}

static void vpsWrite(IRbspWriter* writer, AL_TBitStreamLite* bitstream, void const* param)
{
  writer->WriteVPS(bitstream, param);
}

AL_NalUnit AL_CreateVps(AL_THevcVps* vps)
{
  AL_NalUnit nal = { &vpsWrite, vps, AL_HEVC_NUT_VPS, 1 };
  return nal;
}

static void seiPrefixAPSWrite(IRbspWriter* writer, AL_TBitStreamLite* bitstream, void const* param)
{
  SeiPrefixAPSCtx* pCtx = (SeiPrefixAPSCtx*)param;
  writer->WriteSEI_ActiveParameterSets(bitstream, pCtx->vps, pCtx->sps);
}

AL_NalUnit AL_CreateSeiPrefixAPS(SeiPrefixAPSCtx* ctx, int nut)
{
  AL_NalUnit nal = { &seiPrefixAPSWrite, ctx, nut, 0 };
  return nal;
}

static void seiPrefixWrite(IRbspWriter* writer, AL_TBitStreamLite* bitstream, void const* param)
{
  SeiPrefixCtx* pCtx = (SeiPrefixCtx*)param;
  uint32_t uFlags = pCtx->uFlags;

  while(uFlags)
  {
    if(uFlags & SEI_BP)
    {
      writer->WriteSEI_BufferingPeriod(bitstream, pCtx->sps, pCtx->cpbInitialRemovalDelay, 0);
      uFlags &= ~SEI_BP;
    }
    else if(uFlags & SEI_RP)
    {
      writer->WriteSEI_RecoveryPoint(bitstream);
      uFlags &= ~SEI_RP;
    }
    else if(uFlags & SEI_PT)
    {
      writer->WriteSEI_PictureTiming(bitstream, pCtx->sps,
                                     pCtx->cpbRemovalDelay,
                                     pCtx->pPicStatus->uDpbOutputDelay, pCtx->pPicStatus->ePicStruct);
      uFlags &= ~SEI_PT;
    }

    if(!uFlags)
      AL_RbspEncoding_CloseSEI(bitstream);
  }
}

AL_NalUnit AL_CreateSeiPrefix(SeiPrefixCtx* ctx, int nut)
{
  AL_NalUnit nal = { &seiPrefixWrite, ctx, nut, 0 };
  return nal;
}

static void seiSuffixWrite(IRbspWriter* writer, AL_TBitStreamLite* bitstream, void const* param)
{
  SeiSuffixCtx* pCtx = (SeiSuffixCtx*)param;
  writer->WriteSEI_UserDataUnregistered(bitstream, pCtx->uuid);
}

AL_NalUnit AL_CreateSeiSuffix(SeiSuffixCtx* ctx, int nut)
{
  AL_NalUnit nal = { &seiSuffixWrite, ctx, nut, 0 };
  return nal;
}

