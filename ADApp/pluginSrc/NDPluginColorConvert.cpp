/*
 * NDPluginColorConvert.cpp
 *
 * Plugin to convert from one color mode to another
 *
 * Author: Mark Rivers
 *
 * Created December 22, 2008
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include <iocsh.h>

#include "NDPluginDriver.h"
#include "colorMaps.h"
#include "NDPluginColorConvert.h"

#include <epicsExport.h>

static const char *driverName="NDPluginColorConvert";

/* This function returns 1 if it did a conversion, 0 if it did not */
template <typename epicsType>
void NDPluginColorConvert::convertColor(NDArray *pArray)
{
    NDColorMode_t colorModeOut;
    static const char* functionName = "convertColor";
    size_t i, j;
    epicsType *pIn, *pRedIn, *pGreenIn, *pBlueIn;
    epicsType *pOut, *pRedOut=NULL, *pGreenOut=NULL, *pBlueOut=NULL;
    epicsType *p1, *p2, *p3, *p4, *p6, *p7, *p8, *p9; // pointers used for bayer interpolation
    epicsType *pDataIn  = (epicsType *)pArray->pData;
    epicsType *pDataOut=NULL;
    NDArray *pArrayOut=NULL;
    size_t imageSize, rowSize, numRows;
    size_t dims[3];
    NDDimension_t tmpDim;
    enum Colour {red, green, blue};
    double value;
    int colorMode=NDColorModeMono, bayerPattern=NDBayerRGGB;
    int falseColor=0;
    int changedColorMode=0;
    const unsigned char *colorMapR=NULL;
    const unsigned char *colorMapG=NULL;
    const unsigned char *colorMapB=NULL;
    const unsigned char *colorMapRGB=NULL;
    NDAttribute *pAttribute;

    getIntegerParam(NDPluginColorConvertColorModeOut, (int *)&colorModeOut);
    pAttribute = pArray->pAttributeList->find("ColorMode");
    if (pAttribute) pAttribute->getValue(NDAttrInt32, &colorMode);
    pAttribute = pArray->pAttributeList->find("BayerPattern");
    if (pAttribute) pAttribute->getValue(NDAttrInt32, &bayerPattern);

    /* if we have int8 data then check for false color */
    if (pArray->dataType == NDInt8 || pArray->dataType == NDUInt8) {
        getIntegerParam(NDPluginColorConvertFalseColor, &falseColor);
        switch (falseColor) {
        case 1:
            colorMapR = RainbowColorR;
            colorMapG = RainbowColorG;
            colorMapB = RainbowColorB;
            colorMapRGB = RainbowColorRGB;
            break;
        case 2:
            colorMapR = IronColorR;
            colorMapG = IronColorG;
            colorMapB = IronColorB;
            colorMapRGB = IronColorRGB;
            break;
        default:
            falseColor = 0;
        }
    }
    /* This function is called with the lock taken, and it must be set when we exit.
     * The following code can be exected without the mutex because we are not accessing elements of
     * pPvt that other threads can access. */
    this->unlock();
    switch (colorMode) {
        case NDColorModeMono:
            if (pArray->ndims != 2) break;
            rowSize   = pArray->dims[0].size;
            numRows   = pArray->dims[1].size;
            imageSize = rowSize * numRows;
            switch (colorModeOut) {
                case NDColorModeRGB1:
                    dims[0] = 3;
                    dims[1] = rowSize;
                    dims[2] = numRows;
                    pArrayOut = this->pNDArrayPool->alloc(3, dims, pArray->dataType, 0, NULL);
                    tmpDim = pArrayOut->dims[0];
                    /* Copy everything except the data, e.g. uniqueId and timeStamp, attributes. */
                    this->pNDArrayPool->copy(pArray, pArrayOut, 0);
                    /* That replaced the dimensions in the output array, need to fix. */
                    pArrayOut->ndims = 3;
                    pArrayOut->dims[2] = pArrayOut->dims[1];
                    pArrayOut->dims[1] = pArrayOut->dims[0];
                    pArrayOut->dims[0] = tmpDim;
                    pDataOut = (epicsType *)pArrayOut->pData;
                    pOut = pDataOut;
                    pIn  = pDataIn;
                    if (falseColor) {
                        for (i=0; i<imageSize; i++) {
                            memcpy(pOut, colorMapRGB + 3 * ((unsigned char)*pIn++), 3);
                            pOut+=3;
                        }
                    } else {
                        for (i=0; i<imageSize; i++) {
                            *pOut++ = *pIn;
                            *pOut++ = *pIn;
                            *pOut++ = *pIn++;
                        }
                    }
                    changedColorMode = 1;
                    break;
                case NDColorModeRGB2:
                    /* Make a new 3-D array */
                    dims[0] = rowSize;
                    dims[1] = 3;
                    dims[2] = numRows;
                    pArrayOut = this->pNDArrayPool->alloc(3, dims, pArray->dataType, 0, NULL);
                    tmpDim = pArrayOut->dims[1];
                    /* Copy everything except the data, e.g. uniqueId and timeStamp, attributes. */
                    this->pNDArrayPool->copy(pArray, pArrayOut, 0);
                    /* That replaced the dimensions in the output array, need to fix. */
                    pArrayOut->ndims = 3;
                    pArrayOut->dims[2] = pArrayOut->dims[1];
                    pArrayOut->dims[1] = tmpDim;
                    pDataOut = (epicsType *)pArrayOut->pData;
                    pIn  = pDataIn;
                    if (falseColor) {
                        for (i=0; i<numRows; i++) {
                            pRedOut   = pDataOut + 3*i*rowSize;
                            pGreenOut = pRedOut + rowSize;
                            pBlueOut  = pRedOut + 2*rowSize;
                            for (j=0; j<rowSize; j++) {
                                *pRedOut++   = colorMapR[(unsigned char)*pIn];
                                *pGreenOut++ = colorMapG[(unsigned char)*pIn];
                                *pBlueOut++  = colorMapB[(unsigned char)*pIn++];
                            }
                        }
                    } else {
                        for (i=0; i<numRows; i++) {
                            pRedOut   = pDataOut + 3*i*rowSize;
                            pGreenOut = pRedOut + rowSize;
                            pBlueOut  = pRedOut + 2*rowSize;
                            for (j=0; j<rowSize; j++) {
                                *pRedOut++   = *pIn;
                                *pGreenOut++ = *pIn;
                                *pBlueOut++  = *pIn++;
                            }
                        }
                    }
                    changedColorMode = 1;
                    break;
                case NDColorModeRGB3:
                    /* Make a new 3-D array */
                    dims[0] = rowSize;
                    dims[1] = numRows;
                    dims[2] = 3;
                    pArrayOut = this->pNDArrayPool->alloc(3, dims, pArray->dataType, 0, NULL);
                    tmpDim = pArrayOut->dims[2];
                    /* Copy everything except the data, e.g. uniqueId and timeStamp, attributes. */
                    this->pNDArrayPool->copy(pArray, pArrayOut, 0);
                    /* That replaced the dimensions in the output array, need to fix. */
                    pArrayOut->ndims = 3;
                    pArrayOut->dims[2] = tmpDim;
                    pDataOut = (epicsType *)pArrayOut->pData;
                    pRedOut   = pDataOut;
                    pGreenOut = pDataOut + imageSize;
                    pBlueOut  = pDataOut + 2*imageSize;
                    pIn  = pDataIn;
                    if (falseColor) {
                        for (i=0; i<imageSize; i++) {
                            *pRedOut++   = colorMapR[(unsigned char)*pIn];
                            *pGreenOut++ = colorMapG[(unsigned char)*pIn];
                            *pBlueOut++  = colorMapB[(unsigned char)*pIn++];
                        }
                    } else {
                        for (i=0; i<imageSize; i++) {
                            *pRedOut++   = *pIn;
                            *pGreenOut++ = *pIn;
                            *pBlueOut++  = *pIn++;
                        }
                    }
                    changedColorMode = 1;
                    break;
                default:
                    break;
            }
            break;

        case NDColorModeBayer:
            if (pArray->ndims != 2) break;
            rowSize   = pArray->dims[0].size; // x pixels
            numRows   = pArray->dims[1].size; // y pixels
            imageSize = rowSize * numRows;    // total pixels

            // configure output array depending on output mode
            switch (colorModeOut) {
                case NDColorModeMono:
                    dims[0] = rowSize;
                    dims[1] = numRows;
                    pArrayOut = this->pNDArrayPool->copy(pArray, NULL, 0);
                    pDataOut = (epicsType *)pArrayOut->pData;
                    break;

                case NDColorModeRGB1:
                    dims[0] = 3;
                    dims[1] = rowSize;
                    dims[2] = numRows;
                    // There is a problem: the uniqueId and timeStamp are not preserved!
                    pArrayOut = this->pNDArrayPool->alloc(3, dims, pArray->dataType, 0, NULL);
                    tmpDim = pArrayOut->dims[0];
                    this->pNDArrayPool->copy(pArray, pArrayOut, 0);
                    pArrayOut->uniqueId = pArray->uniqueId;
                    pArrayOut->epicsTS = pArray->epicsTS;
                    pArrayOut->timeStamp = pArray->timeStamp;
                    pArrayOut->ndims = 3;
                    pArrayOut->dims[2] = pArrayOut->dims[1];
                    pArrayOut->dims[1] = pArrayOut->dims[0];
                    pArrayOut->dims[0] = tmpDim;
                    pArrayOut->dims[0].size = 3;
                    pDataOut = (epicsType *)pArrayOut->pData;
                    break;

                case NDColorModeRGB2:
                    dims[0] = rowSize;
                    dims[1] = 3;
                    dims[2] = numRows;
                    pArrayOut = this->pNDArrayPool->alloc(3, dims, pArray->dataType, 0, NULL);
                    tmpDim = pArrayOut->dims[1];
                    this->pNDArrayPool->copy(pArray, pArrayOut, 0);
                    pArrayOut->uniqueId = pArray->uniqueId;
                    pArrayOut->epicsTS = pArray->epicsTS;
                    pArrayOut->timeStamp = pArray->timeStamp;
                    pArrayOut->ndims = 3;
                    pArrayOut->dims[2] = pArrayOut->dims[1];
                    pArrayOut->dims[1] = tmpDim;
                    pArrayOut->dims[1].size = 3;
                    pDataOut = (epicsType *)pArrayOut->pData;
                    break;

                case NDColorModeRGB3:
                    dims[0] = rowSize;
                    dims[1] = numRows;
                    dims[2] = 3;
                    pArrayOut = this->pNDArrayPool->alloc(3, dims, pArray->dataType, 0, NULL);
                    tmpDim = pArrayOut->dims[2];
                    this->pNDArrayPool->copy(pArray, pArrayOut, 0);
                    pArrayOut->uniqueId = pArray->uniqueId;
                    pArrayOut->epicsTS = pArray->epicsTS;
                    pArrayOut->timeStamp = pArray->timeStamp;
                    pArrayOut->ndims = 3;
                    pArrayOut->dims[2] = tmpDim;
                    pArrayOut->dims[2].size = 3;
                    pDataOut = (epicsType *)pArrayOut->pData;
                    pRedOut   = pDataOut;
                    pGreenOut = pDataOut + imageSize;
                    pBlueOut  = pDataOut + 2*imageSize;
                    break;

                default:
                    break;
            }

            pIn = pDataIn; // will be used to iterate over input pixel array
            pOut = pDataOut; // for writing to output pixel array

            // loop over each pixel
            for(unsigned int ipixel=0; ipixel<imageSize; ipixel++) {
                unsigned int x = ipixel % rowSize;
                unsigned int y = (unsigned int)(ipixel / rowSize);
                // x and y value used for determining pixel colour w.r.t. bayer pattern & offsets
                unsigned int bx = (unsigned int)(x + pArray->dims[0].offset); // original x before offset
                unsigned int by = (unsigned int)(y + pArray->dims[1].offset); // original y before offset
                // account for bayer pattern in x and y
                // bayerPattern = {0:RGGB, 1:GBRG. 2:GRBG, 3:BGGR}
                bx += int(bayerPattern>>1)&1; // first bit of bayer pattern enum
                by += int(bayerPattern)&1; // second bit of bayer pattern enum
                unsigned int rvalue = 0;
                unsigned int gvalue = 0;
                unsigned int bvalue = 0;
                unsigned int whatcolour = red;

                p1 = pIn-rowSize-1; // above left of pixel
                p2 = pIn-rowSize;   // above pixel
                p3 = pIn-rowSize+1; // above right of pixel
                p4 = pIn-1;         // left  of pixel
                p6 = pIn+1;         // right of pixel
                p7 = pIn+rowSize-1; // below left of pixel
                p8 = pIn+rowSize;   // below pixel
                p9 = pIn+rowSize+1; // below right of pixel

                // coordinates from original image to get colour type
                if (bx%2==0 && by%2==0) { rvalue = (unsigned int)*pIn; whatcolour = red; }
                else if (bx%2==1 && by%2==1) { bvalue = (unsigned int)*pIn; whatcolour = blue; }
                else if (bx%2 != by%2) { gvalue = (unsigned int)*pIn; whatcolour = green; }

                // only interpolate pixels not touching a border
                if (x>0 && x<rowSize-1 && y>0 && y<numRows-1) {
                    if (whatcolour == red) {
                        // if pixel is red
                        bvalue = (unsigned int)(*p1 + *p3 + *p7 + *p9) / 4;
                        gvalue = (unsigned int)(*p2 + *p4 + *p6 + *p8) / 4;
                    }
                    if (whatcolour == blue) {
                        // if pixel is blue
                        rvalue = (unsigned int)(*p1 + *p3 + *p7 + *p9) / 4;
                        gvalue = (unsigned int)(*p2 + *p4 + *p6 + *p8) / 4;
                    }
                    if (whatcolour == green && bx%2 == 1) {
                        // if pixel is green (next to red)
                        rvalue = (unsigned int)(*p4 + *p6) / 2;
                        bvalue = (unsigned int)(*p2 + *p8) / 2;
                    }
                    if (whatcolour == green && bx%2 == 0) {
                        // if pixel is green (next to blue)
                        bvalue = (unsigned int)(*p4 + *p6) / 2;
                        rvalue = (unsigned int)(*p2 + *p8) / 2;
                    }
                }

                // write values out depending on output mode
                switch (colorModeOut) {
                    case NDColorModeMono:
                        *pOut++ = epicsType((rvalue + gvalue + bvalue) / 3);
                        pIn++; // increment input data pointer
                        changedColorMode = 1;
                        break;

                    case NDColorModeRGB1:
                        *pOut++ = (epicsType)rvalue; // red
                        *pOut++ = (epicsType)gvalue; // green
                        *pOut++ = (epicsType)bvalue; // blue
                        pIn++; // increment input data pointer
                        changedColorMode = 1;
                        break;

                    case NDColorModeRGB2:
                        // at start of input data row, jump output pointers by 3 rows
                        if(x==0) {
                            pRedOut   = pDataOut + 3*y*rowSize;
                            pGreenOut = pRedOut  + rowSize;
                            pBlueOut  = pRedOut  + 2*rowSize;
                        }
                        *pRedOut++   = (epicsType)rvalue;
                        *pGreenOut++ = (epicsType)gvalue;
                        *pBlueOut++  = (epicsType)bvalue;
                        pIn++; // increment input data pointer
                        changedColorMode = 1;
                        break;

                    case NDColorModeRGB3:
                        *pRedOut++   = (epicsType)rvalue;
                        *pGreenOut++ = (epicsType)gvalue;
                        *pBlueOut++  = (epicsType)bvalue;
                        pIn++; // increment input data pointer
                        changedColorMode = 1;
                        break;

                    default:
                        break;
                }
            }
            break;

        case NDColorModeRGB1:
            if (pArray->ndims != 3) break;
            rowSize   = pArray->dims[1].size;
            numRows   = pArray->dims[2].size;
            imageSize = rowSize * numRows;
            switch (colorModeOut) {
                case NDColorModeMono:
                    /* Make a new 2-D array */
                    dims[0] = rowSize;
                    dims[1] = numRows;
                    pArrayOut = this->pNDArrayPool->alloc(2, dims, pArray->dataType, 0, NULL);
                    /* Copy everything except the data, e.g. uniqueId and timeStamp, attributes. */
                    this->pNDArrayPool->copy(pArray, pArrayOut, 0);
                    /* That replaced the dimensions in the output array, need to fix. */
                    pArrayOut->ndims = 2;
                    pArrayOut->dims[0] = pArrayOut->dims[1];
                    pArrayOut->dims[1] = pArrayOut->dims[2];
                    pDataOut = (epicsType *)pArrayOut->pData;
                    pOut = pDataOut;
                    pIn = pDataIn;
                    for (i=0; i<imageSize; i++) {
                        value  = (*pIn + *(pIn+1) + *(pIn+2))/3.;
                        *pOut++ = (epicsType)value;
                        pIn += 3;
                    }
                    changedColorMode = 1;
                    break;
                case NDColorModeRGB2:
                    pArrayOut = this->pNDArrayPool->copy(pArray, NULL, 0);
                    pDataOut = (epicsType *)pArrayOut->pData;
                    pIn = pDataIn;
                    for (i=0; i<numRows; i++) {
                        pRedOut   = pDataOut + 3*i*rowSize;
                        pGreenOut = pRedOut + rowSize;
                        pBlueOut  = pRedOut + 2*rowSize;
                        for (j=0; j<rowSize; j++) {
                            *pRedOut++   = *pIn++;
                            *pGreenOut++ = *pIn++;
                            *pBlueOut++  = *pIn++;
                        }
                    }
                    memcpy(&pArrayOut->dims[0], &pArray->dims[1], sizeof(NDDimension_t));
                    memcpy(&pArrayOut->dims[1], &pArray->dims[0], sizeof(NDDimension_t));
                    memcpy(&pArrayOut->dims[2], &pArray->dims[2], sizeof(NDDimension_t));
                    changedColorMode = 1;
                    break;
                case NDColorModeRGB3:
                    pArrayOut = this->pNDArrayPool->copy(pArray, NULL, 0);
                    pDataOut = (epicsType *)pArrayOut->pData;
                    pIn = pDataIn;
                    pRedOut   = pDataOut;
                    pGreenOut = pDataOut + imageSize;
                    pBlueOut  = pDataOut + 2*imageSize;
                    for (i=0; i<imageSize; i++) {
                        *pRedOut++   = *pIn++;
                        *pGreenOut++ = *pIn++;
                        *pBlueOut++  = *pIn++;
                    }
                    memcpy(&pArrayOut->dims[0], &pArray->dims[1], sizeof(NDDimension_t));
                    memcpy(&pArrayOut->dims[1], &pArray->dims[2], sizeof(NDDimension_t));
                    memcpy(&pArrayOut->dims[2], &pArray->dims[0], sizeof(NDDimension_t));
                    changedColorMode = 1;
                    break;
                default:
                    break;
            }
            break;
        case NDColorModeRGB2:
            if (pArray->ndims != 3) break;
            rowSize   = pArray->dims[0].size;
            numRows   = pArray->dims[2].size;
            imageSize = rowSize * numRows;
            switch (colorModeOut) {
                case NDColorModeMono:
                    /* Make a new 2-D array */
                    dims[0] = rowSize;
                    dims[1] = numRows;
                    pArrayOut = this->pNDArrayPool->alloc(2, dims, pArray->dataType, 0, NULL);
                    /* Copy everything except the data, e.g. uniqueId and timeStamp, attributes. */
                    this->pNDArrayPool->copy(pArray, pArrayOut, 0);
                    /* That replaced the dimensions in the output array, need to fix. */
                    pArrayOut->ndims = 2;
                    pArrayOut->dims[1] = pArrayOut->dims[2];
                    pDataOut = (epicsType *)pArrayOut->pData;
                    pOut = pDataOut;
                    for (i=0; i<numRows; i++) {
                        pRedIn   = pDataIn + 3*i*rowSize;
                        pGreenIn = pRedIn + rowSize;
                        pBlueIn  = pRedIn + 2*rowSize;
                        for (j=0; j<rowSize; j++) {
                            value = (*pRedIn++ + *pGreenIn++ + *pBlueIn++)/3.;
                            *pOut++ = (epicsType)value;
                        }
                    }
                    changedColorMode = 1;
                    break;
                case NDColorModeRGB1:
                    pArrayOut = this->pNDArrayPool->copy(pArray, NULL, 0);
                    pDataOut = (epicsType *)pArrayOut->pData;
                    pOut = pDataOut;
                    for (i=0; i<numRows; i++) {
                        pRedIn   = pDataIn + 3*i*rowSize;
                        pGreenIn = pRedIn + rowSize;
                        pBlueIn  = pRedIn + 2*rowSize;
                        for (j=0; j<rowSize; j++) {
                            *pOut++  = *pRedIn++;
                            *pOut++  = *pGreenIn++;
                            *pOut++  = *pBlueIn++;
                        }
                    }
                    memcpy(&pArrayOut->dims[0], &pArray->dims[1], sizeof(NDDimension_t));
                    memcpy(&pArrayOut->dims[1], &pArray->dims[0], sizeof(NDDimension_t));
                    memcpy(&pArrayOut->dims[2], &pArray->dims[2], sizeof(NDDimension_t));
                    changedColorMode = 1;
                    break;
                case NDColorModeRGB3:
                    pArrayOut = this->pNDArrayPool->copy(pArray, NULL, 0);
                    pDataOut = (epicsType *)pArrayOut->pData;
                    pRedOut   = pDataOut;
                    pGreenOut = pDataOut + imageSize;
                    pBlueOut  = pDataOut + 2*imageSize;
                    for (i=0; i<numRows; i++) {
                        pRedIn   = pDataIn + 3*i*rowSize;
                        pGreenIn = pRedIn + rowSize;
                        pBlueIn  = pRedIn + 2*rowSize;
                        for (j=0; j<rowSize; j++) {
                            *pRedOut++   = *pRedIn++;
                            *pGreenOut++ = *pGreenIn++;
                            *pBlueOut++  = *pBlueIn++;
                        }
                    }
                    memcpy(&pArrayOut->dims[0], &pArray->dims[0], sizeof(NDDimension_t));
                    memcpy(&pArrayOut->dims[1], &pArray->dims[2], sizeof(NDDimension_t));
                    memcpy(&pArrayOut->dims[2], &pArray->dims[1], sizeof(NDDimension_t));
                    changedColorMode = 1;
                    break;
                default:
                    break;
            }
            break;
        case NDColorModeRGB3:
            if (pArray->ndims != 3) break;
            rowSize   = pArray->dims[0].size;
            numRows   = pArray->dims[1].size;
            imageSize = rowSize * numRows;
            switch (colorModeOut) {
                case NDColorModeMono:
                    /* Make a new 2-D array */
                    dims[0] = rowSize;
                    dims[1] = numRows;
                    pArrayOut = this->pNDArrayPool->alloc(2, dims, pArray->dataType, 0, NULL);
                    /* Copy everything except the data, e.g. uniqueId and timeStamp, attributes. */
                    this->pNDArrayPool->copy(pArray, pArrayOut, 0);
                    /* That replaced the dimensions in the output array, need to fix. */
                    pArrayOut->ndims = 2;
                    pDataOut = (epicsType *)pArrayOut->pData;
                    pOut = pDataOut;
                    pRedIn   = pDataIn;
                    pGreenIn = pDataIn + imageSize;
                    pBlueIn  = pDataIn + 2*imageSize;
                    for (i=0; i<imageSize; i++) {
                        value = (*pRedIn++ + *pGreenIn++ + *pBlueIn++)/3.;
                        *pOut++ = (epicsType)value;
                    }
                    changedColorMode = 1;
                    break;
                case NDColorModeRGB1:
                    pArrayOut = this->pNDArrayPool->copy(pArray, NULL, 0);
                    pDataOut = (epicsType *)pArrayOut->pData;
                    pRedIn   = pDataIn;
                    pGreenIn = pDataIn + imageSize;
                    pBlueIn  = pDataIn + 2*imageSize;
                    pOut = pDataOut;
                    for (i=0; i<imageSize; i++) {
                        *pOut++ = *pRedIn++;
                        *pOut++ = *pGreenIn++;
                        *pOut++ = *pBlueIn++;
                    }
                    memcpy(&pArrayOut->dims[0], &pArray->dims[2], sizeof(NDDimension_t));
                    memcpy(&pArrayOut->dims[1], &pArray->dims[0], sizeof(NDDimension_t));
                    memcpy(&pArrayOut->dims[2], &pArray->dims[1], sizeof(NDDimension_t));
                    changedColorMode = 1;
                    break;
                case NDColorModeRGB2:
                    pArrayOut = this->pNDArrayPool->copy(pArray, NULL, 0);
                    pDataOut = (epicsType *)pArrayOut->pData;
                    pRedIn   = pDataIn;
                    pGreenIn = pDataIn + imageSize;
                    pBlueIn  = pDataIn + 2*imageSize;
                    for (i=0; i<numRows; i++) {
                        pRedOut   = pDataOut + 3*i*rowSize;
                        pGreenOut = pRedOut + rowSize;
                        pBlueOut  = pRedOut + 2*rowSize;
                        for (j=0; j<rowSize; j++) {
                            *pRedOut++   = *pRedIn++;
                            *pGreenOut++ = *pGreenIn++;
                            *pBlueOut++  = *pBlueIn++;
                        }
                    }
                    memcpy(&pArrayOut->dims[0], &pArray->dims[0], sizeof(NDDimension_t));
                    memcpy(&pArrayOut->dims[1], &pArray->dims[2], sizeof(NDDimension_t));
                    memcpy(&pArrayOut->dims[2], &pArray->dims[1], sizeof(NDDimension_t));
                    changedColorMode = 1;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }

    if ( pArrayOut ) {
        /* Set nBits based on total bits used to represent one pixel element. */
        pArrayOut->bitsPerElement = GetNDColorModeBits( colorModeOut, pArrayOut->dataType );
    }

    /* If the output array pointer is null then no conversion was done, copy the input to the output */
    if (!pArrayOut) pArrayOut = this->pNDArrayPool->copy(pArray, NULL, 1);
    this->lock();
    /* Get the attributes for this plugin */
    this->getAttributes(pArrayOut->pAttributeList);
    /* If we changed the color mode then set the attribute */
    if (changedColorMode) pArrayOut->pAttributeList->add("ColorMode", "Color Mode", NDAttrInt32, &colorModeOut);

    // Do NDArray callbacks.  We don't need to copy the array or get the attributes
    NDPluginDriver::endProcessCallbacks(pArrayOut, false, false);
    asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW,
              "%s:%s: pArray->colorMode=%d, colorModeOut=%d, pArrayOut=%p\n",
              driverName, functionName, colorMode, colorModeOut, pArrayOut);
}

