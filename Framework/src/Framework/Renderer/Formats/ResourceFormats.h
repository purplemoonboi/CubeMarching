#pragma once


namespace Foundation::Graphics
{

#ifdef CM_WINDOWS_PLATFORM
#include "dxgiFormat.h"
    enum ResourceFormat
    {
       UNKNOWN = 0,
       R32G32B32A32_TYPELESS = 1,
       R32G32B32A32_FLOAT = 2,
       R32G32B32A32_UINT = 3,
       R32G32B32A32_SINT = 4,
       R32G32B32_TYPELESS = 5,
       R32G32B32_FLOAT = 6,
       R32G32B32_UINT = 7,
       R32G32B32_SINT = 8,
       R16G16B16A16_TYPELESS = 9,
       R16G16B16A16_FLOAT = 10,
       R16G16B16A16_UNORM = 11,
       R16G16B16A16_UINT = 12,
       R16G16B16A16_SNORM = 13,
       R16G16B16A16_SINT = 14,
       R32G32_TYPELESS = 15,
       R32G32_FLOAT = 16,
       R32G32_UINT = 17,
       R32G32_SINT = 18,
       R32G8X24_TYPELESS = 19,
       D32_FLOAT_S8X24_UINT = 20,
       R32_FLOAT_X8X24_TYPELESS = 21,
       X32_TYPELESS_G8X24_UINT = 22,
       R10G10B10A2_TYPELESS = 23,
       R10G10B10A2_UNORM = 24,
       R10G10B10A2_UINT = 25,
       R11G11B10_FLOAT = 26,
       R8G8B8A8_TYPELESS = 27,
       R8G8B8A8_UNORM = 28,
       R8G8B8A8_UNORM_SRGB = 29,
       R8G8B8A8_UINT = 30,
       R8G8B8A8_SNORM = 31,
       R8G8B8A8_SINT = 32,
       R16G16_TYPELESS = 33,
       R16G16_FLOAT = 34,
       R16G16_UNORM = 35,
       R16G16_UINT = 36,
       R16G16_SNORM = 37,
       R16G16_SINT = 38,
       R32_TYPELESS = 39,
       D32_FLOAT = 40,
       R32_FLOAT = 41,
       R32_UINT = 42,
       R32_SINT = 43,
       R24G8_TYPELESS = 44,
       D24_UNORM_S8_UINT = 45,
       R24_UNORM_X8_TYPELESS = 46,
       X24_TYPELESS_G8_UINT = 47,
       R8G8_TYPELESS = 48,
       R8G8_UNORM = 49,
       R8G8_UINT = 50,
       R8G8_SNORM = 51,
       R8G8_SINT = 52,
       R16_TYPELESS = 53,
       R16_FLOAT = 54,
       D16_UNORM = 55,
       R16_UNORM = 56,
       R16_UINT = 57,
       R16_SNORM = 58,
       R16_SINT = 59,
       R8_TYPELESS = 60,
       R8_UNORM = 61,
       R8_UINT = 62,
       R8_SNORM = 63,
       R8_SINT = 64,
       A8_UNORM = 65,
       R1_UNORM = 66,
       R9G9B9E5_SHAREDEXP = 67,
       R8G8_B8G8_UNORM = 68,
       G8R8_G8B8_UNORM = 69,
       BC1_TYPELESS = 70,
       BC1_UNORM = 71,
       BC1_UNORM_SRGB = 72,
       BC2_TYPELESS = 73,
       BC2_UNORM = 74,
       BC2_UNORM_SRGB = 75,
       BC3_TYPELESS = 76,
       BC3_UNORM = 77,
       BC3_UNORM_SRGB = 78,
       BC4_TYPELESS = 79,
       BC4_UNORM = 80,
       BC4_SNORM = 81,
       BC5_TYPELESS = 82,
       BC5_UNORM = 83,
       BC5_SNORM = 84,
       B5G6R5_UNORM = 85,
       B5G5R5A1_UNORM = 86,
       B8G8R8A8_UNORM = 87,
       B8G8R8X8_UNORM = 88,
       R10G10B10_XR_BIAS_A2_UNORM = 89,
       B8G8R8A8_TYPELESS = 90,
       B8G8R8A8_UNORM_SRGB = 91,
       B8G8R8X8_TYPELESS = 92,
       B8G8R8X8_UNORM_SRGB = 93,
       BC6H_TYPELESS = 94,
       BC6H_UF16 = 95,
       BC6H_SF16 = 96,
       BC7_TYPELESS = 97,
       BC7_UNORM = 98,
       BC7_UNORM_SRGB = 99,
       AYUV = 100,
       Y410 = 101,
       Y416 = 102,
       NV12 = 103,
       P010 = 104,
       P016 = 105,
       OPAQUE_420 = 106,
       YUY2 = 107,
       Y210 = 108,
       Y216 = 109,
       NV11 = 110,
       AI44 = 111,
       IA44 = 112,
       P8 = 113,
       A8P8 = 114,
       B4G4R4A4_UNORM = 115,

