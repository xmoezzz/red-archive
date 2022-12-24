#include "Init1stKey.h"
#include "NodeKeyDef.h"


BYTE BaseDecoder1_Key[]=
{
	0x27, 0x51, 0xed, 0x0b, 0x65, 0xce, 0x09,
	0x7c, 0x59, 0x34, 0x20, 0xbf, 0xf8, 0x2b, 0x10,
	0xb5, 0x2d, 0xed, 0xef, 0x14, 0x56, 0xc9, 0x84,
	0xe5, 0xc5, 0xa3, 0xcc, 0x04, 0x40, 0x71, 0x77,
	0xa2, 0x81, 0xc0, 0x68, 0xc6, 0xb6, 0x2d, 0x38,
	0xc8, 0x02, 0xed, 0xb3, 0xd5, 0xfa, 0x45, 0x59,
	0x6e, 0x29, 0x13, 0xdf, 0xe9, 0x8d, 0x43, 0xac,
	0xee, 0x18, 0x5a, 0x5c, 0xfb, 0x30, 0xef, 0x40,
	0xe1, 0x2f, 0x2d, 0xdc, 0x45, 0xe4, 0x53, 0x68,
	0x20, 0x10, 0x34, 0x4f, 0x3f, 0xe9, 0xb7, 0xb3,
	0xc3, 0x9a, 0x58, 0xe7, 0xa1, 0xc2, 0xa5, 0xf4,
	0x24, 0xf1, 0xc1, 0x15, 0x67, 0x2e, 0xe5, 0x3b,
	0xdb, 0x73, 0xda, 0x89, 0xc7, 0x2f, 0x80, 0xd9,
	0xc2, 0xc3, 0x4b, 0x35, 0x3b, 0x06, 0xc0, 0x5f,
	0xf2, 0xc1, 0xfc, 0x49, 0x7d, 0x33, 0x2e, 0x9e,
	0xc4, 0x8e, 0x18, 0x37, 0x85, 0x79, 0x91, 0xa7,
	0xd0, 0x8c, 0x06, 0xb0, 0x8c, 0xd7, 0xf5, 0xcb,
	0xf0, 0x5b, 0x70, 0xa4, 0x0b, 0x8f, 0xa0, 0x9b,
	0x3d, 0xdd, 0x40, 0x45, 0xbc, 0x22, 0x1e, 0xe8,
	0x10, 0x31, 0x9d, 0x03, 0x96, 0x51, 0x35, 0xc4,
	0x01, 0xbb, 0xf1, 0x90, 0xd4, 0x1d, 0xf0, 0x7e,
	0xea, 0x19, 0xe6, 0xdc, 0xee, 0xc6, 0x98, 0xa9,
	0xe3, 0x2e, 0x63, 0x19, 0x9e, 0xce, 0xb5, 0x14,
	0x47, 0x1a, 0x92, 0xb8, 0xdb, 0xf6, 0x10, 0xd2,
	0xad, 0x3e, 0xdc, 0x69, 0xe0, 0x3f, 0xb3, 0x33,
	0xee, 0x3c, 0xea, 0x1d, 0x25, 0xea, 0xe7, 0xc8,
	0x25, 0xf4, 0xa5, 0x07, 0x63, 0x78, 0x34, 0x62,
	0xa9, 0x87, 0x36, 0x96, 0x93, 0xa9, 0x63, 0x12,
	0x14, 0x57, 0x06, 0x7b, 0xef, 0x7f, 0x7e, 0x29,
	0x3e, 0x04, 0xbe, 0xa8, 0xef, 0x3b, 0xcd, 0x38,
	0x41, 0x6f, 0x46, 0x4e, 0x4c, 0x5e, 0xd9, 0x10,
	0x76, 0xba, 0xc9, 0xdd, 0xff, 0xa9, 0x6d, 0xc3,
	0x76, 0x45, 0xaf, 0x07, 0x41, 0x1d, 0x8f, 0xa0,
	0x19, 0xb2, 0xa0, 0xbc, 0x8c, 0xfa, 0x8a, 0x3a,
	0x79, 0xe0, 0x87, 0x2e, 0x97, 0xc2, 0xe7, 0x60,
	0xef, 0xf2, 0x8c, 0xcd, 0x5d, 0x36, 0x6d, 0x25,
	0x13, 0x49, 0x17, 0x4b, 0x16, 0x57, 0x28, 0xd9,
	0xbf, 0x84, 0xd3, 0x99, 0x3c, 0x66, 0x5f, 0x0d,
	0x0c, 0x86, 0xa7, 0xe7, 0x86, 0xe3, 0x9b, 0x91,
	0x53, 0x6f, 0xbd, 0xa7, 0xef, 0x91, 0xa6, 0x78,
	0x2c, 0xa1, 0x7f, 0x89, 0xaf, 0x6f, 0x88, 0x12,
	0x71, 0xbc, 0x94, 0x7f, 0x3f, 0xbf, 0x8a, 0xf1,
	0x3b, 0xa1, 0xe6, 0xba, 0x59, 0x02, 0x37, 0xe4,
	0xe2, 0x72, 0x9e, 0xaa, 0xf4, 0xf8, 0x55, 0xfd,
	0x00, 0x8f, 0x26, 0x01, 0x4b, 0xa4, 0xef, 0x8d,
	0x6e, 0x99, 0x25, 0xaf, 0xd6, 0x45, 0x4d, 0x26,
	0x44, 0x72, 0x85, 0xe6, 0x4e, 0x5d, 0xf9, 0x97,
	0xdc, 0x3a, 0x6f, 0x16, 0xac, 0xad, 0xbc, 0xf3,
	0xcf, 0x52, 0x4c, 0xf1, 0x2a, 0x36, 0x9d, 0x8a,
	0xf6, 0x5c, 0xc4, 0x68, 0x40, 0x39, 0xe8, 0xec,
	0x69, 0x38, 0xc2, 0xab, 0xa7, 0x36, 0x23, 0xec,
	0x82, 0x07, 0x6e, 0x2c, 0x58, 0xf0, 0x1a, 0x9a,
	0xda, 0x2a, 0x31, 0x9b, 0x8c, 0x66, 0xd3, 0x47,
	0x49, 0x43, 0xb4, 0xea, 0xbd, 0xda, 0x99, 0x84,
	0xe9, 0x33, 0xdf, 0x4a, 0xa3, 0xcc, 0xf5, 0x22,
	0x13, 0x19, 0xdd, 0x2b, 0x37, 0xff, 0xaf, 0x33,
	0x60, 0x58, 0x15, 0x3e, 0xb2, 0x72, 0xd0, 0x06,
	0xa8, 0x90, 0x32, 0x75, 0x8e, 0x68, 0xa2, 0x2d,
	0x05, 0xa3, 0x1b, 0x01, 0x82, 0x60, 0xad, 0x7a,
	0xcf, 0xb1, 0xfb, 0x53, 0x89, 0x1c, 0xbb, 0xfc,
	0xa1, 0x1b, 0x39, 0x1b, 0xdb, 0x9d, 0xd4, 0x06,
	0x52, 0x83, 0x80, 0x4a, 0xf1, 0x23, 0x42, 0x27,
	0xfb, 0xc8, 0xb7, 0x12, 0x85, 0x31, 0x8d, 0x32,
	0xf7, 0x0e, 0x08, 0xe4, 0x8e, 0x86, 0x7f, 0x37,
	0xdd, 0xb3, 0xdc, 0x70, 0x47, 0x24, 0x20, 0x87,
	0x87, 0x5a, 0xdc, 0xa8, 0x28, 0x4c, 0xb9, 0xb3,
	0x0d, 0xe3, 0xf2, 0xbc, 0xea, 0x7e, 0xd4, 0x8c,
	0xc9, 0x70, 0x45, 0x1e, 0x87, 0x7d, 0x3a, 0x23,
	0x54, 0x60, 0x3f, 0x7f, 0x37, 0x48, 0xf3, 0xca,
	0x87, 0x57, 0x89, 0xcf, 0x72, 0x22, 0x48, 0x10,
	0x7a, 0x33, 0x0c, 0x40, 0xf3, 0x8a, 0xc3, 0xc7,
	0x87, 0x17, 0xf0, 0x42, 0xb3, 0x41, 0x2c, 0x01,
	0x47, 0x63, 0xa0, 0x87, 0xe9, 0x4a, 0x8d, 0x0d,
	0x92, 0xb9, 0xc4, 0xff, 0x10, 0xe5, 0x2e, 0x7e,
	0x83, 0xf8, 0x45, 0xdd, 0xe0, 0x92, 0x98, 0x24,
	0x70, 0x43, 0x4b, 0x8f, 0x52, 0x13, 0x95, 0x0f,
	0xf5, 0xfb, 0x41, 0xc8, 0x9f, 0x69, 0x2e, 0x92,
	0xe9, 0xc0, 0xcf, 0x79, 0x41, 0xd5, 0xab, 0x3d,
	0x66, 0x73, 0xdd, 0xd2, 0xef, 0xd8, 0x95, 0xe1,
	0xc5, 0x35, 0x96, 0x45, 0xa4, 0x32, 0xb6, 0x8f,
	0x9e, 0x68, 0x61, 0x83, 0x98, 0xe5, 0x16, 0x98,
	0xcb, 0xac, 0xe8, 0x7c, 0x44, 0x33, 0xff, 0x8e,
	0x65, 0xe3, 0x15, 0x62, 0x62, 0x9b, 0xf9, 0x40,
	0xc4, 0x2c, 0x0f, 0xa5, 0xea, 0xde, 0xce, 0xc0,
	0x83, 0xea, 0x40, 0xf7, 0x15, 0xff, 0x86, 0x60,
	0x79, 0xbe, 0x52, 0x48, 0x5c, 0x3d, 0x6b, 0xaf,
	0xbf, 0x88, 0x2c, 0xca, 0x78, 0x1b, 0x05, 0x80,
	0xaf, 0x69, 0xf8, 0xee, 0x63, 0x58, 0x1d, 0xe3,
	0xe2, 0xc2, 0x1f, 0x64, 0x54, 0xf6, 0xbd, 0x29,
	0x31, 0x35, 0x4a, 0x1e, 0xc6, 0x35, 0x2d, 0xe3,
	0xb4, 0xa2, 0x62, 0x4c, 0x71, 0x98, 0xf7, 0xe1,
	0xc6, 0x2a, 0x90, 0x60, 0x4f, 0xde, 0xe3, 0x37,
	0xfd, 0x2f, 0x3d, 0x0a, 0x97, 0x0a, 0xfb, 0x33,
	0x35, 0x51, 0x12, 0x3c, 0xc4, 0x5b, 0x87, 0x67,
	0x85, 0x71, 0xf7, 0x27, 0x8e, 0x53, 0x11, 0xa4,
	0x47, 0xb1, 0x17, 0x3b, 0xee, 0xb2, 0x61, 0xfb,
	0x14, 0x71, 0xda, 0x2a, 0x1d, 0x7b, 0x81, 0xbe,
	0xc4, 0x52, 0xe9, 0xe4, 0x95, 0xed, 0xb9, 0x7c,
	0x71, 0x36, 0x2c, 0x9b, 0x0e, 0x8b, 0x92, 0x08,
	0x74, 0x3d, 0xce, 0xbf, 0x81, 0x14, 0xd6, 0x72,
	0x65, 0xc8, 0x36, 0x03, 0x27, 0x8a, 0x8d, 0x0a,
	0x1e, 0x79, 0x0f, 0x55, 0x79, 0x2d, 0x01, 0x63,
	0xb8, 0x30, 0x40, 0xe8, 0x31, 0x80, 0xba, 0x4d,
	0x8c, 0x0e, 0xf3, 0x2d, 0x46, 0x42, 0x82, 0xd9,
	0x32, 0x75, 0x92, 0xd5, 0xf3, 0x75, 0x61, 0x58,
	0x84, 0x05, 0xc4, 0xd0, 0xb0, 0x5a, 0xa1, 0x5b,
	0x9a, 0x9f, 0x73, 0x4f, 0x37, 0x72, 0xca, 0xb3,
	0xcf, 0x65, 0xc8, 0xc4, 0x7f, 0x7e, 0xa6, 0x72,
	0xba, 0xb7, 0x2d, 0xe0, 0xc3, 0x7e, 0x3d, 0xe7,
	0x35, 0x36, 0x49, 0x93, 0x7b, 0xb5, 0xd8, 0xa4,
	0x58, 0xc4, 0x06, 0x0f, 0x60, 0xa2, 0x01, 0x7b,
	0x7d, 0x81, 0x8d, 0xc4, 0x6c, 0x07, 0x80, 0x7c,
	0x3d, 0xce, 0x47, 0x64, 0xd6, 0xe5, 0x5f, 0xf7,
	0x71, 0x4d, 0xdd, 0xe0, 0x19, 0x7c, 0xe6, 0x7f,
	0x31, 0xde, 0x37, 0x68, 0xed, 0x4f, 0x9f, 0xe4,
	0xd7, 0xa2, 0x80, 0x6e, 0x4b, 0x1d, 0x52, 0x37,
	0xfc, 0xfa, 0x20, 0xa2, 0x6d, 0xe8, 0x09, 0xc9,
	0x78, 0x88, 0xc0, 0xf6, 0xca, 0xf1, 0x0c, 0x2b,
	0x65, 0x2d, 0x48, 0x9b, 0x1d, 0xb9, 0xe4, 0x2e,
	0x1c, 0x08, 0xe3, 0x01, 0x5e, 0x00, 0x5b, 0xe3,
	0x35, 0x7c, 0xf8, 0xd9, 0xc6, 0xc9, 0x7a, 0x9b,
	0x8a, 0x29, 0x32, 0x16, 0xcf, 0x53, 0x89, 0xe8,
	0x34, 0xf1, 0x78, 0xe6, 0x30, 0x20, 0x11, 0x99,
	0x8c, 0xf4, 0xf5, 0xbd, 0xe4, 0xf1, 0xdc, 0xc1,
	0x2a, 0x93, 0x10, 0x4a, 0x23, 0xc7, 0xf2, 0xaf,
	0xe8, 0x6f, 0x74, 0x7e, 0x66, 0xe2, 0x9d, 0xf6,
	0xdf, 0x6a, 0x08, 0x8b, 0x67, 0xc5, 0x65, 0x66,
	0x67, 0xa4, 0xf6, 0xe2, 0x1e, 0x2f, 0x13, 0x10,
	0x1b
};