/** Callback function that is called by the NDArray driver with new NDArray data.
  * Looks for the NDArray attribute called "ColorMode" to determine the color
  * mode of the input array.  Uses the parameter NDPluginColorConvertColorModeOut
  * to determine the desired color mode of the output array.  The NDArray is converted
  * between these color modes if possible.  If not the input array is passed on without
  * being changed.  Does callbacks to all registered clients on the asynGenericPointer
  * interface with the output array.
  * \param[in] pArray  The NDArray from the callback.
  */
void NDPluginColorConvert::processCallbacks(NDArray *pArray)
{
    /* This function converts the color mode.
     * If no conversion can be performed it simply uses the input as the output
     * It is called with the mutex already locked.  It unlocks it during long calculations when private
     * structures don't need to be protected.
     */

    static const char* functionName = "processCallbacks";

    /* Call the base class method */
    NDPluginDriver::beginProcessCallbacks(pArray);

    asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW,
              "%s:%s: dataType=%d\n",
              driverName, functionName, pArray->dataType);

    switch (pArray->dataType) {
        case NDInt8:
            this->convertColor<epicsInt8>(pArray);
            break;
        case NDUInt8:
            this->convertColor<epicsUInt8>(pArray);
            break;
        case NDInt16:
            this->convertColor<epicsInt16>(pArray);
            break;
        case NDUInt16:
            this->convertColor<epicsUInt16>(pArray);
            break;
       case NDInt32:
            this->convertColor<epicsInt32>(pArray);
            break;
        case NDUInt32:
            this->convertColor<epicsUInt32>(pArray);
            break;
       case NDInt64:
            this->convertColor<epicsInt64>(pArray);
            break;
        case NDUInt64:
            this->convertColor<epicsUInt64>(pArray);
            break;
        case NDFloat32:
            this->convertColor<epicsFloat32>(pArray);
            break;
        case NDFloat64:
            this->convertColor<epicsFloat64>(pArray);
            break;
        default:
            asynPrint(this->pasynUserSelf, ASYN_TRACE_ERROR,
                      "%s:%s: ERROR: unknown data type=%d\n",
                      driverName, functionName, pArray->dataType);
            break;
    }

    callParamCallbacks();
}


