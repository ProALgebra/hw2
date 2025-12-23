/* Lama SM Bytecode interpreter */

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include "../runtime32/runtime.h"
#define BYTERUN_NO_HEADER_IMPL
# include "byterun.h"

char *get_string(bytefile *f, uint32_t pos)
{
  return &f->string_ptr[pos];
}

char *get_public_name(bytefile *f, uint32_t i)
{
  return get_string(f, f->public_ptr[i * 2]);
}

uint32_t get_public_offset(bytefile *f, uint32_t i)
{
  return f->public_ptr[i * 2 + 1];
}

bytefile *read_file(char *fname)
{
  FILE *f = fopen(fname, "rb");
  long size;
  bytefile *file;
  const size_t header_size = sizeof(bytefile_header);

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

  if ((size_t)size < header_size)
  {
    fclose(f);
    failure("*** FAILURE: bytecode file is too small or corrupted.\n");
  }

  {
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
    size_t payload_size = (size_t)size - header_size;
    bytefile_header header;

    if (fread(&header, sizeof(header), 1U, f) != 1U)
    {
      int err = errno;
      fclose(f);
      failure("%s\n", strerror(err));
    }

    file->stringtab_size = header.stringtab_size;
    file->global_area_size = header.global_area_size;
    file->public_symbols_number = header.public_symbols_number;

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

  {
    size_t payload_size = (size_t)size - header_size;
    size_t entries_per_symbol = 2U * sizeof(uint32_t);
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

    file->public_ptr = (uint32_t *)file->buffer;
    file->string_ptr = &file->buffer[public_table_bytes];
    file->code_ptr = &file->string_ptr[file->stringtab_size];
    {
      size_t consumed = public_table_bytes + (size_t)file->stringtab_size;
      if (consumed > payload_size)
      {
        failure("*** FAILURE: bytecode section pointers are inconsistent.\n");
      }
      if (payload_size - consumed > UINT32_MAX)
      {
        failure("*** FAILURE: bytecode section size exceeds supported range.\n");
      }
      file->code_size = (uint32_t)(payload_size - consumed);
    }

    if (file->code_ptr < file->buffer ||
        (size_t)(file->code_ptr - file->buffer) > payload_size)
    {
      failure("*** FAILURE: bytecode section pointers are inconsistent.\n");
    }

    if ((size_t)file->code_size > payload_size ||
        (size_t)(file->code_ptr - file->buffer) + (size_t)file->code_size != payload_size)
    {
      failure("*** FAILURE: bytecode section size is inconsistent.\n");
    }
  }

  file->global_ptr = NULL;

  return file;
}

static int32_t rd_int(bytefile *bf, char **ip, char *end) {
  if ((size_t)(end - *ip) < sizeof(int32_t)) {
    failure("ERROR: truncated bytecode\n");
  }
  int32_t v = *(int32_t *)(*ip);
  *ip += sizeof(int32_t);
  return v;
}

static char rd_byte(bytefile *bf, char **ip, char *end) {
  if (*ip >= end) {
    failure("ERROR: truncated bytecode\n");
  }
  char v = **ip;
  (*ip)++;
  return v;
}

typedef enum {
  OG_BinOp = 0,
  OG_Storage = 1,
  OG_Load = 2,
  OG_LoadAddress = 3,
  OG_Store = 4,
  OG_Control = 5,
  OG_Pattern = 6,
  OG_Builtin = 7,
  OG_Halt = 15
} OpcodeGroup;

typedef enum {
  ST_Const = 0,
  ST_String = 1,
  ST_Sexp = 2,
  ST_StoreIndexed = 3,
  ST_StoreArray = 4,
  ST_Jump = 5,
  ST_End = 6,
  ST_Return = 7,
  ST_Drop = 8,
  ST_Dup = 9,
  ST_Swap = 10,
  ST_Elem = 11
} StorageOpcode;

typedef enum {
  Mode_Global = 0,
  Mode_Local = 1,
  Mode_Argument = 2,
  Mode_Closure = 3
} AddressMode;

typedef enum {
  Ctrl_JumpIfZero = 0,
  Ctrl_JumpIfNotZero = 1,
  Ctrl_Begin = 2,
  Ctrl_BeginCaptured = 3,
  Ctrl_Closure = 4,
  Ctrl_CallClosure = 5,
  Ctrl_Call = 6,
  Ctrl_Tag = 7,
  Ctrl_Array = 8,
  Ctrl_Fail = 9,
  Ctrl_Line = 10
} ControlOpcode;

typedef enum {
  Patt_StringMatch = 0,
  Patt_StringTag = 1,
  Patt_ArrayTag = 2,
  Patt_Unboxed = 5,
  Patt_ClosureTag = 6
} PatternOpcode;

typedef enum {
  Builtin_Read = 0,
  Builtin_Write = 1,
  Builtin_Length = 2,
  Builtin_ToString = 3,
  Builtin_MakeArray = 4
} BuiltinOpcode;

int decode_instruction(bytefile *bf, uint32_t offset, FILE *f, instr_info *info) {
  if (offset >= bf->code_size) {
    return -1;
  }

  char *pats[] = {"=str", "#string", "#array", "#sexp", "#ref", "#val", "#fun"};
  char *lds[] = {"LD", "LDA", "ST"};

  char *start = bf->code_ptr + offset;
  char *ip = start;
  char *end = bf->code_ptr + bf->code_size;

  char x = rd_byte(bf, &ip, end);
  char h = (x & 0xF0) >> 4;
  char l = x & 0x0F;

  instr_info local_info;
  local_info.target_count = 0;
  local_info.breaks_flow = 0;
  local_info.group = (uint8_t)h;
  local_info.opcode = (uint8_t)l;

  if (f != NULL) {
    fprintf(f, "0x%.8x:\t", offset);
  }

  switch ((OpcodeGroup)h) {
  case OG_BinOp:
    if (f != NULL) {
      char *ops[] = {"+", "-", "*", "/", "%", "<", "<=", ">", ">=", "==", "!=", "&&", "!!"};
      if (l == 0 || l > 13) {
        failure("ERROR: invalid opcode %d-%d\n", h, l);
      }
      fprintf(f, "BINOP\t%s", ops[(int)l - 1]);
    }
    break;

  case OG_Storage:
    switch ((StorageOpcode)l) {
    case ST_Const: {
      int32_t v = rd_int(bf, &ip, end);
      if (f != NULL)
        fprintf(f, "CONST\t%d", v);
    } break;

    case ST_String: {
      int32_t idx = rd_int(bf, &ip, end);
      if (idx < 0 || (uint32_t)idx >= bf->stringtab_size) {
        failure("ERROR: invalid string index %d\n", idx);
      }
      if (f != NULL)
        fprintf(f, "STRING\t%s", get_string(bf, (uint32_t)idx));
    } break;

    case ST_Sexp: {
      int32_t idx = rd_int(bf, &ip, end);
      if (idx < 0 || (uint32_t)idx >= bf->stringtab_size) {
        failure("ERROR: invalid string index %d\n", idx);
      }
      int32_t cnt = rd_int(bf, &ip, end);
      if (f != NULL)
        fprintf(f, "SEXP\t%s %d", get_string(bf, (uint32_t)idx), cnt);
    } break;

    case ST_StoreIndexed:
      if (f != NULL)
        fprintf(f, "STI");
      break;

    case ST_StoreArray:
      if (f != NULL)
        fprintf(f, "STA");
      break;

    case ST_Jump: {
      int32_t dst = rd_int(bf, &ip, end);
      if (dst < 0 || (uint32_t)dst >= bf->code_size) {
        failure("ERROR: invalid jump target %d\n", dst);
      }
      local_info.targets[local_info.target_count++] = (uint32_t)dst;
      local_info.breaks_flow = 1;
      if (f != NULL)
        fprintf(f, "JMP\t0x%.8x", dst);
    } break;

    case ST_End:
      if (f != NULL)
        fprintf(f, "END");
      local_info.breaks_flow = 1;
      break;

    case ST_Return:
      if (f != NULL)
        fprintf(f, "RET");
      local_info.breaks_flow = 1;
      break;

    case ST_Drop:
      if (f != NULL)
        fprintf(f, "DROP");
      break;

    case ST_Dup:
      if (f != NULL)
        fprintf(f, "DUP");
      break;

    case ST_Swap:
      if (f != NULL)
        fprintf(f, "SWAP");
      break;

    case ST_Elem:
      if (f != NULL)
        fprintf(f, "ELEM");
      break;

    default:
      failure("ERROR: invalid opcode %d-%d\n", h, l);
    }
    break;

  case OG_Load:
  case OG_LoadAddress:
  case OG_Store: {
    int32_t idx = rd_int(bf, &ip, end);
    if (f != NULL) {
      if ((AddressMode)l < Mode_Global || (AddressMode)l > Mode_Closure) {
        failure("ERROR: invalid opcode %d-%d\n", h, l);
      }
      fprintf(f, "%s\t", lds[h - OG_Load]);
      switch ((AddressMode)l) {
      case Mode_Global:
        fprintf(f, "G(%d)", idx);
        break;
      case Mode_Local:
        fprintf(f, "L(%d)", idx);
        break;
      case Mode_Argument:
        fprintf(f, "A(%d)", idx);
        break;
      case Mode_Closure:
        fprintf(f, "C(%d)", idx);
        break;
      }
    }
  } break;

  case OG_Control:
    switch ((ControlOpcode)l) {
    case Ctrl_JumpIfZero: {
      int32_t dst = rd_int(bf, &ip, end);
      if (dst < 0 || (uint32_t)dst >= bf->code_size) {
        failure("ERROR: invalid jump target %d\n", dst);
      }
      local_info.targets[local_info.target_count++] = (uint32_t)dst;
      if (f != NULL)
        fprintf(f, "CJMPz\t0x%.8x", dst);
    } break;

    case Ctrl_JumpIfNotZero: {
      int32_t dst = rd_int(bf, &ip, end);
      if (dst < 0 || (uint32_t)dst >= bf->code_size) {
        failure("ERROR: invalid jump target %d\n", dst);
      }
      local_info.targets[local_info.target_count++] = (uint32_t)dst;
      if (f != NULL)
        fprintf(f, "CJMPnz\t0x%.8x", dst);
    } break;

    case Ctrl_Begin: {
      int32_t a = rd_int(bf, &ip, end);
      int32_t b = rd_int(bf, &ip, end);
      if (f != NULL)
        fprintf(f, "BEGIN\t%d %d", a, b);
    } break;

    case Ctrl_BeginCaptured: {
      int32_t a = rd_int(bf, &ip, end);
      int32_t b = rd_int(bf, &ip, end);
      if (f != NULL)
        fprintf(f, "CBEGIN\t%d %d", a, b);
    } break;

    case Ctrl_Closure: {
      int32_t dst = rd_int(bf, &ip, end);
      int32_t n = rd_int(bf, &ip, end);
      if (dst < 0 || (uint32_t)dst >= bf->code_size) {
        failure("ERROR: invalid closure entry %d\n", dst);
      }
      if (n < 0) {
        failure("ERROR: negative capture count\n");
      }
      local_info.targets[local_info.target_count++] = (uint32_t)dst;
      if (f != NULL) {
        fprintf(f, "CLOSURE\t0x%.8x", dst);
        for (int i = 0; i < n; i++) {
          char mode = rd_byte(bf, &ip, end);
          int32_t idx = rd_int(bf, &ip, end);
          switch (mode) {
          case Mode_Global:
            fprintf(f, "G(%d)", idx);
            break;
          case Mode_Local:
            fprintf(f, "L(%d)", idx);
            break;
          case Mode_Argument:
            fprintf(f, "A(%d)", idx);
            break;
          case Mode_Closure:
            fprintf(f, "C(%d)", idx);
            break;
          default:
            failure("ERROR: invalid closure capture mode %d\n", mode);
          }
        }
      } else {
        for (int i = 0; i < n; i++) {
          (void)rd_byte(bf, &ip, end);
          (void)rd_int(bf, &ip, end);
        }
      }
    } break;

    case Ctrl_CallClosure: {
      int32_t dst = rd_int(bf, &ip, end);
      if (dst < 0 || (uint32_t)dst >= bf->code_size) {
        failure("ERROR: invalid CALLC target %d\n", dst);
      }
      local_info.targets[local_info.target_count++] = (uint32_t)dst;
      local_info.breaks_flow = 1;
      if (f != NULL)
        fprintf(f, "CALLC\t%d", dst);
    } break;

    case Ctrl_Call: {
      int32_t dst = rd_int(bf, &ip, end);
      int32_t argc = rd_int(bf, &ip, end);
      if (dst < 0 || (uint32_t)dst >= bf->code_size) {
        failure("ERROR: invalid CALL target %d\n", dst);
      }
      local_info.targets[local_info.target_count++] = (uint32_t)dst;
      local_info.breaks_flow = 1;
      if (f != NULL)
        fprintf(f, "CALL\t0x%.8x %d", dst, argc);
    } break;

    case Ctrl_Tag: {
      int32_t idx = rd_int(bf, &ip, end);
      int32_t n = rd_int(bf, &ip, end);
      if (idx < 0 || (uint32_t)idx >= bf->stringtab_size) {
        failure("ERROR: invalid string index %d\n", idx);
      }
      if (f != NULL)
        fprintf(f, "TAG\t%s %d", get_string(bf, (uint32_t)idx), n);
    } break;

    case Ctrl_Array: {
      int32_t n = rd_int(bf, &ip, end);
      if (f != NULL)
        fprintf(f, "ARRAY\t%d", n);
    } break;

    case Ctrl_Fail: {
      int32_t a = rd_int(bf, &ip, end);
      int32_t b = rd_int(bf, &ip, end);
      local_info.breaks_flow = 1;
      if (f != NULL)
        fprintf(f, "FAIL\t%d %d", a, b);
    } break;

    case Ctrl_Line: {
      int32_t line = rd_int(bf, &ip, end);
      if (f != NULL)
        fprintf(f, "LINE\t%d", line);
    } break;

    default:
      failure("ERROR: invalid opcode %d-%d\n", h, l);
    }
    break;

  case OG_Pattern:
    if (f != NULL) {
      if (l < 0 || l > 6) {
        failure("ERROR: invalid opcode %d-%d\n", h, l);
      }
      fprintf(f, "PATT\t%s", pats[(int)l]);
    }
    break;

  case OG_Builtin: {
    switch ((BuiltinOpcode)l) {
    case Builtin_Read:
      if (f != NULL)
        fprintf(f, "CALL\tLread");
      break;

    case Builtin_Write:
      if (f != NULL)
        fprintf(f, "CALL\tLwrite");
      break;

    case Builtin_Length:
      if (f != NULL)
        fprintf(f, "CALL\tLlength");
      break;

    case Builtin_ToString:
      if (f != NULL)
        fprintf(f, "CALL\tLstring");
      break;

    case Builtin_MakeArray: {
      int32_t n = rd_int(bf, &ip, end);
      if (f != NULL)
        fprintf(f, "CALL\tBarray\t%d", n);
    } break;

    default:
      failure("ERROR: invalid opcode %d-%d\n", h, l);
    }
  } break;

  case OG_Halt:
    if (f != NULL)
      fprintf(f, "<end>");
    local_info.breaks_flow = 1;
    break;

  default:
    failure("ERROR: invalid opcode %d-%d\n", h, l);
  }

  if (f != NULL) {
    fprintf(f, "\n");
  }

  local_info.size = (uint32_t)(ip - start);
  local_info.next_offset = offset + local_info.size;

  if (info != NULL) {
    *info = local_info;
  }
  return 0;
}

int decode_instruction_char(bytefile *bf, uint32_t offset, char *out, size_t out_size, instr_info *info) {
  if (out_size == 0) {
    return -1;
  }
  FILE *mem = fmemopen(out, out_size, "w");
  if (mem == NULL) {
    return -1;
  }
  int res = decode_instruction(bf, offset, mem, info);
  fclose(mem);
  return res;
}

/* Disassembles the bytecode pool */
void disassemble(FILE *f, bytefile *bf)
{
  uint32_t off = 0;
  instr_info info;
  while (off < bf->code_size) {
    if (decode_instruction(bf, off, f, &info) != 0) {
      break;
    }
    if (((bf->code_ptr[off] & 0xF0) >> 4) == 15) {
      break;
    }
    off = info.next_offset;
  }
}

/* Dumps the contents of the file */
void dump_file(FILE *f, bytefile *bf)
{
  uint32_t i;

  fprintf(f, "String table size       : %u\n", bf->stringtab_size);
  fprintf(f, "Global area size        : %u\n", bf->global_area_size);
  fprintf(f, "Number of public symbols: %u\n", bf->public_symbols_number);
  fprintf(f, "Public symbols          :\n");

  for (i = 0; i < bf->public_symbols_number; i++)
    fprintf(f, "   0x%.8x: %s\n", (unsigned int)get_public_offset(bf, i), get_public_name(bf, i));

  fprintf(f, "Code:\n");
  disassemble(f, bf);
}

#ifndef NO_MAIN
int main(int argc, char *argv[])
{
  bytefile *f = read_file(argv[1]);
  dump_file(stdout, f);
  return 0;
}
#endif
