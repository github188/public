#include <stdio.h>
#include <stdlib.h>

int public_yuv420_split(char *url, int w, int h,int num)
{
	FILE *fp = fopen(url,"rb+");
	FILE *fpy=fopen("output_420_y.y", "wb+");
	FILE *fpu=fopen("output_420_u.y", "wb+");
	FILE *fpv=fopen("output_420_v.y", "wb+");
	int i =0;
	unsigned char *pic = (unsigned char *)malloc(w * h * 3 / 2);

	for (i = 0; i < num; i++)
	{

		fread(pic, 1, w * h * 3 / 2, fp);
		//Y
		fwrite(pic, 1, w * h, fpy);
		//U
		fwrite(pic + w * h, 1, w * h / 4,fpu);
		//V
		fwrite(pic + w * h * 5 / 4, 1, w * h / 4, fpv);
	}

	free(pic);
	fclose(fp);
	fclose(fpy);
	fclose(fpu);
	fclose(fpv);

	return 0;
}