/** Constructor for NDPluginColorConvert; most parameters are simply passed to NDPluginDriver::NDPluginDriver.
  * After calling the base class constructor this method sets reasonable default values for all of the
  * ROI parameters.
  * \param[in] portName The name of the asyn port driver to be created.
  * \param[in] queueSize The number of NDArrays that the input queue for this plugin can hold when
  *            NDPluginDriverBlockingCallbacks=0.  Larger queues can decrease the number of dropped arrays,
  *            at the expense of more NDArray buffers being allocated from the underlying driver's NDArrayPool.
  * \param[in] blockingCallbacks Initial setting for the NDPluginDriverBlockingCallbacks flag.
  *            0=callbacks are queued and executed by the callback thread; 1 callbacks execute in the thread
  *            of the driver doing the callbacks.
  * \param[in] NDArrayPort Name of asyn port driver for initial source of NDArray callbacks.
  * \param[in] NDArrayAddr asyn port driver address for initial source of NDArray callbacks.
  * \param[in] maxBuffers The maximum number of NDArray buffers that the NDArrayPool for this driver is
  *            allowed to allocate. Set this to 0 to allow an unlimited number of buffers.
  * \param[in] maxMemory The maximum amount of memory that the NDArrayPool for this driver is
  *            allowed to allocate. Set this to 0 to allow an unlimited amount of memory.
  * \param[in] priority The thread priority for the asyn port driver thread if ASYN_CANBLOCK is set in asynFlags.
  * \param[in] stackSize The stack size for the asyn port driver thread if ASYN_CANBLOCK is set in asynFlags.
  * \param[in] maxThreads The maximum number of threads this driver is allowed to use. If 0 then 1 will be used.
  */
