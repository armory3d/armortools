#!/bin/bash

#
# Copyright 2014-2015 Dario Manesku. All rights reserved.
# License: http://www.opensource.org/licenses/BSD-2-Clause
#

CMFT=./cmft-osx

# Prints help.
#eval $CMFT --help

# Use this to list available OpenCL devices that can be used with cmft for processing.
#eval $CMFT --printCLDevices

# Typical parameters for irradiance filter.
# eval $CMFT $@ --input "test.jpg"           \
#              --filter irradiance               \
#              --srcFaceSize 0                   \
#              --dstFaceSize 0                   \
#              --outputNum 1                     \
#              --output0 "test_irr"         \
#              --output0params hdr,rgbe,latlong

# Typical parameters for generating spherical harmonics coefficients.
eval $CMFT $@ --input "test.hdr"           \
             --filter shcoeffs                 \
             --outputNum 1                     \
             --output0 "test"

# Typical parameters for radiance filter.
# eval $CMFT $@ --input "test.jpg"           \
#               ::Filter options                  \
#               --filter radiance                 \
#               --srcFaceSize 64                 \
#               --excludeBase false               \
#               --mipCount 7                      \
#               --glossScale 10                   \
#               --glossBias 3                     \
#               --lightingModel blinnbrdf         \
#               --edgeFixup none                  \
#               --dstFaceSize 64                 \
#               ::Processing devices              \
#               --numCpuProcessingThreads 4       \
#               --useOpenCL true                  \
#               --clVendor anyGpuVendor           \
#               --deviceType gpu                  \
#               --deviceIndex 0                   \
#               ::Aditional operations            \
#               --inputGammaNumerator 2.2         \
#               --inputGammaDenominator 1.0       \
#               --outputGammaNumerator 1.0        \
#               --outputGammaDenominator 2.2      \
#               --generateMipChain true          \
#               ::Output                          \
#               --outputNum 1                     \
#               --output0 "envmap_rad"       \
#               --output0params hdr,rgbe,latlong \
              # --output1 "okretnica_pmrem"       \
              # --output1params ktx,rgba8,cubemap

# Cmft can also be run without any processing filter. This can be used for performing image manipulations or exporting different image format.
#eval $CMFT $@ --input "okretnica.tga"           \
#              --filter none                     \
#              ::Aditional operations            \
#              --inputGamma 1.0                  \
#              --inputGammaDenominator 1.0       \
#              --outputGamma 1.0                 \
#              --outputGammaDenominator 1.0      \
#              --generateMipChain true           \
#              ::Cubemap transformations         \
#              --posXrotate90                    \
#              --posXrotate180                   \
#              --posXrotate270                   \
#              --posXflipH                       \
#              --posXflipV                       \
#              --negXrotate90                    \
#              --negXrotate180                   \
#              --negXrotate270                   \
#              --negXflipH                       \
#              --negXflipV                       \
#              --posYrotate90                    \
#              --posYrotate180                   \
#              --posYrotate270                   \
#              --posYflipH                       \
#              --posYflipV                       \
#              --negYrotate90                    \
#              --negYrotate180                   \
#              --negYrotate270                   \
#              --negYflipH                       \
#              --negYflipV                       \
#              --posZrotate90                    \
#              --posZrotate180                   \
#              --posZrotate270                   \
#              --posZflipH                       \
#              --posZflipV                       \
#              --negZrotate90                    \
#              --negZrotate180                   \
#              --negZrotate270                   \
#              --negZflipH                       \
#              --negZflipV                       \
#              ::Output                          \
#              --outputNum 1                     \
#              --output0 "okretnica_dds"         \
#              --output0params dds,bgra8,cubemap \

# Cmft with all parameters listed. This is mainly to have a look at what is all possible.
#eval $CMFT $@ --input "okretnica.tga"       \
#              ::Filter options              \
#              --filter radiance             \
#              --srcFaceSize 256             \
#              --excludeBase false           \
#              --mipCount 7                  \
#              --glossScale 10               \
#              --glossBias 3                 \
#              --lightingModel blinnbrdf     \
#              --edgeFixup none              \
#              --dstFaceSize 256             \
#              ::Processing devices          \
#              --numCpuProcessingThreads 4   \
#              --useOpenCL true              \
#              --clVendor anyGpuVendor       \
#              --deviceType gpu              \
#              --deviceIndex 0               \
#              ::Aditional operations        \
#              --inputGamma 1.0              \
#              --inputGammaDenominator 1.0   \
#              --outputGamma 1.0             \
#              --outputGammaDenominator 1.0  \
#              --generateMipChain false      \
#              ::Cubemap transformations     \
#              --posXrotate90                \
#              --posXrotate180               \
#              --posXrotate270               \
#              --posXflipH                   \
#              --posXflipV                   \
#              --negXrotate90                \
#              --negXrotate180               \
#              --negXrotate270               \
#              --negXflipH                   \
#              --negXflipV                   \
#              --posYrotate90                \
#              --posYrotate180               \
#              --posYrotate270               \
#              --posYflipH                   \
#              --posYflipV                   \
#              --negYrotate90                \
#              --negYrotate180               \
#              --negYrotate270               \
#              --negYflipH                   \
#              --negYflipV                   \
#              --posZrotate90                \
#              --posZrotate180               \
#              --posZrotate270               \
#              --posZflipH                   \
#              --posZflipV                   \
#              --negZrotate90                \
#              --negZrotate180               \
#              --negZrotate270               \
#              --negZflipH                   \
#              --negZflipV                   \
#              ::Output                      \
#              --outputNum 5                 \
#              --output0 "cmft_cubemap"    --output0params dds,bgra8,cubemap  \
#              --output1 "cmft_hstrip"     --output1params dds,bgra8,hstrip   \
#              --output2 "cmft_cubecross"  --output2params ktx,rgba32f,hcross \
#              --output3 "cmft_facelist"   --output3params tga,bgra8,facelist \
#              --output4 "cmft_latlong"    --output4params hdr,rgbe,latlong

# eval $CMFT $@ --input "shrine.hdr"          \
#              --filter none                     \
#              ::Output                          \
#              --outputNum 1                     \
#              --dstFaceSize 0 \
#              --output0 "test"         \
#              --output0params hdr,rgbe,latlong