BYTE HC128BaseKey[] = 
{
	0x0e, 0x56, 0x86, 0x87, 0x08, 0x3d, 0xb5,
	0xc4, 0x4f, 0x08, 0x4c, 0x4c, 0x29, 0x1f, 0x01,
	0x8c, 0x5e, 0x92, 0x4f, 0xd4, 0xbb, 0x90, 0xf4,
	0xd5, 0x8f, 0xf1, 0x74, 0x29, 0x31, 0xae, 0x94,
	0xcc, 0x78, 0x63, 0xdf, 0x9b, 0x41, 0xd4, 0x24,
	0xdc, 0xed, 0x65, 0xf6, 0xb6, 0xe0, 0xa0, 0x2b,
	0xb4, 0x03, 0xb3, 0x5d, 0x46, 0x43, 0xef, 0x6c,
	0x40, 0x10, 0x4b, 0xfa, 0x59, 0xdf, 0xdf, 0xee,
	0xad, 0xa8, 0x69, 0xf2, 0x3c, 0x69, 0xcb, 0xf5,
	0x69, 0xa1, 0x8a, 0xaa, 0x7b, 0xd5, 0x51, 0x06,
	0x20, 0x0f, 0x6d, 0xc6, 0xe5, 0x5a, 0x4f, 0xe7,
	0xbf, 0x47, 0x0c, 0x2c, 0x85, 0x6b, 0xe0, 0x9b,
	0x73, 0xdf, 0xa7, 0x00, 0xa9, 0xbf, 0x62, 0x69,
	0xaa, 0xab, 0xb8, 0xa8, 0xdd, 0x49, 0x73, 0xd4,
	0x10, 0xc1, 0xff, 0xc9, 0xf0, 0x3f, 0xee, 0xa3,
	0x92, 0x75, 0x76, 0x47, 0xed, 0x16, 0xf1, 0xda,
	0x5d, 0x5c, 0x5d, 0x48, 0x22, 0x83, 0xda, 0xbe,
	0xdf, 0x4c, 0x2e, 0x30, 0x1c, 0x7b, 0x44, 0xd5,
	0xc4, 0x59, 0xa9, 0xa6, 0xa7, 0x32, 0x0e, 0xe2,
	0xf9, 0xd8, 0xc8, 0x8d, 0xd1, 0x1f, 0x53, 0xec,
	0xab, 0x5f, 0xcb, 0x0a, 0xe7, 0xf5, 0x71, 0x36,
	0x48, 0xc2, 0x2d, 0x83, 0x76, 0xab, 0x06, 0x47,
	0x7c, 0x17, 0xab, 0x9d, 0x4b, 0x74, 0xed, 0xe3,
	0x35, 0xb2, 0x43, 0x3c, 0x72, 0xc6, 0x45, 0x0f,
	0x9e, 0x28, 0x31, 0x86, 0x39, 0x56, 0x69, 0x10,
	0x26, 0x4f, 0xf3, 0xe0, 0x2d, 0x19, 0xf8, 0x6c,
	0x79, 0x3c, 0x46, 0xee, 0x1a, 0x44, 0xce, 0xe6,
	0x84, 0x42, 0x26, 0x96, 0x0f, 0x4b, 0x07, 0x85,
	0x75, 0xf8, 0xd0, 0xfd, 0x57, 0xe5, 0x02, 0x8d,
	0xb8, 0x32, 0xc2, 0x87, 0x80, 0x05, 0x5a, 0x83,
	0xfa, 0x06, 0xb8, 0xda, 0x56, 0xe2, 0xee, 0x2c,
	0x28, 0xc8, 0xb0, 0xdb, 0xe8, 0xef, 0xda, 0x8d,
	0x70, 0x0e, 0xe7, 0xae, 0x81, 0xe2, 0x7a, 0xec,
	0x3e, 0xac, 0xd9, 0xb9, 0xaf, 0xb0, 0x6d, 0xcc,
	0x3f, 0xb7, 0x44, 0xa0, 0x3f, 0x8e, 0x8f, 0xf4,
	0x61, 0x85, 0x24, 0x49, 0x3d, 0xf1, 0xfc, 0x68,
	0xcf, 0xaa, 0xb6, 0xd9, 0xf7, 0x8e, 0x13, 0x6d,
	0xf8, 0xfc, 0x78, 0xb5, 0xfa, 0x59, 0x70, 0x88,
	0x89, 0x8f, 0x27, 0x81, 0x13, 0x89, 0xf0, 0x7e,
	0x6d, 0xb8, 0xbf, 0x22, 0x4f, 0x91, 0xaf, 0x54,
	0xd3, 0x0d, 0x7e, 0xbf, 0xfa, 0x27, 0x0c, 0x50,
	0x27, 0x62, 0xe0, 0xba, 0xa2, 0x40, 0xa3, 0xf5,
	0x16, 0xcc, 0xa3, 0xbb, 0x14, 0x11, 0x51, 0x0a,
	0x8e, 0xa1, 0xc3, 0xa5, 0x5c, 0x0f, 0x32, 0x93,
	0xbb, 0x75, 0x7d, 0x9e, 0xc9, 0xef, 0xa5, 0xd5,
	0x0a, 0x1e, 0x50, 0x0b, 0xe6, 0xa6, 0x46, 0x55,
	0x28, 0xb0, 0xf6, 0x90, 0x81, 0x68, 0xf2, 0xd9,
	0x03, 0x80, 0x6f, 0x13, 0xa6, 0xac, 0xc6, 0x64,
	0xc7, 0x24, 0xf5, 0xb8, 0xa4, 0x25, 0x1f, 0x3d,
	0xe1, 0x70, 0x08, 0xe5, 0x06, 0xca, 0x9a, 0xe8,
	0xff, 0x7a, 0x63, 0x3f, 0x9a, 0xce, 0x14, 0x2a,
	0x0c, 0x96, 0x03, 0xaa, 0x6d, 0xa7, 0xaa, 0x08,
	0x37, 0x59, 0x26, 0x4c, 0xcb, 0x0a, 0xb9, 0xc7,
	0xed, 0x99, 0x48, 0x8a, 0x43, 0xec, 0xde, 0xec,
	0xd9, 0x6a, 0x27, 0x08, 0xa0, 0x81, 0xf6, 0x3d,
	0xea, 0x22, 0xbf, 0xac, 0xef, 0x40, 0x1e, 0xbd,
	0x4c, 0x55, 0x4e, 0x9b, 0x7f, 0xdc, 0xb3, 0xb3,
	0x6c, 0xd8, 0x51, 0x39, 0xdb, 0x4c, 0x52, 0xa3,
	0xf8, 0xc1, 0x84, 0x2c, 0xd1, 0xc3, 0xd8, 0x52,
	0xdc, 0x64, 0xe4, 0x58, 0x6e, 0xb7, 0x62, 0xc5,
	0x45, 0x56, 0xaf, 0xe3, 0xff, 0xdd, 0x4d, 0x41,
	0xa0, 0x6d, 0x61, 0x32, 0x10, 0x2a, 0x36, 0x4c,
	0x9b, 0xbd, 0xb8, 0xe9, 0x6f, 0xd3, 0xfa, 0xa9,
	0x22, 0x9c, 0xb1, 0xef, 0x29, 0x4d, 0xb6, 0x5f,
	0x62, 0x9e, 0x88, 0x66, 0x8b, 0x4c, 0xc7, 0xb2,
	0xc8, 0x99, 0xba, 0xb6, 0x21, 0xc7, 0xcb, 0x27,
	0x02, 0xa0, 0x06, 0x81, 0xba, 0xf1, 0x9d, 0x83,
	0xfc, 0x0b, 0x66, 0xaf, 0x61, 0x41, 0x5b, 0xcc,
	0xe3, 0x6c, 0x19, 0x63, 0x63, 0x6a, 0x62, 0x45,
	0x25, 0x9a, 0x9c, 0x03, 0x4f, 0x62, 0x4f, 0x75,
	0x6d, 0xaa, 0xab, 0x34, 0xf0, 0x5e, 0x00, 0x1f,
	0xaa, 0xef, 0x44, 0xda, 0x54, 0xd3, 0x90, 0x4a,
	0x09, 0x01, 0xa3, 0x1b, 0xc8, 0x76, 0x5d, 0x3a,
	0xf5, 0xb2, 0x45, 0x5b, 0xd9, 0x3b, 0x05, 0x74,
	0x1d, 0x19, 0xe8, 0x40, 0x53, 0x59, 0x63, 0xbe,
	0x6d, 0x8b, 0x89, 0xaf, 0x44, 0x43, 0x95, 0x1b,
	0x13, 0x9b, 0x64, 0xcc, 0xf9, 0xaf, 0xf8, 0xd2,
	0x7a, 0x20, 0xf7, 0xfd, 0xfe, 0x93, 0x2a, 0x66,
	0x51, 0x2f, 0xfe, 0xe7, 0x22, 0x22, 0x06, 0x9e,
	0x84, 0x1c, 0x77, 0x6e, 0x70, 0xd2, 0xab, 0x7e,
	0x41, 0x7c, 0x9e, 0xb8, 0x36, 0x58, 0x74, 0x4b,
	0xf3, 0x25, 0xf1, 0x2a, 0x01, 0xa8, 0xff, 0x8b,
	0x49, 0x2b, 0x2c, 0x68, 0x9d, 0xf9, 0x2a, 0x01,
	0x30, 0xe3, 0x4d, 0x58, 0x18, 0xbf, 0x10, 0xb4,
	0xd3, 0xe3, 0x91, 0x1e, 0xbf, 0xae, 0x10, 0xe7,
	0xa1, 0x00, 0x73, 0x21, 0x1f, 0xbc, 0xc5, 0x21,
	0x46, 0x4d, 0xb3, 0x03, 0x05, 0x1f, 0x0e, 0x26,
	0xaf, 0x21, 0x4c, 0xac, 0x7d, 0x4a, 0x06, 0xfb,
	0x0a, 0x11, 0x7b, 0x3f, 0xd5, 0xf3, 0x0c, 0xe5,
	0xc2, 0xf0, 0xbe, 0x22, 0x9a, 0x0f, 0xbb, 0x6a,
	0x87, 0xd6, 0xd1, 0xf9, 0x98, 0xd2, 0xf2, 0x4d,
	0x43, 0x15, 0xb2, 0xaa, 0xde, 0xb3, 0xcc, 0x95,
	0x25, 0x44, 0x9e, 0x5a, 0xb7, 0x66, 0xa8, 0x86,
	0x98, 0x38, 0x11, 0x6d, 0xb1, 0xdf, 0x22, 0xa5,
	0x4c, 0x04, 0xc8, 0x89, 0x98, 0x54, 0x16, 0xb7,
	0x2b, 0x00, 0xc1, 0x92, 0x7b, 0x3b, 0xa3, 0xc1,
	0x64, 0xbe, 0x39, 0xae, 0xa5, 0x47, 0x25, 0x09,
	0x63, 0x15, 0xac, 0x42, 0xa4, 0x6e, 0x38, 0x12,
	0xd5, 0x19, 0xd7, 0xf3, 0x45, 0xe5, 0xbb, 0xa3,
	0xa7, 0x20, 0xb8, 0xa5, 0x94, 0x21, 0xca, 0xc0,
	0x07, 0xbe, 0x8c, 0x7e, 0xdf, 0xd6, 0xc1, 0xad,
	0x61, 0xc9, 0xcf, 0xe2, 0xb3, 0xfb, 0x3f, 0xf2,
	0x62, 0x55, 0x3f, 0x77, 0xdd, 0xc3, 0x20, 0x51,
	0xf8, 0xb7, 0xd8, 0x22, 0x6a, 0xa5, 0x81, 0xd0,
	0x4f, 0x85, 0xd8, 0x07, 0xa6, 0x54, 0xbe, 0xb5,
	0xd4, 0x93, 0xbb, 0x8c, 0x1f, 0xc6, 0x76, 0x83,
	0x34, 0xf6, 0x3e, 0x56, 0xa2, 0x30, 0x85, 0x01,
	0x5d, 0x04, 0x60, 0x49, 0x3b, 0x07, 0x08, 0x33,
	0x7a, 0x51, 0x5b, 0x8b, 0x39, 0x00, 0x5c, 0x5e,
	0xfb, 0xb3, 0xaf, 0x80, 0x27, 0x10, 0x1e, 0x07,
	0x8a, 0x3e, 0x16, 0xce, 0xd3, 0x6b, 0x2a, 0xf4,
	0x16, 0x47, 0x90, 0x5a, 0x4a, 0x88, 0x9f, 0x28,
	0xca, 0x64, 0x57, 0x49, 0xd8, 0x1a, 0xd9, 0xea,
	0x16, 0x69, 0xeb, 0xff, 0x0b, 0x17, 0x75, 0xbe,
	0xa4, 0x6c, 0x06, 0x21, 0xb0, 0xb4, 0x50, 0x68,
	0x63, 0xc1, 0xa8, 0x96, 0xd4, 0x66, 0x87, 0xf0,
	0x7f, 0xfd, 0x0c, 0x81, 0xc3, 0xe2, 0x77, 0x98,
	0x65, 0xf6, 0xaf, 0x47, 0x0c, 0x1d, 0xbd, 0xe6,
	0xc3, 0xc0, 0x4f, 0x8f, 0x7a, 0x4c, 0x36, 0xa0,
	0x84, 0xb1, 0xe8, 0x3c, 0x1a, 0xe4, 0xff, 0xc9,
	0xd7, 0x5d, 0xb8, 0x73, 0x3b, 0x99, 0x75, 0xa8,
	0x29, 0x99, 0x3b, 0x9a, 0x68, 0x61, 0x35, 0xc1,
	0x25, 0x7b, 0x2f, 0x56, 0x6f, 0x72, 0x1c, 0xd9,
	0xba, 0x57, 0x91, 0x8c, 0x5d, 0x3f, 0x47, 0xf5,
	0x14, 0xc2, 0x9d, 0x60, 0x7f, 0x7e, 0x13, 0x5a,
	0xa1, 0x92, 0xd0, 0x38, 0x61, 0x24, 0x1d, 0x8e,
	0x0c, 0xdc, 0xe8, 0xb8, 0xd1, 0x66, 0x42, 0x54,
	0x44, 0xf3, 0xe2, 0xc6, 0xdc, 0xb9, 0xa0, 0xb3,
	0x75
};

