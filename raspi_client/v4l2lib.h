#ifndef __V4L2LIB_H__
#define __V4L2LIB_H__

#include <stdint.h>


/******************************************************************************
 Function definitions
 *****************************************************************************/
int print_caps(int fd);
int init_mmap(int fd, uint8_t *buffer);
int capture_image(int fd, int outfd);

#endif