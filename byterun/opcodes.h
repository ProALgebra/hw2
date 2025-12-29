#pragma once
#include <cstdint>

enum class BinOp : uint8_t {
    BinOpAdd = 0,
    BinOpSub = 1,
    BinOpMul = 2,
    BinOpDiv = 3,
    BinOpMod = 4,
    BinOpLt = 5,
    BinOpLe = 6,
    BinOpGt = 7,
    BinOpGe = 8,
    BinOpEq = 9,
    BinOpNe = 10,
    BinOpAnd = 11,
    BinOpOr = 12
};

enum class OpcodeGroup : uint8_t {
    BinOp = 0,
    Storage = 1,
    Load = 2,
    LoadAddress = 3,
    Store = 4,
    Control = 5,
    Pattern = 6,
    Builtin = 7,
    Halt = 15
};

enum class StorageOpcode : uint8_t {
    Const = 0,
    String = 1,
    Sexp = 2,
    StoreIndexed = 3,
    StoreArray = 4,
    Jump = 5,
    End = 6,
    Return = 7,
    Drop = 8,
    Dup = 9,
    Swap = 10,
    Elem = 11
};

enum class AddressMode : uint8_t {
    Global = 0,
    Local = 1,
    Argument = 2,
    Closure = 3
};

enum class LoadOpcode : uint8_t {
    Load = 2,
    LoadAddress = 3,
    Store = 4
};

enum class ControlOpcode : uint8_t {
    JumpIfZero = 0,
    JumpIfNotZero = 1,
    Begin = 2,
    BeginCaptured = 3,
    Closure = 4,
    CallClosure = 5,
    Call = 6,
    Tag = 7,
    Array = 8,
    Fail = 9,
    Line = 10
};

enum class PatternOpcode : uint8_t {
    StringMatch = 0,
    StringTag = 1,
    ArrayTag = 2,
    Unboxed = 5,
    ClosureTag = 6
};

enum class BuiltinOpcode : uint8_t {
    Read = 0,
    Write = 1,
    Length = 2,
    ToString = 3,
    MakeArray = 4
};
