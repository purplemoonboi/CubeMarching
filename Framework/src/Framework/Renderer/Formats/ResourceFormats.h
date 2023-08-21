#pragma once


namespace Foundation::Graphics
{

#ifdef CM_WINDOWS_PLATFORM
#include "dxgiFormat.h"
    enum ResourceFormat
    {
        Foundation_Format_UNKNOWN = 0,
        Foundation_Format_R32G32B32A32_TYPELESS = 1,
        Foundation_Format_R32G32B32A32_FLOAT = 2,
        Foundation_Format_R32G32B32A32_UINT = 3,
        Foundation_Format_R32G32B32A32_SINT = 4,
        Foundation_Format_R32G32B32_TYPELESS = 5,
        Foundation_Format_R32G32B32_FLOAT = 6,
        Foundation_Format_R32G32B32_UINT = 7,
        Foundation_Format_R32G32B32_SINT = 8,
        Foundation_Format_R16G16B16A16_TYPELESS = 9,
        Foundation_Format_R16G16B16A16_FLOAT = 10,
        Foundation_Format_R16G16B16A16_UNORM = 11,
        Foundation_Format_R16G16B16A16_UINT = 12,
        Foundation_Format_R16G16B16A16_SNORM = 13,
        Foundation_Format_R16G16B16A16_SINT = 14,
        Foundation_Format_R32G32_TYPELESS = 15,
        Foundation_Format_R32G32_FLOAT = 16,
        Foundation_Format_R32G32_UINT = 17,
        Foundation_Format_R32G32_SINT = 18,
        Foundation_Format_R32G8X24_TYPELESS = 19,
        Foundation_Format_D32_FLOAT_S8X24_UINT = 20,
        Foundation_Format_R32_FLOAT_X8X24_TYPELESS = 21,
        Foundation_Format_X32_TYPELESS_G8X24_UINT = 22,
        Foundation_Format_R10G10B10A2_TYPELESS = 23,
        Foundation_Format_R10G10B10A2_UNORM = 24,
        Foundation_Format_R10G10B10A2_UINT = 25,
        Foundation_Format_R11G11B10_FLOAT = 26,
        Foundation_Format_R8G8B8A8_TYPELESS = 27,
        Foundation_Format_R8G8B8A8_UNORM = 28,
        Foundation_Format_R8G8B8A8_UNORM_SRGB = 29,
        Foundation_Format_R8G8B8A8_UINT = 30,
        Foundation_Format_R8G8B8A8_SNORM = 31,
        Foundation_Format_R8G8B8A8_SINT = 32,
        Foundation_Format_R16G16_TYPELESS = 33,
        Foundation_Format_R16G16_FLOAT = 34,
        Foundation_Format_R16G16_UNORM = 35,
        Foundation_Format_R16G16_UINT = 36,
        Foundation_Format_R16G16_SNORM = 37,
        Foundation_Format_R16G16_SINT = 38,
        Foundation_Format_R32_TYPELESS = 39,
        Foundation_Format_D32_FLOAT = 40,
        Foundation_Format_R32_FLOAT = 41,
        Foundation_Format_R32_UINT = 42,
        Foundation_Format_R32_SINT = 43,
        Foundation_Format_R24G8_TYPELESS = 44,
        Foundation_Format_D24_UNORM_S8_UINT = 45,
        Foundation_Format_R24_UNORM_X8_TYPELESS = 46,
        Foundation_Format_X24_TYPELESS_G8_UINT = 47,
        Foundation_Format_R8G8_TYPELESS = 48,
        Foundation_Format_R8G8_UNORM = 49,
        Foundation_Format_R8G8_UINT = 50,
        Foundation_Format_R8G8_SNORM = 51,
        Foundation_Format_R8G8_SINT = 52,
        Foundation_Format_R16_TYPELESS = 53,
        Foundation_Format_R16_FLOAT = 54,
        Foundation_Format_D16_UNORM = 55,
        Foundation_Format_R16_UNORM = 56,
        Foundation_Format_R16_UINT = 57,
        Foundation_Format_R16_SNORM = 58,
        Foundation_Format_R16_SINT = 59,
        Foundation_Format_R8_TYPELESS = 60,
        Foundation_Format_R8_UNORM = 61,
        Foundation_Format_R8_UINT = 62,
        Foundation_Format_R8_SNORM = 63,
        Foundation_Format_R8_SINT = 64,
        Foundation_Format_A8_UNORM = 65,
        Foundation_Format_R1_UNORM = 66,
        Foundation_Format_R9G9B9E5_SHAREDEXP = 67,
        Foundation_Format_R8G8_B8G8_UNORM = 68,
        Foundation_Format_G8R8_G8B8_UNORM = 69,
        Foundation_Format_BC1_TYPELESS = 70,
        Foundation_Format_BC1_UNORM = 71,
        Foundation_Format_BC1_UNORM_SRGB = 72,
        Foundation_Format_BC2_TYPELESS = 73,
        Foundation_Format_BC2_UNORM = 74,
        Foundation_Format_BC2_UNORM_SRGB = 75,
        Foundation_Format_BC3_TYPELESS = 76,
        Foundation_Format_BC3_UNORM = 77,
        Foundation_Format_BC3_UNORM_SRGB = 78,
        Foundation_Format_BC4_TYPELESS = 79,
        Foundation_Format_BC4_UNORM = 80,
        Foundation_Format_BC4_SNORM = 81,
        Foundation_Format_BC5_TYPELESS = 82,
        Foundation_Format_BC5_UNORM = 83,
        Foundation_Format_BC5_SNORM = 84,
        Foundation_Format_B5G6R5_UNORM = 85,
        Foundation_Format_B5G5R5A1_UNORM = 86,
        Foundation_Format_B8G8R8A8_UNORM = 87,
        Foundation_Format_B8G8R8X8_UNORM = 88,
        Foundation_Format_R10G10B10_XR_BIAS_A2_UNORM = 89,
        Foundation_Format_B8G8R8A8_TYPELESS = 90,
        Foundation_Format_B8G8R8A8_UNORM_SRGB = 91,
        Foundation_Format_B8G8R8X8_TYPELESS = 92,
        Foundation_Format_B8G8R8X8_UNORM_SRGB = 93,
        Foundation_Format_BC6H_TYPELESS = 94,
        Foundation_Format_BC6H_UF16 = 95,
        Foundation_Format_BC6H_SF16 = 96,
        Foundation_Format_BC7_TYPELESS = 97,
        Foundation_Format_BC7_UNORM = 98,
        Foundation_Format_BC7_UNORM_SRGB = 99,
        Foundation_Format_AYUV = 100,
        Foundation_Format_Y410 = 101,
        Foundation_Format_Y416 = 102,
        Foundation_Format_NV12 = 103,
        Foundation_Format_P010 = 104,
        Foundation_Format_P016 = 105,
        Foundation_Format_420_OPAQUE = 106,
        Foundation_Format_YUY2 = 107,
        Foundation_Format_Y210 = 108,
        Foundation_Format_Y216 = 109,
        Foundation_Format_NV11 = 110,
        Foundation_Format_AI44 = 111,
        Foundation_Format_IA44 = 112,
        Foundation_Format_P8 = 113,
        Foundation_Format_A8P8 = 114,
        Foundation_Format_B4G4R4A4_UNORM = 115,

