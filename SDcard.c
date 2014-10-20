#include "cfs/cfs.h"

int file_read(char* filename, char* storage, int max_len)
{
  // Obtain a file descriptor for the file, capable of
  // handling both reads and writes.
  int fd = cfs_open(filename, CFS_READ);
  if (fd < 0) {
    printf("Failed to open %s\n", filename);
    return -1;
  }
  printf("Opened file %s\n", filename);

  // Read the message back
  int file_len = cfs_seek(fd, 0, CFS_SEEK_END);
  file_len = file_len<max_len?file_len:max_len;
  cfs_seek(fd, 0, CFS_SEEK_SET);

  int r = cfs_read(fd, storage, file_len);

  if (r < file_len) {
    printf("Failed to read %d bytes from %s, got %d bytes\n",
             file_len, filename, r);
      cfs_close(fd);
      return 0;
  }
	storage[r] = '\0';
  // Close the file and release resources associated with fd
  cfs_close(fd);
  return file_len;
}
