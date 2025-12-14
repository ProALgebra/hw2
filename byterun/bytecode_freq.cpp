#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <exception>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

extern "C" void failure(const char *fmt, ...);

#include "byterun.h"

extern "C" void failure(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), fmt, ap);
    va_end(ap);
    throw std::runtime_error(buffer);
}

namespace {

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

enum class AddressMode : uint8_t { Global = 0, Local = 1, Argument = 2, Closure = 3 };

enum class LoadOpcode : uint8_t { Load = 2, LoadAddress = 3, Store = 4 };

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

enum class BuiltinOpcode : uint8_t { Read = 0, Write = 1, Length = 2, ToString = 3, MakeArray = 4 };

struct Instruction {
    uint32_t offset = 0;
    std::string text;
};

struct Reader {
    explicit Reader(bytefile *bf_ptr)
        : bf(bf_ptr), ip(bf_ptr->code_ptr), end(bf_ptr->code_ptr + bf_ptr->code_size) {}

    bool has_more() const { return ip < end; }

    uint32_t offset() const { return static_cast<uint32_t>(ip - bf->code_ptr); }

    uint32_t code_size() const { return bf->code_size; }

    void ensure(size_t bytes) const {
        if (static_cast<size_t>(end - ip) < bytes) {
            std::ostringstream msg;
            msg << "unexpected end of bytecode at offset 0x" << std::hex << std::setw(8)
                << std::setfill('0') << offset();
            throw std::runtime_error(msg.str());
        }
    }

    uint8_t byte() {
        ensure(1);
        return static_cast<uint8_t>(*ip++);
    }

    int32_t int32() {
        ensure(sizeof(int32_t));
        int32_t value;
        std::memcpy(&value, ip, sizeof(value));
        ip += sizeof(int32_t);
        return value;
    }

    const char *string() {
        int32_t raw_index = int32();
        if (raw_index < 0) {
            std::ostringstream msg;
            msg << "negative string index " << raw_index << " at offset 0x" << std::hex
                << std::setw(8) << std::setfill('0') << offset();
            throw std::runtime_error(msg.str());
        }
        uint32_t pos = static_cast<uint32_t>(raw_index);
        if (pos >= bf->stringtab_size) {
            std::ostringstream msg;
            msg << "string index " << pos << " is out of range at offset 0x" << std::hex
                << std::setw(8) << std::setfill('0') << offset();
            throw std::runtime_error(msg.str());
        }
        return get_string(bf, pos);
    }