        Foundation_Format_P208 = 130,
        Foundation_Format_V208 = 131,
        Foundation_Format_V408 = 132,


        Foundation_Format_Sampler_Feedback_Min_Mip_Opaque = 189,
        Foundation_Format_Sampler_Feedback_Mip_Region_Used_Opaque = 190,


        Foundation_Format_FORCE_UINT = 0xffffffff
    };

    DXGI_FORMAT FoundationResourceFormatToDX12(ResourceFormat format)
    {
	    switch (format)
	    {
            case Foundation_Format_UNKNOWN                       : return DXGI_FORMAT_UNKNOWN;
            case Foundation_Format_R32G32B32A32_TYPELESS         : return DXGI_FORMAT_R32G32B32A32_TYPELESS;
            case Foundation_Format_R32G32B32A32_FLOAT            : return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case Foundation_Format_R32G32B32A32_UINT             : return DXGI_FORMAT_R32G32B32A32_UINT;
            case Foundation_Format_R32G32B32A32_SINT             : return DXGI_FORMAT_R32G32B32A32_SINT;
            case Foundation_Format_R32G32B32_TYPELESS            : return DXGI_FORMAT_R32G32B32_TYPELESS;
            case Foundation_Format_R32G32B32_FLOAT               : return DXGI_FORMAT_R32G32B32_FLOAT;
            case Foundation_Format_R32G32B32_UINT                : return DXGI_FORMAT_R32G32B32_UINT;
            case Foundation_Format_R32G32B32_SINT                : return DXGI_FORMAT_R32G32B32_SINT;
            case Foundation_Format_R16G16B16A16_TYPELESS         : return DXGI_FORMAT_R16G16B16A16_TYPELESS;
            case Foundation_Format_R16G16B16A16_FLOAT            : return DXGI_FORMAT_R16G16B16A16_FLOAT;
            case Foundation_Format_R16G16B16A16_UNORM            : return DXGI_FORMAT_R16G16B16A16_UNORM;
            case Foundation_Format_R16G16B16A16_UINT             : return DXGI_FORMAT_R16G16B16A16_UINT;
            case Foundation_Format_R16G16B16A16_SNORM            : return DXGI_FORMAT_R16G16B16A16_SNORM;
            case Foundation_Format_R16G16B16A16_SINT             : return DXGI_FORMAT_R16G16B16A16_SINT;
            case Foundation_Format_R32G32_TYPELESS               : return DXGI_FORMAT_R32G32_TYPELESS;
            case Foundation_Format_R32G32_FLOAT                  : return DXGI_FORMAT_R32G32_FLOAT;
            case Foundation_Format_R32G32_UINT                   : return DXGI_FORMAT_R32G32_UINT;
            case Foundation_Format_R32G32_SINT                   : return DXGI_FORMAT_R32G32_SINT;
            case Foundation_Format_R32G8X24_TYPELESS             : return DXGI_FORMAT_R32G8X24_TYPELESS;
            case Foundation_Format_D32_FLOAT_S8X24_UINT          : return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
            case Foundation_Format_R32_FLOAT_X8X24_TYPELESS      : return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
            case Foundation_Format_X32_TYPELESS_G8X24_UINT       : return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
            case Foundation_Format_R10G10B10A2_TYPELESS          : return DXGI_FORMAT_R10G10B10A2_TYPELESS;
            case Foundation_Format_R10G10B10A2_UNORM             : return DXGI_FORMAT_R10G10B10A2_UNORM;
            case Foundation_Format_R10G10B10A2_UINT              : return DXGI_FORMAT_R10G10B10A2_UINT;
            case Foundation_Format_R11G11B10_FLOAT               : return DXGI_FORMAT_R11G11B10_FLOAT;
            case Foundation_Format_R8G8B8A8_TYPELESS             : return DXGI_FORMAT_R8G8B8A8_TYPELESS;
            case Foundation_Format_R8G8B8A8_UNORM                : return DXGI_FORMAT_R8G8B8A8_UNORM;
            case Foundation_Format_R8G8B8A8_UNORM_SRGB           : return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            case Foundation_Format_R8G8B8A8_UINT                 : return DXGI_FORMAT_R8G8B8A8_UINT;
            case Foundation_Format_R8G8B8A8_SNORM                : return DXGI_FORMAT_R8G8B8A8_SNORM;
            case Foundation_Format_R8G8B8A8_SINT                 : return DXGI_FORMAT_R8G8B8A8_SINT;
            case Foundation_Format_R16G16_TYPELESS               : return DXGI_FORMAT_R16G16_TYPELESS;
            case Foundation_Format_R16G16_FLOAT                  : return DXGI_FORMAT_R16G16_FLOAT;
            case Foundation_Format_R16G16_UNORM                  : return DXGI_FORMAT_R16G16_UNORM;
            case Foundation_Format_R16G16_UINT                   : return DXGI_FORMAT_R16G16_UINT;
            case Foundation_Format_R16G16_SNORM                  : return DXGI_FORMAT_R16G16_SNORM;
            case Foundation_Format_R16G16_SINT                   : return DXGI_FORMAT_R16G16_SINT;
            case Foundation_Format_R32_TYPELESS                  : return DXGI_FORMAT_R32_TYPELESS;
            case Foundation_Format_D32_FLOAT                     : return DXGI_FORMAT_D32_FLOAT;
            case Foundation_Format_R32_FLOAT                     : return DXGI_FORMAT_R32_FLOAT;
            case Foundation_Format_R32_UINT                      : return DXGI_FORMAT_R32_UINT;
            case Foundation_Format_R32_SINT                      : return DXGI_FORMAT_R32_SINT;
            case Foundation_Format_R24G8_TYPELESS                : return DXGI_FORMAT_R24G8_TYPELESS;
            case Foundation_Format_D24_UNORM_S8_UINT             : return DXGI_FORMAT_D24_UNORM_S8_UINT;
            case Foundation_Format_R24_UNORM_X8_TYPELESS         : return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
            case Foundation_Format_X24_TYPELESS_G8_UINT          : return DXGI_FORMAT_X24_TYPELESS_G8_UINT;
            case Foundation_Format_R8G8_TYPELESS                 : return DXGI_FORMAT_R8G8_TYPELESS;
            case Foundation_Format_R8G8_UNORM                    : return DXGI_FORMAT_R8G8_UNORM;
            case Foundation_Format_R8G8_UINT                     : return DXGI_FORMAT_R8G8_UINT;
            case Foundation_Format_R8G8_SNORM                    : return DXGI_FORMAT_R8G8_SNORM;
            case Foundation_Format_R8G8_SINT                     : return DXGI_FORMAT_R8G8_SINT;
            case Foundation_Format_R16_TYPELESS                  : return DXGI_FORMAT_R16_TYPELESS;
            case Foundation_Format_R16_FLOAT                     : return DXGI_FORMAT_R16_FLOAT;
            case Foundation_Format_D16_UNORM                     : return DXGI_FORMAT_D16_UNORM;
            case Foundation_Format_R16_UNORM                     : return DXGI_FORMAT_R16_UNORM;
            case Foundation_Format_R16_UINT                      : return DXGI_FORMAT_R16_UINT;
            case Foundation_Format_R16_SNORM                     : return DXGI_FORMAT_R16_SNORM;
            case Foundation_Format_R16_SINT                      : return DXGI_FORMAT_R16_SINT;
            case Foundation_Format_R8_TYPELESS                   : return DXGI_FORMAT_R8_TYPELESS;
            case Foundation_Format_R8_UNORM                      : return DXGI_FORMAT_R8_UNORM;
            case Foundation_Format_R8_UINT                       : return DXGI_FORMAT_R8_UINT;
            case Foundation_Format_R8_SNORM                      : return DXGI_FORMAT_R8_SNORM;
            case Foundation_Format_R8_SINT                       : return DXGI_FORMAT_R8_SINT;
            case Foundation_Format_A8_UNORM                      : return DXGI_FORMAT_A8_UNORM;
            case Foundation_Format_R1_UNORM                      : return DXGI_FORMAT_R1_UNORM;
            case Foundation_Format_R9G9B9E5_SHAREDEXP            : return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
            case Foundation_Format_R8G8_B8G8_UNORM               : return DXGI_FORMAT_R8G8_B8G8_UNORM;
            case Foundation_Format_G8R8_G8B8_UNORM               : return DXGI_FORMAT_G8R8_G8B8_UNORM;
            case Foundation_Format_BC1_TYPELESS                  : return DXGI_FORMAT_BC1_TYPELESS;
            case Foundation_Format_BC1_UNORM                     : return DXGI_FORMAT_BC1_UNORM;
            case Foundation_Format_BC1_UNORM_SRGB                : return DXGI_FORMAT_BC1_UNORM_SRGB;
            case Foundation_Format_BC2_TYPELESS                  : return DXGI_FORMAT_BC2_TYPELESS;
            case Foundation_Format_BC2_UNORM                     : return DXGI_FORMAT_BC2_UNORM;
            case Foundation_Format_BC2_UNORM_SRGB                : return DXGI_FORMAT_BC2_UNORM_SRGB;
            case Foundation_Format_BC3_TYPELESS                  : return DXGI_FORMAT_BC3_TYPELESS;
            case Foundation_Format_BC3_UNORM                     : return DXGI_FORMAT_BC3_UNORM;
            case Foundation_Format_BC3_UNORM_SRGB                : return DXGI_FORMAT_BC3_UNORM_SRGB;
            case Foundation_Format_BC4_TYPELESS                  : return DXGI_FORMAT_BC4_TYPELESS;
            case Foundation_Format_BC4_UNORM                     : return DXGI_FORMAT_BC4_UNORM;
            case Foundation_Format_BC4_SNORM                     : return DXGI_FORMAT_BC4_SNORM;
            case Foundation_Format_BC5_TYPELESS                  : return DXGI_FORMAT_BC5_TYPELESS;
            case Foundation_Format_BC5_UNORM                     : return DXGI_FORMAT_BC5_UNORM;
            case Foundation_Format_BC5_SNORM                     : return DXGI_FORMAT_BC5_SNORM;
            case Foundation_Format_B5G6R5_UNORM                  : return DXGI_FORMAT_B5G6R5_UNORM;
            case Foundation_Format_B5G5R5A1_UNORM                : return DXGI_FORMAT_B5G5R5A1_UNORM;
            case Foundation_Format_B8G8R8A8_UNORM                : return DXGI_FORMAT_B8G8R8A8_UNORM;
            case Foundation_Format_B8G8R8X8_UNORM                : return DXGI_FORMAT_B8G8R8X8_UNORM;
            case Foundation_Format_R10G10B10_XR_BIAS_A2_UNORM    : return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;
            case Foundation_Format_B8G8R8A8_TYPELESS             : return DXGI_FORMAT_B8G8R8A8_TYPELESS;
            case Foundation_Format_B8G8R8A8_UNORM_SRGB           : return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
            case Foundation_Format_B8G8R8X8_TYPELESS             : return DXGI_FORMAT_B8G8R8X8_TYPELESS;
            case Foundation_Format_B8G8R8X8_UNORM_SRGB           : return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
            case Foundation_Format_BC6H_TYPELESS                 : return DXGI_FORMAT_BC6H_TYPELESS;
            case Foundation_Format_BC6H_UF16                     : return DXGI_FORMAT_BC6H_UF16;
            case Foundation_Format_BC6H_SF16                     : return DXGI_FORMAT_BC6H_SF16;
            case Foundation_Format_BC7_TYPELESS                  : return DXGI_FORMAT_BC7_TYPELESS;
            case Foundation_Format_BC7_UNORM                     : return DXGI_FORMAT_BC7_UNORM;
            case Foundation_Format_BC7_UNORM_SRGB                : return DXGI_FORMAT_BC7_UNORM_SRGB;
            case Foundation_Format_AYUV                          : return DXGI_FORMAT_AYUV;
            case Foundation_Format_Y410                          : return DXGI_FORMAT_Y410;
            case Foundation_Format_Y416                          : return DXGI_FORMAT_Y416;
            case Foundation_Format_NV12                          : return DXGI_FORMAT_NV12;
            case Foundation_Format_P010                          : return DXGI_FORMAT_P010;
            case Foundation_Format_P016                          : return DXGI_FORMAT_P016;
            case Foundation_Format_420_OPAQUE                    : return DXGI_FORMAT_420_OPAQUE;
            case Foundation_Format_YUY2                          : return DXGI_FORMAT_YUY2;
            case Foundation_Format_Y210                          : return DXGI_FORMAT_Y210;
            case Foundation_Format_Y216                          : return DXGI_FORMAT_Y216;
            case Foundation_Format_NV11                          : return DXGI_FORMAT_NV11;
            case Foundation_Format_AI44                          : return DXGI_FORMAT_AI44;
            case Foundation_Format_IA44                          : return DXGI_FORMAT_IA44;
            case Foundation_Format_P8                            : return DXGI_FORMAT_P8;
            case Foundation_Format_A8P8                          : return DXGI_FORMAT_A8P8;
            case Foundation_Format_B4G4R4A4_UNORM                : return DXGI_FORMAT_B4G4R4A4_UNORM;
            
            case Foundation_Format_P208                          : return DXGI_FORMAT_P208;
            case Foundation_Format_V208                          : return DXGI_FORMAT_V208;
            case Foundation_Format_V408                          : return DXGI_FORMAT_V408;
         
            case Foundation_Format_Sampler_Feedback_Min_Mip_Opaque: return DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE;
            case Foundation_Format_Sampler_Feedback_Mip_Region_Used_Opaque: return DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE;

			case Foundation_Format_FORCE_UINT: return DXGI_FORMAT_FORCE_UINT;

                default: return DXGI_FORMAT_UNKNOWN;
        }
    }

#elif VULKAN_PLATFORM

#endif

}
