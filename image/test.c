#include <stdio.h>
#include "image_public.h"

int main(void)
{
	public_yuv420_split("./yy.yuv", 320, 240, 1);
	return 0;
}
