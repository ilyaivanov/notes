#include <sys/stat.h>
#include <fcntl.h>   // For open flags (O_RDONLY)
#include <unistd.h>  // For open, read, close, ssize_t
#include <stdio.h>   // For printf, perror
#include <stdlib.h>  // For exit, malloc, free


char *readFile(const char *filepath, size_t *length_out) {
  int fd = open(filepath, O_RDONLY);
  if (fd == -1) {
    perror("Error opening file");
    return NULL;
  }

  struct stat file_status;
  if (fstat(fd, &file_status) == -1) {
    perror("Error getting file size");
    close(fd);
    return NULL;
  }

  // Use the file size from stat to determine buffer size
  off_t file_size = file_status.st_size;

  // Allocate memory for the buffer, plus one byte for a null terminator
  // (optional, but good practice for text)
  char *buffer = (char *)malloc(file_size + 1);
  if (buffer == NULL) {
    perror("Error allocating memory");
    close(fd);
    return NULL;
  }

  // Read the entire file into the buffer
  ssize_t bytes_read = read(fd, buffer, file_size);
  if (bytes_read == -1) {
    perror("Error reading file");
    free(buffer);
    close(fd);
    return NULL;
  }

  // Ensure all data was read (or handle partial reads if necessary, though
  // unlikely with small files)
  if (bytes_read != file_size) {
    fprintf(stderr, "Warning: Expected %lld bytes, but read %zd bytes\n",
            (long long)file_size, bytes_read);
  }

  // Null-terminate the buffer for string functions (only if dealing with text)
  buffer[bytes_read] = '\0';

  // Close the file descriptor
  close(fd);

  // If the caller wants the length, provide it
  if (length_out != NULL) {
    *length_out = (size_t)bytes_read;
  }

  return buffer;
}