BYTE VMPCBaseKey[] =
{
	0xeb, 0x7f, 0x0e, 0x99, 0xcc, 0xe3, 0xc9,
	0x3b, 0xf2, 0x1d, 0xb4, 0x71, 0x6b, 0xa7, 0x1e,
	0x06, 0x69, 0x41, 0xba, 0x4c, 0xc1, 0x6d, 0x21,
	0x96, 0x6c, 0xf2, 0x4e, 0xbe, 0x08, 0x5a, 0x20,
	0xa0, 0x57, 0x74, 0xdc, 0x9c, 0xbf, 0xd3, 0xa8,
	0x18, 0xc9, 0x4c, 0x11, 0xfc, 0xa3, 0x7c, 0x86,
	0x35, 0x9e, 0x3f, 0xdb, 0x33, 0xb0, 0x3a, 0xc7,
	0x6a, 0xf3, 0x53, 0x66, 0xd4, 0x24, 0x34, 0xb7,
	0x6d, 0x24, 0xcb, 0x1f, 0xb6, 0x7b, 0xcc, 0xe4,
	0x34, 0xd0, 0x2e, 0xb3, 0xee, 0x72, 0xaa, 0x1b,
	0xf2, 0xd3, 0x40, 0x10, 0xd0, 0x08, 0xb1, 0x69,
	0x1d, 0x4a, 0x06, 0x61, 0xf1, 0x77, 0x07, 0x1a,
	0x69, 0x92, 0xc5, 0x16, 0x27, 0x3f, 0x11, 0xbc,
	0xcd, 0x48, 0x02, 0xd9, 0x86, 0x1a, 0x73, 0x1c,
	0x7d, 0x48, 0x82, 0x99, 0x63, 0x08, 0x13, 0x47,
	0xed, 0xb1, 0x4b, 0x82, 0x54, 0x43, 0x15, 0x89,
	0xd4, 0xdf, 0xa0, 0x01, 0x2d, 0x4b, 0xdf, 0x70,
	0x25, 0x6f, 0x08, 0xc3, 0x04, 0xda, 0x16, 0xc9,
	0x16, 0x3e, 0x47, 0xb6, 0x2d, 0xf0, 0x9e, 0xa1,
	0x1c, 0x69, 0x62, 0x06, 0x3d, 0xc8, 0x9d, 0x44,
	0xec, 0x4d, 0x9e, 0x20, 0x0a, 0xdf, 0x78, 0x41,
	0x7b, 0x87, 0x81, 0xb2, 0xa8, 0xf3, 0xd3, 0x63,
	0xfe, 0xf4, 0xce, 0xa7, 0x6c, 0x00, 0x94, 0xb8,
	0xe9, 0xb1, 0x8c, 0x2f, 0xec, 0x44, 0xe0, 0x8c,
	0xf3, 0x1b, 0xfe, 0xb4, 0xfd, 0x3b, 0x1b, 0x6d,
	0x0f, 0xcf, 0xab, 0xe4, 0xb2, 0xa3, 0xeb, 0x28,
	0x73, 0xaa, 0x57, 0xad, 0x62, 0x79, 0x34, 0xca,
	0x94, 0xc9, 0x08, 0x3b, 0xa2, 0xf9, 0x1c, 0xa0,
	0x28, 0x89, 0x01, 0xfc, 0x46, 0xa0, 0x08, 0x36,
	0x22, 0x87, 0xc8, 0x9b, 0x63, 0x2c, 0x9c, 0x5a,
	0xb7, 0xa1, 0x23, 0x07, 0x4f, 0x9a, 0xbf, 0x19,
	0x5e, 0xf2, 0x16, 0x6c, 0x9e, 0x26, 0x93, 0xc0,
	0xcb, 0xd8, 0xe6, 0x38, 0x25, 0x4d, 0x80, 0xdc,
	0xf2, 0xf0, 0x18, 0x17, 0xfa, 0xcd, 0x29, 0x39,
	0x0a, 0x17, 0x71, 0xf6, 0x71, 0xa3, 0x73, 0xe5,
	0x86, 0x6a, 0xf6, 0x02, 0x20, 0x0b, 0x84, 0x2c,
	0x1c, 0x47, 0xed, 0xa8, 0xdb, 0x83, 0xc1, 0x9d,
	0xc1, 0x49, 0xd9, 0x96, 0xb7, 0xc7, 0xcf, 0x03,
	0xaa, 0x4e, 0x81, 0xb8, 0x0a, 0xd4, 0x92, 0x6c,
	0x4c, 0x73, 0xe9, 0x3b, 0x68, 0xe9, 0x30, 0x25,
	0x5c, 0x16, 0x56, 0x8d, 0xa7, 0x80, 0x0d, 0xbb,
	0xce, 0xd2, 0x4d, 0x5a, 0xdb, 0x58, 0xcf, 0xfa,
	0xd9, 0x85, 0x93, 0x8f, 0x59, 0x6e, 0x5b, 0xf0,
	0xf0, 0x4d, 0x2e, 0x59, 0xb7, 0xfe, 0xd5, 0xea,
	0xca, 0x85, 0x61, 0x26, 0xc9, 0x86, 0xa3, 0x75,
	0x5a, 0xcb, 0xb3, 0xa2, 0xa5, 0xc2, 0x6a, 0x5d,
	0xd6, 0xfd, 0xe8, 0xba, 0x9f, 0xb0, 0x0e, 0xb0,
	0xb3, 0x36, 0x05, 0x9b, 0x4c, 0x8c, 0xb5, 0xbb,
	0xa6, 0xd4, 0x4f, 0xb3, 0x82, 0xd4, 0xc4, 0x0b,
	0xa4, 0x75, 0x4b, 0xae, 0x55, 0x44, 0xdf, 0x6c,
	0xe1, 0xf4, 0xbf, 0x79, 0x1a, 0xda, 0xec, 0xec,
	0xd4, 0x70, 0xae, 0x42, 0x67, 0xd3, 0x0f, 0xd8,
	0x30, 0x44, 0x5f, 0x74, 0x10, 0xaa, 0xaf, 0xbd,
	0xec, 0x0f, 0x56, 0xbe, 0x2b, 0x1f, 0x6e, 0x68,
	0x3b, 0xac, 0x58, 0x0d, 0x0c, 0x2d, 0x34, 0xe5,
	0x93, 0x3a, 0x6a, 0x8c, 0x48, 0x11, 0x24, 0x82,
	0xa9, 0x14, 0xd1, 0xaa, 0xb5, 0x49, 0xa4, 0xcc,
	0x72, 0xd9, 0x12, 0x13, 0x67, 0x91, 0x58, 0x90,
	0x23, 0x64, 0xf3, 0xb5, 0xb4, 0xe7, 0x26, 0xda,
	0x30, 0xd4, 0x78, 0xbb, 0x30, 0x88, 0x33, 0xf8,
	0x50, 0x84, 0xe5, 0x94, 0xb0, 0xf0, 0xe3, 0x77,
	0x76, 0x13, 0xc1, 0xec, 0x4a, 0xdc, 0xdc, 0x24,
	0xd9, 0x5d, 0xd0, 0xb1, 0x52, 0x4a, 0x02, 0x0b,
	0xec, 0x7e, 0x18, 0x0e, 0x5e, 0x77, 0x7b, 0x7a,
	0x65, 0xd5, 0xdc, 0x72, 0x42, 0xdf, 0xac, 0xfe,
	0x39, 0xfe, 0xa3, 0x89, 0x13, 0x3f, 0x3a, 0x64,
	0x9d, 0xd5, 0x30, 0x41, 0x27, 0x95, 0x09, 0xb8,
	0x06, 0x79, 0x8a, 0xc6, 0x12, 0x1e, 0x3f, 0x48,
	0x28, 0x46, 0xf5, 0x85, 0xaa, 0x56, 0x40, 0xa2,
	0xfa, 0xd8, 0xf6, 0x2b, 0x03, 0xfb, 0xb2, 0x90,
	0xb0, 0x0e, 0x52, 0xa5, 0x72, 0x09, 0x7a, 0x22,
	0xbe, 0x04, 0x0f, 0x21, 0x8c, 0xbd, 0xbc, 0xa3,
	0xda, 0x17, 0x70, 0x0b, 0x28, 0x96, 0xde, 0xa1,
	0xf9, 0xe4, 0xfc, 0x11, 0x58, 0x4e, 0x85, 0xe9,
	0x50, 0x47, 0x77, 0x1e, 0x73, 0xe4, 0x95, 0x88,
	0x54, 0x5f, 0xe6, 0x61, 0x0d, 0x95, 0x34, 0xca,
	0xba, 0x88, 0x8e, 0x46, 0xfc, 0xdd, 0xc6, 0x3d,
	0x77, 0x5f, 0xf4, 0x7b, 0x54, 0x7a, 0xf1, 0xae,
	0xbf, 0xc1, 0xdd, 0xeb, 0x6a, 0x68, 0x9a, 0x2a,
	0x09, 0xcb, 0x4f, 0xc5, 0xd4, 0xe5, 0xe6, 0xfd,
	0x08, 0xda, 0x8d, 0x75, 0x66, 0x6d, 0x39, 0xb5,
	0xb3, 0x8a, 0x1e, 0xa9, 0x36, 0xbe, 0x38, 0x1f,
	0x3d, 0xba, 0xc6, 0x4d, 0x98, 0xd5, 0xca, 0x48,
	0x1c, 0x86, 0x8a, 0x8d, 0x21, 0xed, 0x12, 0x7d,
	0x05, 0x4b, 0xaf, 0xd9, 0xa7, 0x86, 0x75, 0x4a,
	0xec, 0xa6, 0xbb, 0xdb, 0x3e, 0x5b, 0x9a, 0x7d,
	0x08, 0x74, 0x71, 0x82, 0x3c, 0x69, 0x64, 0x23,
	0xcd, 0xd2, 0xd8, 0xfa, 0x35, 0xee, 0xf8, 0x88,
	0xef, 0x1d, 0x33, 0xb0, 0xfe, 0x66, 0xbc, 0x3a,
	0x65, 0xf2, 0x09, 0x52, 0xad, 0x8f, 0x55, 0x07,
	0x62, 0x2e, 0x1e, 0xcc, 0x96, 0x66, 0xa8, 0xf9,
	0x5c, 0xef, 0x78, 0x4b, 0x4e, 0x27, 0xd9, 0x60,
	0x08, 0x90, 0x5a, 0x3c, 0xab, 0x4f, 0x4e, 0xc7,
	0x5b, 0xaf, 0x4a, 0x4d, 0xc1, 0x9c, 0xab, 0xfd,
	0x8a, 0x29, 0x0e, 0x6a, 0xe6, 0x0b, 0xd6, 0x0c,
	0x0a, 0x1b, 0xaa, 0xc0, 0xae, 0xd8, 0xf4, 0x44,
	0x8f, 0xe2, 0x63, 0xbc, 0xef, 0x80, 0x69, 0x31,
	0x10, 0x1b, 0xbe, 0x0c, 0xbc, 0xc1, 0xdb, 0x9f,
	0xc0, 0xa4, 0x80, 0x9c, 0x6d, 0x98, 0x2f, 0x9c,
	0x16, 0x98, 0xae, 0x99, 0x94, 0x41, 0x89, 0x75,
	0xc5, 0x55, 0x8d, 0x71, 0x08, 0x39, 0x4f, 0xb6,
	0xc3, 0x78, 0xa3, 0xd0, 0xde, 0x3f, 0x25, 0x2d,
	0x45, 0xde, 0xb4, 0xa3, 0x69, 0x4d, 0xf1, 0xe7,
	0xc0, 0xa5, 0xc5, 0x17, 0x41, 0xa2, 0xd8, 0x31,
	0xe9, 0x28, 0x1b, 0x9a, 0x38, 0xbb, 0x3f, 0x98,
	0xb4, 0x05, 0x3b, 0xd7, 0x65, 0x54, 0xca, 0xe8,
	0x58, 0x1a, 0xea, 0xbd, 0x1c, 0x6b, 0x5e, 0x2f,
	0x48, 0x82, 0x2e, 0x79, 0xf3, 0x3c, 0x22, 0xba,
	0x3a, 0x9b, 0x4a, 0x76, 0xbe, 0x45, 0x79, 0x16,
	0x24, 0x03, 0xc5, 0x63, 0x93, 0x42, 0x09, 0x0f,
	0x39, 0x95, 0x63, 0x2d, 0xc6, 0x31, 0xb6, 0xb3,
	0xef, 0x6f, 0x29, 0xff, 0xec, 0x4e, 0xa6, 0x4f,
	0xfb, 0xef, 0x5c, 0x48, 0xdb, 0x17, 0x3d, 0x70,
	0x51, 0xb0, 0x81, 0xb4, 0xa7, 0x48, 0x22, 0xe2,
	0x28, 0x91, 0x5d, 0x30, 0xa5, 0xdf, 0x37, 0xb4,
	0xf4, 0xad, 0xf6, 0xe9, 0x6b, 0x18, 0xa4, 0x31,
	0x69, 0x63, 0x90, 0x4d, 0xce, 0x71, 0xcc, 0xe6,
	0x7d, 0x4e, 0xb0, 0x08, 0xe1, 0xa7, 0x55, 0xa2,
	0x66, 0x4c, 0x1b, 0x07, 0xfb, 0xb5, 0x23, 0x70,
	0x97, 0x7b, 0xd6, 0x78, 0xb1, 0xdb, 0x5c, 0x9e,
	0xc6, 0x37, 0x26, 0xc7, 0xd6, 0x94, 0x65, 0xb9,
	0xe8, 0x1c, 0x90, 0xa1, 0x82, 0x9d, 0xe2, 0x8e,
	0x31, 0x09, 0xda, 0xf3, 0x07, 0xf4, 0xb9, 0x29,
	0x18, 0x19, 0x08, 0xeb, 0xfc, 0xd6, 0x0f, 0xd8,
	0x51, 0xab, 0x5e, 0xf5, 0x36, 0xbf, 0x48, 0x28,
	0xd0, 0x5a, 0x63, 0xbe, 0xc8, 0x6c, 0x0a, 0xe6,
	0xcb, 0x05, 0xdb, 0x33, 0x0a, 0xdb, 0x3a, 0x1e,
	0xb8, 0xc8, 0xcc, 0x82, 0x8e, 0x49, 0xfc, 0x1e,
	0x4a
};

Void NTAPI InitAll1StKey()
{
	auto g_GlobalMap = GlobalMap::GetGlobalMap();
	g_GlobalMap->InsertNode(BaseDecoder1_Mask, BaseDecoder1_Key);
	g_GlobalMap->InsertNode(HC128_BaseKey_Mask, HC128BaseKey);
	g_GlobalMap->InsertNode(VMPC_BaseKey_Mask, VMPCBaseKey);
}