       P208 = 130,
       V208 = 131,
       V408 = 132,


       Sampler_Feedback_Min_Mip_Opaque = 189,
       Sampler_Feedback_Mip_Region_Used_Opaque = 190,


       FORCE_UINT = 0xffffffff
    };

    [[nodiscard]] static __inline DXGI_FORMAT FoundationResourceFormatToDX12(ResourceFormat format)
    {
	    switch (format)
	    {
            case UNKNOWN                       : return DXGI_FORMAT_UNKNOWN;
            case R32G32B32A32_TYPELESS         : return DXGI_FORMAT_R32G32B32A32_TYPELESS;
            case R32G32B32A32_FLOAT            : return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case R32G32B32A32_UINT             : return DXGI_FORMAT_R32G32B32A32_UINT;
            case R32G32B32A32_SINT             : return DXGI_FORMAT_R32G32B32A32_SINT;
            case R32G32B32_TYPELESS            : return DXGI_FORMAT_R32G32B32_TYPELESS;
            case R32G32B32_FLOAT               : return DXGI_FORMAT_R32G32B32_FLOAT;
            case R32G32B32_UINT                : return DXGI_FORMAT_R32G32B32_UINT;
            case R32G32B32_SINT                : return DXGI_FORMAT_R32G32B32_SINT;
            case R16G16B16A16_TYPELESS         : return DXGI_FORMAT_R16G16B16A16_TYPELESS;
            case R16G16B16A16_FLOAT            : return DXGI_FORMAT_R16G16B16A16_FLOAT;
            case R16G16B16A16_UNORM            : return DXGI_FORMAT_R16G16B16A16_UNORM;
            case R16G16B16A16_UINT             : return DXGI_FORMAT_R16G16B16A16_UINT;
            case R16G16B16A16_SNORM            : return DXGI_FORMAT_R16G16B16A16_SNORM;
            case R16G16B16A16_SINT             : return DXGI_FORMAT_R16G16B16A16_SINT;
            case R32G32_TYPELESS               : return DXGI_FORMAT_R32G32_TYPELESS;
            case R32G32_FLOAT                  : return DXGI_FORMAT_R32G32_FLOAT;
            case R32G32_UINT                   : return DXGI_FORMAT_R32G32_UINT;
            case R32G32_SINT                   : return DXGI_FORMAT_R32G32_SINT;
            case R32G8X24_TYPELESS             : return DXGI_FORMAT_R32G8X24_TYPELESS;
            case D32_FLOAT_S8X24_UINT          : return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
            case R32_FLOAT_X8X24_TYPELESS      : return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
            case X32_TYPELESS_G8X24_UINT       : return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
            case R10G10B10A2_TYPELESS          : return DXGI_FORMAT_R10G10B10A2_TYPELESS;
            case R10G10B10A2_UNORM             : return DXGI_FORMAT_R10G10B10A2_UNORM;
            case R10G10B10A2_UINT              : return DXGI_FORMAT_R10G10B10A2_UINT;
            case R11G11B10_FLOAT               : return DXGI_FORMAT_R11G11B10_FLOAT;
            case R8G8B8A8_TYPELESS             : return DXGI_FORMAT_R8G8B8A8_TYPELESS;
            case R8G8B8A8_UNORM                : return DXGI_FORMAT_R8G8B8A8_UNORM;
            case R8G8B8A8_UNORM_SRGB           : return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            case R8G8B8A8_UINT                 : return DXGI_FORMAT_R8G8B8A8_UINT;
            case R8G8B8A8_SNORM                : return DXGI_FORMAT_R8G8B8A8_SNORM;
            case R8G8B8A8_SINT                 : return DXGI_FORMAT_R8G8B8A8_SINT;
            case R16G16_TYPELESS               : return DXGI_FORMAT_R16G16_TYPELESS;
            case R16G16_FLOAT                  : return DXGI_FORMAT_R16G16_FLOAT;
            case R16G16_UNORM                  : return DXGI_FORMAT_R16G16_UNORM;
            case R16G16_UINT                   : return DXGI_FORMAT_R16G16_UINT;
            case R16G16_SNORM                  : return DXGI_FORMAT_R16G16_SNORM;
            case R16G16_SINT                   : return DXGI_FORMAT_R16G16_SINT;
            case R32_TYPELESS                  : return DXGI_FORMAT_R32_TYPELESS;
            case D32_FLOAT                     : return DXGI_FORMAT_D32_FLOAT;
            case R32_FLOAT                     : return DXGI_FORMAT_R32_FLOAT;
            case R32_UINT                      : return DXGI_FORMAT_R32_UINT;
            case R32_SINT                      : return DXGI_FORMAT_R32_SINT;
            case R24G8_TYPELESS                : return DXGI_FORMAT_R24G8_TYPELESS;
            case D24_UNORM_S8_UINT             : return DXGI_FORMAT_D24_UNORM_S8_UINT;
            case R24_UNORM_X8_TYPELESS         : return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
            case X24_TYPELESS_G8_UINT          : return DXGI_FORMAT_X24_TYPELESS_G8_UINT;
            case R8G8_TYPELESS                 : return DXGI_FORMAT_R8G8_TYPELESS;
            case R8G8_UNORM                    : return DXGI_FORMAT_R8G8_UNORM;
            case R8G8_UINT                     : return DXGI_FORMAT_R8G8_UINT;
            case R8G8_SNORM                    : return DXGI_FORMAT_R8G8_SNORM;
            case R8G8_SINT                     : return DXGI_FORMAT_R8G8_SINT;
            case R16_TYPELESS                  : return DXGI_FORMAT_R16_TYPELESS;
            case R16_FLOAT                     : return DXGI_FORMAT_R16_FLOAT;
            case D16_UNORM                     : return DXGI_FORMAT_D16_UNORM;
            case R16_UNORM                     : return DXGI_FORMAT_R16_UNORM;
            case R16_UINT                      : return DXGI_FORMAT_R16_UINT;
            case R16_SNORM                     : return DXGI_FORMAT_R16_SNORM;
            case R16_SINT                      : return DXGI_FORMAT_R16_SINT;
            case R8_TYPELESS                   : return DXGI_FORMAT_R8_TYPELESS;
            case R8_UNORM                      : return DXGI_FORMAT_R8_UNORM;
            case R8_UINT                       : return DXGI_FORMAT_R8_UINT;
            case R8_SNORM                      : return DXGI_FORMAT_R8_SNORM;
            case R8_SINT                       : return DXGI_FORMAT_R8_SINT;
            case A8_UNORM                      : return DXGI_FORMAT_A8_UNORM;
            case R1_UNORM                      : return DXGI_FORMAT_R1_UNORM;
            case R9G9B9E5_SHAREDEXP            : return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
            case R8G8_B8G8_UNORM               : return DXGI_FORMAT_R8G8_B8G8_UNORM;
            case G8R8_G8B8_UNORM               : return DXGI_FORMAT_G8R8_G8B8_UNORM;
            case BC1_TYPELESS                  : return DXGI_FORMAT_BC1_TYPELESS;
            case BC1_UNORM                     : return DXGI_FORMAT_BC1_UNORM;
            case BC1_UNORM_SRGB                : return DXGI_FORMAT_BC1_UNORM_SRGB;
            case BC2_TYPELESS                  : return DXGI_FORMAT_BC2_TYPELESS;
            case BC2_UNORM                     : return DXGI_FORMAT_BC2_UNORM;
            case BC2_UNORM_SRGB                : return DXGI_FORMAT_BC2_UNORM_SRGB;
            case BC3_TYPELESS                  : return DXGI_FORMAT_BC3_TYPELESS;
            case BC3_UNORM                     : return DXGI_FORMAT_BC3_UNORM;
            case BC3_UNORM_SRGB                : return DXGI_FORMAT_BC3_UNORM_SRGB;
            case BC4_TYPELESS                  : return DXGI_FORMAT_BC4_TYPELESS;
            case BC4_UNORM                     : return DXGI_FORMAT_BC4_UNORM;
            case BC4_SNORM                     : return DXGI_FORMAT_BC4_SNORM;
            case BC5_TYPELESS                  : return DXGI_FORMAT_BC5_TYPELESS;
            case BC5_UNORM                     : return DXGI_FORMAT_BC5_UNORM;
            case BC5_SNORM                     : return DXGI_FORMAT_BC5_SNORM;
            case B5G6R5_UNORM                  : return DXGI_FORMAT_B5G6R5_UNORM;
            case B5G5R5A1_UNORM                : return DXGI_FORMAT_B5G5R5A1_UNORM;
            case B8G8R8A8_UNORM                : return DXGI_FORMAT_B8G8R8A8_UNORM;
            case B8G8R8X8_UNORM                : return DXGI_FORMAT_B8G8R8X8_UNORM;
            case R10G10B10_XR_BIAS_A2_UNORM    : return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;
            case B8G8R8A8_TYPELESS             : return DXGI_FORMAT_B8G8R8A8_TYPELESS;
            case B8G8R8A8_UNORM_SRGB           : return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
            case B8G8R8X8_TYPELESS             : return DXGI_FORMAT_B8G8R8X8_TYPELESS;
            case B8G8R8X8_UNORM_SRGB           : return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
            case BC6H_TYPELESS                 : return DXGI_FORMAT_BC6H_TYPELESS;
            case BC6H_UF16                     : return DXGI_FORMAT_BC6H_UF16;
            case BC6H_SF16                     : return DXGI_FORMAT_BC6H_SF16;
            case BC7_TYPELESS                  : return DXGI_FORMAT_BC7_TYPELESS;
            case BC7_UNORM                     : return DXGI_FORMAT_BC7_UNORM;
            case BC7_UNORM_SRGB                : return DXGI_FORMAT_BC7_UNORM_SRGB;
            case AYUV                          : return DXGI_FORMAT_AYUV;
            case Y410                          : return DXGI_FORMAT_Y410;
            case Y416                          : return DXGI_FORMAT_Y416;
            case NV12                          : return DXGI_FORMAT_NV12;
            case P010                          : return DXGI_FORMAT_P010;
            case P016                          : return DXGI_FORMAT_P016;
            case OPAQUE_420                    : return DXGI_FORMAT_420_OPAQUE;
            case YUY2                          : return DXGI_FORMAT_YUY2;
            case Y210                          : return DXGI_FORMAT_Y210;
            case Y216                          : return DXGI_FORMAT_Y216;
            case NV11                          : return DXGI_FORMAT_NV11;
            case AI44                          : return DXGI_FORMAT_AI44;
            case IA44                          : return DXGI_FORMAT_IA44;
            case P8                            : return DXGI_FORMAT_P8;
            case A8P8                          : return DXGI_FORMAT_A8P8;
            case B4G4R4A4_UNORM                : return DXGI_FORMAT_B4G4R4A4_UNORM;
                 
            case P208                          : return DXGI_FORMAT_P208;
            case V208                          : return DXGI_FORMAT_V208;
            case V408                          : return DXGI_FORMAT_V408;
                 
            case Sampler_Feedback_Min_Mip_Opaque: return DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE;
            case Sampler_Feedback_Mip_Region_Used_Opaque: return DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE;
                 
			case FORCE_UINT: return DXGI_FORMAT_FORCE_UINT;

			default: return DXGI_FORMAT_UNKNOWN;
        }
    }

#elif VULKAN_PLATFORM

#endif

}
