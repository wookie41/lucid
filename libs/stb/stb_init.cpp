#define STB_DS_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include "stb_ds.h"
#include "stb_image.h"
#include <time.h>

void InitSTB() 
{
     stbi_set_flip_vertically_on_load(true);
     stbds_rand_seed(rand()); 
}