    bytefile *bf;
    const char *ip;
    const char *end;
};

std::string hex_offset(uint32_t offset) {
    std::ostringstream out;
    out << "0x" << std::hex << std::setw(8) << std::setfill('0') << offset;
    return out.str();
}

std::string escape_string(const char *raw, bytefile *bf) {
    const uint32_t start_index = static_cast<uint32_t>(raw - bf->string_ptr);
    const uint32_t max_len =
        start_index < bf->stringtab_size ? bf->stringtab_size - start_index : 0U;

    std::ostringstream out;
    out << '"';
    for (uint32_t i = 0; i < max_len && raw[i] != '\0'; ++i) {
        unsigned char ch = static_cast<unsigned char>(raw[i]);
        switch (ch) {
        case '\\':
            out << "\\\\";
            break;
        case '"':
            out << "\\\"";
            break;
        case '\n':
            out << "\\n";
            break;
        case '\t':
            out << "\\t";
            break;
        case '\r':
            out << "\\r";
            break;
        default:
            if (std::isprint(ch)) {
                out << raw[i];
            } else {
                out << "\\x" << std::hex << std::setw(2) << std::setfill('0')
                    << static_cast<int>(ch) << std::dec;
            }
        }
    }
    out << '"';
    return out.str();
}

std::string join(const std::vector<std::string> &items, const char *delim) {
    std::ostringstream out;
    for (size_t i = 0; i < items.size(); ++i) {
        if (i > 0) {
            out << delim;
        }
        out << items[i];
    }
    return out.str();
}

std::string mode_name(AddressMode mode) {
    switch (mode) {
    case AddressMode::Global:
        return "G";
    case AddressMode::Local:
        return "L";
    case AddressMode::Argument:
        return "A";
    case AddressMode::Closure:
        return "C";
    }
    std::ostringstream msg;
    msg << "unknown address mode " << static_cast<int>(mode);
    throw std::runtime_error(msg.str());
}

void add_label(uint32_t offset, const Reader &r, std::set<uint32_t> &labels,
               const std::string &context) {
    if (offset >= r.code_size()) {
        std::ostringstream msg;
        msg << context << " target " << hex_offset(offset) << " is out of code range";
        throw std::runtime_error(msg.str());
    }
    labels.insert(offset);
}

std::string decode_binop(uint8_t l) {
    static const char *names[] = {"add", "sub", "mul", "div", "mod", "lt",  "le",
                                  "gt",  "ge",  "eq",  "ne",  "and", "or"};
    if (l == 0 || l > sizeof(names) / sizeof(names[0])) {
        std::ostringstream msg;
        msg << "unknown BINOP opcode " << static_cast<int>(l);
        throw std::runtime_error(msg.str());
    }
    std::ostringstream out;
    out << "BINOP " << names[l - 1];
    return out.str();
}

std::string decode_storage(uint8_t l, Reader &r, std::set<uint32_t> &labels) {
    switch (static_cast<StorageOpcode>(l)) {
    case StorageOpcode::Const: {
        int32_t value = r.int32();
        return "CONST " + std::to_string(value);
    }
    case StorageOpcode::String: {
        const char *s = r.string();
        return std::string("STRING ") + escape_string(s, r.bf);
    }
    case StorageOpcode::Sexp: {
        const char *tag = r.string();
        int32_t count = r.int32();
        std::ostringstream out;
        out << "SEXP " << escape_string(tag, r.bf) << " " << count;
        return out.str();
    }
    case StorageOpcode::StoreIndexed:
        return "STI";
    case StorageOpcode::StoreArray:
        return "STA";
    case StorageOpcode::Jump: {
        int32_t target = r.int32();
        if (target < 0) {
            throw std::runtime_error("negative jump target");
        }
        uint32_t off = static_cast<uint32_t>(target);
        add_label(off, r, labels, "jump");
        return "JMP " + hex_offset(off);
    }
    case StorageOpcode::End:
        return "END";
    case StorageOpcode::Return:
        return "RET";
    case StorageOpcode::Drop:
        return "DROP";
    case StorageOpcode::Dup:
        return "DUP";
    case StorageOpcode::Swap:
        return "SWAP";
    case StorageOpcode::Elem:
        return "ELEM";
    default: {
        std::ostringstream msg;
        msg << "unknown storage opcode " << static_cast<int>(l);
        throw std::runtime_error(msg.str());
    }
    }
}

std::string decode_load(uint8_t h, uint8_t l, Reader &r) {
    auto mode = static_cast<AddressMode>(l);
    switch (mode) {
    case AddressMode::Global:
    case AddressMode::Local:
    case AddressMode::Argument:
    case AddressMode::Closure:
        break;
    default: {
        std::ostringstream msg;
        msg << "unknown address mode " << static_cast<int>(l);
        throw std::runtime_error(msg.str());
    }
    }
    int32_t index = r.int32();
    static const char *ops[] = {"LD", "LDA", "ST"};
    const char *op = ops[h - static_cast<uint8_t>(OpcodeGroup::Load)];
    std::ostringstream out;
    out << op << " " << mode_name(mode) << "(" << index << ")";
    return out.str();
}

std::string decode_control(uint8_t l, Reader &r, std::set<uint32_t> &labels) {
    switch (static_cast<ControlOpcode>(l)) {
    case ControlOpcode::JumpIfZero: {
        int32_t target = r.int32();
        if (target < 0) {
            throw std::runtime_error("negative jump-if-zero target");
        }
        uint32_t off = static_cast<uint32_t>(target);
        add_label(off, r, labels, "jump-if-zero");
        return "CJMPZ " + hex_offset(off);
    }
    case ControlOpcode::JumpIfNotZero: {
        int32_t target = r.int32();
        if (target < 0) {
            throw std::runtime_error("negative jump-if-not-zero target");
        }
        uint32_t off = static_cast<uint32_t>(target);
        add_label(off, r, labels, "jump-if-not-zero");
        return "CJMPNZ " + hex_offset(off);
    }
    case ControlOpcode::Begin: {
        int32_t arg_slots = r.int32();
        int32_t var_slots = r.int32();
        std::ostringstream out;
        out << "BEGIN args=" << arg_slots << " locals=" << var_slots;
        return out.str();
    }
    case ControlOpcode::BeginCaptured: {
        int32_t arg_slots = r.int32();
        int32_t var_slots = r.int32();
        std::ostringstream out;
        out << "CBEGIN args=" << arg_slots << " locals=" << var_slots;
        return out.str();
    }
    case ControlOpcode::Closure: {
        int32_t raw = r.int32();
        if (raw < 0) {
            throw std::runtime_error("negative closure entry");
        }
        uint32_t entry = static_cast<uint32_t>(raw);
        add_label(entry, r, labels, "closure");
        int32_t count = r.int32();
        if (count < 0) {
            throw std::runtime_error("negative capture count");
        }
        std::vector<std::string> captures;
        captures.reserve(static_cast<size_t>(count));
        for (int32_t i = 0; i < count; ++i) {
            uint8_t tag = r.byte();
            auto mode = static_cast<AddressMode>(tag);
            int32_t idx = r.int32();
            captures.push_back(mode_name(mode) + "(" + std::to_string(idx) + ")");
        }
        std::ostringstream out;
        out << "CLOSURE " << hex_offset(entry) << " captures=" << count;
        if (!captures.empty()) {
            out << " [" << join(captures, ", ") << "]";
        }
        return out.str();
    }
    case ControlOpcode::CallClosure: {
        int32_t argc = r.int32();
        std::ostringstream out;
        out << "CALLC args=" << argc;
        return out.str();
    }
    case ControlOpcode::Call: {
        int32_t raw = r.int32();
        if (raw < 0) {
            throw std::runtime_error("negative call target");
        }
        uint32_t target = static_cast<uint32_t>(raw);
        add_label(target, r, labels, "call");
        int32_t argc = r.int32();
        std::ostringstream out;
        out << "CALL " << hex_offset(target) << " args=" << argc;
        return out.str();
    }
    case ControlOpcode::Tag: {
        const char *name = r.string();
        int32_t fields = r.int32();
        std::ostringstream out;
        out << "TAG " << escape_string(name, r.bf) << " " << fields;
        return out.str();
    }
    case ControlOpcode::Array: {
        int32_t length = r.int32();
        return "ARRAY " + std::to_string(length);
    }
    case ControlOpcode::Fail: {
        int32_t a = r.int32();
        int32_t b = r.int32();
        std::ostringstream out;
        out << "FAIL " << a << " " << b;
        return out.str();
    }
    case ControlOpcode::Line: {
        int32_t line = r.int32();
        return "LINE " + std::to_string(line);
    }
    default: {
        std::ostringstream msg;
        msg << "unknown control opcode " << static_cast<int>(l);
        throw std::runtime_error(msg.str());
    }
    }
}

std::string decode_pattern(uint8_t l) {
    switch (static_cast<PatternOpcode>(l)) {
    case PatternOpcode::StringMatch:
        return "PATT =str";
    case PatternOpcode::StringTag:
        return "PATT #string";
    case PatternOpcode::ArrayTag:
        return "PATT #array";
    case PatternOpcode::Unboxed:
        return "PATT #val";
    case PatternOpcode::ClosureTag:
        return "PATT #fun";
    default: {
        std::ostringstream msg;
        msg << "unknown pattern opcode " << static_cast<int>(l);
        throw std::runtime_error(msg.str());
    }
    }
}

std::string decode_builtin(uint8_t l, Reader &r) {
    switch (static_cast<BuiltinOpcode>(l)) {
    case BuiltinOpcode::Read:
        return "BUILTIN read";
    case BuiltinOpcode::Write:
        return "BUILTIN write";
    case BuiltinOpcode::Length:
        return "BUILTIN length";
    case BuiltinOpcode::ToString:
        return "BUILTIN string";
    case BuiltinOpcode::MakeArray: {
        int32_t len = r.int32();
        return "BUILTIN make_array " + std::to_string(len);
    }
    default: {
        std::ostringstream msg;
        msg << "unknown builtin opcode " << static_cast<int>(l);
        throw std::runtime_error(msg.str());
    }
    }
}

std::string decode_instruction(Reader &r, std::set<uint32_t> &labels) {
    uint32_t start_offset = r.offset();
    uint8_t opcode = r.byte();
    uint8_t h = static_cast<uint8_t>((opcode & 0xF0u) >> 4);
    uint8_t l = static_cast<uint8_t>(opcode & 0x0Fu);

    switch (static_cast<OpcodeGroup>(h)) {
    case OpcodeGroup::BinOp:
        return decode_binop(l);
    case OpcodeGroup::Storage:
        return decode_storage(l, r, labels);
    case OpcodeGroup::Load:
    case OpcodeGroup::LoadAddress:
    case OpcodeGroup::Store:
        return decode_load(h, l, r);
    case OpcodeGroup::Control:
        return decode_control(l, r, labels);
    case OpcodeGroup::Pattern:
        return decode_pattern(l);
    case OpcodeGroup::Builtin:
        return decode_builtin(l, r);
    case OpcodeGroup::Halt:
        return "HALT";
    default: {
        std::ostringstream msg;
        msg << "unknown opcode group h=" << static_cast<int>(h) << " l=" << static_cast<int>(l)
            << " at offset " << hex_offset(start_offset);
        throw std::runtime_error(msg.str());
    }
    }
}

std::vector<std::pair<std::string, size_t>>
collect_frequencies(const std::vector<Instruction> &program, const std::set<uint32_t> &labels) {
    std::map<std::string, size_t> freq;
    for (const auto &ins : program) {
        freq["[1] " + ins.text] += 1;
    }

    for (size_t i = 0; i + 1 < program.size(); ++i) {
        if (labels.count(program[i + 1].offset) != 0U) {
            continue; 
        }
        std::string seq = "[2] " + program[i].text + " | " + program[i + 1].text;
        freq[seq] += 1;
    }

    std::vector<std::pair<std::string, size_t>> ordered(freq.begin(), freq.end());
    std::sort(ordered.begin(), ordered.end(),
              [](const auto &lhs, const auto &rhs) {
                  if (lhs.second != rhs.second) {
                      return lhs.second > rhs.second;
                  }
                  return lhs.first < rhs.first;
              });
    return ordered;
}

} 

int main(int argc, char *argv[]) {
    try {
        bytefile *bf = read_file(argv[1]);
        Reader reader(bf);
        std::set<uint32_t> labels;
        std::vector<Instruction> program;

        while (reader.has_more()) {
            Instruction ins;
            ins.offset = reader.offset();
            ins.text = decode_instruction(reader, labels);
            program.push_back(std::move(ins));
        }

        auto ordered = collect_frequencies(program, labels);
        for (const auto &entry : ordered) {
            std::cout << entry.second << '\t' << entry.first << '\n';
        }
    } catch (const std::exception &e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