NDPluginColorConvert::NDPluginColorConvert(const char *portName, int queueSize, int blockingCallbacks,
                                           const char *NDArrayPort, int NDArrayAddr,
                                           int maxBuffers, size_t maxMemory,
                                           int priority, int stackSize, int maxThreads)
    /* Invoke the base class constructor */
    : NDPluginDriver(portName, queueSize, blockingCallbacks,
                   NDArrayPort, NDArrayAddr, 1, maxBuffers, maxMemory,
                   asynGenericPointerMask,
                   asynGenericPointerMask,
                   0, 1, priority, stackSize, maxThreads)  /* Not ASYN_CANBLOCK or ASYN_MULTIDEVICE, do autoConnect */
{
    //static const char *functionName = "NDPluginColorConvert";

    createParam(NDPluginColorConvertColorModeOutString, asynParamInt32, &NDPluginColorConvertColorModeOut);
    createParam(NDPluginColorConvertFalseColorString,   asynParamInt32, &NDPluginColorConvertFalseColor);

    /* Set the plugin type string */
    setStringParam(NDPluginDriverPluginType, "NDPluginColorConvert");

    setIntegerParam(NDPluginColorConvertColorModeOut, NDColorModeMono);

    // Enable ArrayCallbacks.
    // This plugin currently ignores this setting and always does callbacks, so make the setting reflect the behavior
    setIntegerParam(NDArrayCallbacks, 1);

    /* Try to connect to the array port */
    connectToArrayPort();
}

