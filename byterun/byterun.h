#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

void *__start_custom_data;
void *__stop_custom_data;

/* The unpacked representation of bytecode file */
typedef struct
{
  char *string_ptr;          /* A pointer to the beginning of the string table */
  int *public_ptr;           /* A pointer to the beginning of publics table    */
  char *code_ptr;            /* A pointer to the bytecode itself               */
  int *global_ptr;           /* A pointer to the global area                   */
  int stringtab_size;        /* The size (in bytes) of the string table        */
  int global_area_size;      /* The size (in words) of global area             */
  int public_symbols_number; /* The number of public symbols                   */
  size_t code_size;          /* The size (in bytes) of the bytecode            */
  char buffer[0];
} bytefile;

/* Gets a string from a string table by an index */
char *get_string(bytefile *f, int pos)
{
  return &f->string_ptr[pos];
}

/* Gets a name for a public symbol */
char *get_public_name(bytefile *f, int i)
{
  return get_string(f, f->public_ptr[i * 2]);
}

/* Gets an offset for a publie symbol */
int get_public_offset(bytefile *f, int i)
{
  return f->public_ptr[i * 2 + 1];
}

/* Reads a binary bytecode file by name and unpacks it */
bytefile *read_file(char *fname)
{
  FILE *f = fopen(fname, "rb");
  long size;
  bytefile *file;

  if (f == 0)
  {
    failure("%s\n", strerror(errno));
  }

  if (fseek(f, 0, SEEK_END) == -1)
  {
    int err = errno;
    fclose(f);
    failure("%s\n", strerror(err));
  }

  size = ftell(f);

  if (size == -1L)
  {
    int err = errno;
    fclose(f);
    failure("%s\n", strerror(err));
  }

  if (size < (long)(sizeof(int) * 3))
  {
    fclose(f);
    failure("*** FAILURE: bytecode file is too small or corrupted.\n");
  }

  {
    size_t header_size = sizeof(int) * 3U;
    size_t payload_size = (size_t)size - header_size;
    file = (bytefile *)malloc(sizeof(bytefile) + payload_size);
  }

  if (file == 0)
  {
    failure("*** FAILURE: unable to allocate memory.\n");
  }

  if (fseek(f, 0, SEEK_SET) == -1)
  {
    int err = errno;
    fclose(f);
    failure("%s\n", strerror(err));
  }

  {
    size_t header_size = sizeof(int) * 3U;
    size_t payload_size = (size_t)size - header_size;
    int header[3];

    if (fread(header, sizeof(int), 3U, f) != 3U)
    {
      int err = errno;
      fclose(f);
      failure("%s\n", strerror(err));
    }

    file->stringtab_size = header[0];
    file->global_area_size = header[1];
    file->public_symbols_number = header[2];

    if (payload_size != 0U)
    {
      if (payload_size != fread(file->buffer, 1, payload_size, f))
      {
        int err = errno;
        fclose(f);
        failure("%s\n", strerror(err));
      }
    }
  }

  fclose(f);

  if (file->public_symbols_number < 0 || file->global_area_size < 0 || file->stringtab_size < 0)
  {
    failure("*** FAILURE: bytecode header contains negative sizes.\n");
  }

  {
    size_t header_size = sizeof(int) * 3U;
    size_t payload_size = (size_t)size - header_size;
    size_t entries_per_symbol = 2U * sizeof(int);
    size_t public_symbols_number = (size_t)file->public_symbols_number;

    if (public_symbols_number > payload_size / entries_per_symbol)
    {
      failure("*** FAILURE: bytecode public table exceeds file size.\n");
    }

    size_t public_table_bytes = public_symbols_number * entries_per_symbol;

    if ((size_t)file->stringtab_size > payload_size - public_table_bytes)
    {
      failure("*** FAILURE: string table exceeds remaining file size.\n");
    }

    file->public_ptr = (int *)file->buffer;
    file->string_ptr = &file->buffer[public_table_bytes];
    file->code_ptr = &file->string_ptr[file->stringtab_size];
    {
      size_t consumed = public_table_bytes + (size_t)file->stringtab_size;
      if (consumed > payload_size)
      {
        failure("*** FAILURE: bytecode section pointers are inconsistent.\n");
      }
      file->code_size = payload_size - consumed;
    }

    if (file->code_ptr < file->buffer ||
        (size_t)(file->code_ptr - file->buffer) > payload_size)
    {
      failure("*** FAILURE: bytecode section pointers are inconsistent.\n");
    }

    if (file->code_size > payload_size ||
        (size_t)(file->code_ptr - file->buffer) + file->code_size != payload_size)
    {
      failure("*** FAILURE: bytecode section size is inconsistent.\n");
    }
  }

  file->global_ptr = NULL;

  return file;
}
