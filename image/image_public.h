#ifndef _IMAGE_PUBLIC_
#define _IMAGE_PUBLIC_


/**
 * Split Y, U, V planes in YUV420P file.
 * @param url  Location of Input YUV file.
 * @param w    Width of Input YUV file.
 * @param h    Height of Input YUV file.
 * @param num  Number of frames to process.
 *
 */

int public_yuv420_split(char *url, int w, int h,int num);


#endif
