#define STB_DS_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_DEFINE

#include "stb.h"
#include "stb_ds.h"
#include "stb_image.h"
#include <time.h>

void InitSTB() 
{
     stbds_rand_seed(rand()); 
}