extern "C" int NDColorConvertConfigure(const char *portName, int queueSize, int blockingCallbacks,
                                          const char *NDArrayPort, int NDArrayAddr,
                                          int maxBuffers, size_t maxMemory,
                                          int priority, int stackSize, int maxThreads)
{
    NDPluginColorConvert *pPlugin = new NDPluginColorConvert(portName, queueSize, blockingCallbacks, NDArrayPort, NDArrayAddr,
                                                             maxBuffers, maxMemory, priority, stackSize, maxThreads);
    return pPlugin->start();
}

/** EPICS iocsh shell commands */
static const iocshArg initArg0 = { "portName",iocshArgString};
static const iocshArg initArg1 = { "frame queue size",iocshArgInt};
static const iocshArg initArg2 = { "blocking callbacks",iocshArgInt};
static const iocshArg initArg3 = { "NDArrayPort",iocshArgString};
static const iocshArg initArg4 = { "NDArrayAddr",iocshArgInt};
static const iocshArg initArg5 = { "maxBuffers",iocshArgInt};
static const iocshArg initArg6 = { "maxMemory",iocshArgInt};
static const iocshArg initArg7 = { "priority",iocshArgInt};
static const iocshArg initArg8 = { "stackSize",iocshArgInt};
static const iocshArg initArg9 = { "maxThreads",iocshArgInt};
static const iocshArg * const initArgs[] = {&initArg0,
                                            &initArg1,
                                            &initArg2,
                                            &initArg3,
                                            &initArg4,
                                            &initArg5,
                                            &initArg6,
                                            &initArg7,
                                            &initArg8,
                                            &initArg9};
static const iocshFuncDef initFuncDef = {"NDColorConvertConfigure",10,initArgs};
static void initCallFunc(const iocshArgBuf *args)
{
    NDColorConvertConfigure(args[0].sval, args[1].ival, args[2].ival,
                               args[3].sval, args[4].ival, args[5].ival,
                               args[6].ival, args[7].ival, args[8].ival,
                               args[9].ival);
}

extern "C" void NDColorConvertRegister(void)
{
    iocshRegister(&initFuncDef,initCallFunc);
}

extern "C" {
epicsExportRegistrar(NDColorConvertRegister);
}
