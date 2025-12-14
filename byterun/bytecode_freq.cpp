#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <exception>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
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

class StringPool {
  public:
    uint32_t intern(const std::string &value) {
        auto it = ids.find(value);
        if (it != ids.end()) {
            return it->second;
        }
        uint32_t id = static_cast<uint32_t>(items.size());
        items.push_back(value);
        ids.emplace(items.back(), id);
        return id;
    }

    const std::string &get(uint32_t id) const {
        if (id >= items.size()) {
            throw std::runtime_error("string pool index out of range");
        }
        return items[id];
    }

  private:
    std::vector<std::string> items;
    std::unordered_map<std::string, uint32_t> ids;
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

enum class BinOpCode : uint8_t {
    BinOpAdd = 1,
    BinOpSub = 2,
    BinOpMul = 3,
    BinOpDiv = 4,
    BinOpMod = 5,
    BinOpLt = 6,
    BinOpLe = 7,
    BinOpGt = 8,
    BinOpGe = 9,
    BinOpEq = 10,
    BinOpNe = 11,
    BinOpAnd = 12,
    BinOpOr = 13
};

enum class InsKind : uint8_t {
    BinOp,
    StorageConst,
    StorageString,
    StorageSexp,
    StorageStoreIndexed,
    StorageStoreArray,
    StorageJump,
    StorageEnd,
    StorageReturn,
    StorageDrop,
    StorageDup,
    StorageSwap,
    StorageElem,
    Load,
    LoadAddress,
    Store,
    ControlJumpIfZero,
    ControlJumpIfNotZero,
    ControlBegin,
    ControlBeginCaptured,
    ControlClosure,
    ControlCallClosure,
    ControlCall,
    ControlTag,
    ControlArray,
    ControlFail,
    ControlLine,
    PatternStringMatch,
    PatternStringTag,
    PatternArrayTag,
    PatternUnboxed,
    PatternClosureTag,
    BuiltinRead,
    BuiltinWrite,
    BuiltinLength,
    BuiltinToString,
    BuiltinMakeArray,
    Halt
};

struct Capture {
    AddressMode mode;
    int32_t index;
};

struct Instruction {
    uint32_t offset = 0;
    InsKind kind = InsKind::Halt;
    BinOpCode binop = BinOpCode::BinOpAdd;                  
    AddressMode mode = AddressMode::Global; // For LD/LDA/ST
    int32_t a = 0;                      
    int32_t b = 0;                     
    uint32_t str_id = UINT32_MAX;       // String operands (string literals / tags)
    std::vector<Capture> captures;      // For closures
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

Instruction decode_instruction(Reader &r, std::set<uint32_t> &labels, StringPool &literal_pool) {
    Instruction ins;
    ins.offset = r.offset();

    uint8_t opcode = r.byte();
    uint8_t h = static_cast<uint8_t>((opcode & 0xF0u) >> 4);
    uint8_t l = static_cast<uint8_t>(opcode & 0x0Fu);

    switch (static_cast<OpcodeGroup>(h)) {
    case OpcodeGroup::BinOp: {
        switch (static_cast<BinOpCode>(l)) {
        case BinOpCode::BinOpAdd:
        case BinOpCode::BinOpSub:
        case BinOpCode::BinOpMul:
        case BinOpCode::BinOpDiv:
        case BinOpCode::BinOpMod:
        case BinOpCode::BinOpLt:
        case BinOpCode::BinOpLe:
        case BinOpCode::BinOpGt:
        case BinOpCode::BinOpGe:
        case BinOpCode::BinOpEq:
        case BinOpCode::BinOpNe:
        case BinOpCode::BinOpAnd:
        case BinOpCode::BinOpOr:
            ins.kind = InsKind::BinOp;
            ins.binop = static_cast<BinOpCode>(l);
            break;
        default:
            throw std::runtime_error("unknown BINOP opcode");
        }
        break;
    }
    case OpcodeGroup::Storage: {
        switch (static_cast<StorageOpcode>(l)) {
        case StorageOpcode::Const:
            ins.kind = InsKind::StorageConst;
            ins.a = r.int32();
            break;
        case StorageOpcode::String:
            ins.kind = InsKind::StorageString;
            ins.str_id = literal_pool.intern(escape_string(r.string(), r.bf));
            break;
        case StorageOpcode::Sexp:
            ins.kind = InsKind::StorageSexp;
            ins.str_id = literal_pool.intern(escape_string(r.string(), r.bf));
            ins.a = r.int32();
            break;
        case StorageOpcode::StoreIndexed:
            ins.kind = InsKind::StorageStoreIndexed;
            break;
        case StorageOpcode::StoreArray:
            ins.kind = InsKind::StorageStoreArray;
            break;
        case StorageOpcode::Jump: {
            int32_t target = r.int32();
            if (target < 0) {
                throw std::runtime_error("negative jump target");
            }
            uint32_t off = static_cast<uint32_t>(target);
            add_label(off, r, labels, "jump");
            ins.kind = InsKind::StorageJump;
            ins.a = target;
            break;
        }
        case StorageOpcode::End:
            ins.kind = InsKind::StorageEnd;
            break;
        case StorageOpcode::Return:
            ins.kind = InsKind::StorageReturn;
            break;
        case StorageOpcode::Drop:
            ins.kind = InsKind::StorageDrop;
            break;
        case StorageOpcode::Dup:
            ins.kind = InsKind::StorageDup;
            break;
        case StorageOpcode::Swap:
            ins.kind = InsKind::StorageSwap;
            break;
        case StorageOpcode::Elem:
            ins.kind = InsKind::StorageElem;
            break;
        default:
            throw std::runtime_error("unknown storage opcode");
        }
        break;
    }
    case OpcodeGroup::Load:
    case OpcodeGroup::LoadAddress:
    case OpcodeGroup::Store: {
        auto mode = static_cast<AddressMode>(l);
        switch (mode) {
        case AddressMode::Global:
        case AddressMode::Local:
        case AddressMode::Argument:
        case AddressMode::Closure:
            break;
        default:
            throw std::runtime_error("unknown address mode");
        }
        ins.mode = mode;
        ins.a = r.int32();
        if (h == static_cast<uint8_t>(OpcodeGroup::Load)) {
            ins.kind = InsKind::Load;
        } else if (h == static_cast<uint8_t>(OpcodeGroup::LoadAddress)) {
            ins.kind = InsKind::LoadAddress;
        } else {
            ins.kind = InsKind::Store;
        }
        break;
    }
    case OpcodeGroup::Control: {
        switch (static_cast<ControlOpcode>(l)) {
        case ControlOpcode::JumpIfZero: {
            int32_t target = r.int32();
            if (target < 0) {
                throw std::runtime_error("negative jump-if-zero target");
            }
            uint32_t off = static_cast<uint32_t>(target);
            add_label(off, r, labels, "jump-if-zero");
            ins.kind = InsKind::ControlJumpIfZero;
            ins.a = target;
            break;
        }
        case ControlOpcode::JumpIfNotZero: {
            int32_t target = r.int32();
            if (target < 0) {
                throw std::runtime_error("negative jump-if-not-zero target");
            }
            uint32_t off = static_cast<uint32_t>(target);
            add_label(off, r, labels, "jump-if-not-zero");
            ins.kind = InsKind::ControlJumpIfNotZero;
            ins.a = target;
            break;
        }
        case ControlOpcode::Begin:
            ins.kind = InsKind::ControlBegin;
            ins.a = r.int32();
            ins.b = r.int32();
            break;
        case ControlOpcode::BeginCaptured:
            ins.kind = InsKind::ControlBeginCaptured;
            ins.a = r.int32();
            ins.b = r.int32();
            break;
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
            ins.kind = InsKind::ControlClosure;
            ins.a = raw;
            ins.b = count;
            ins.captures.reserve(static_cast<size_t>(count));
            for (int32_t i = 0; i < count; ++i) {
                uint8_t tag = r.byte();
                auto mode = static_cast<AddressMode>(tag);
                int32_t idx = r.int32();
                ins.captures.push_back({mode, idx});
            }
            break;
        }
        case ControlOpcode::CallClosure:
            ins.kind = InsKind::ControlCallClosure;
            ins.a = r.int32();
            break;
        case ControlOpcode::Call: {
            int32_t raw = r.int32();
            if (raw < 0) {
                throw std::runtime_error("negative call target");
            }
            uint32_t target = static_cast<uint32_t>(raw);
            add_label(target, r, labels, "call");
            ins.kind = InsKind::ControlCall;
            ins.a = raw;
            ins.b = r.int32();
            break;
        }
        case ControlOpcode::Tag:
            ins.kind = InsKind::ControlTag;
            ins.str_id = literal_pool.intern(escape_string(r.string(), r.bf));
            ins.a = r.int32();
            break;
        case ControlOpcode::Array:
            ins.kind = InsKind::ControlArray;
            ins.a = r.int32();
            break;
        case ControlOpcode::Fail:
            ins.kind = InsKind::ControlFail;
            ins.a = r.int32();
            ins.b = r.int32();
            break;
        case ControlOpcode::Line:
            ins.kind = InsKind::ControlLine;
            ins.a = r.int32();
            break;
        default:
            throw std::runtime_error("unknown control opcode");
        }
        break;
    }
    case OpcodeGroup::Pattern: {
        switch (static_cast<PatternOpcode>(l)) {
        case PatternOpcode::StringMatch:
            ins.kind = InsKind::PatternStringMatch;
            break;
        case PatternOpcode::StringTag:
            ins.kind = InsKind::PatternStringTag;
            break;
        case PatternOpcode::ArrayTag:
            ins.kind = InsKind::PatternArrayTag;
            break;
        case PatternOpcode::Unboxed:
            ins.kind = InsKind::PatternUnboxed;
            break;
        case PatternOpcode::ClosureTag:
            ins.kind = InsKind::PatternClosureTag;
            break;
        default:
            throw std::runtime_error("unknown pattern opcode");
        }
        break;
    }
    case OpcodeGroup::Builtin: {
        switch (static_cast<BuiltinOpcode>(l)) {
        case BuiltinOpcode::Read:
            ins.kind = InsKind::BuiltinRead;
            break;
        case BuiltinOpcode::Write:
            ins.kind = InsKind::BuiltinWrite;
            break;
        case BuiltinOpcode::Length:
            ins.kind = InsKind::BuiltinLength;
            break;
        case BuiltinOpcode::ToString:
            ins.kind = InsKind::BuiltinToString;
            break;
        case BuiltinOpcode::MakeArray:
            ins.kind = InsKind::BuiltinMakeArray;
            ins.a = r.int32();
            break;
        default:
            throw std::runtime_error("unknown builtin opcode");
        }
        break;
    }
    case OpcodeGroup::Halt:
        ins.kind = InsKind::Halt;
        break;
    default: {
        std::ostringstream msg;
        msg << "unknown opcode group h=" << static_cast<int>(h) << " l=" << static_cast<int>(l)
            << " at offset " << hex_offset(ins.offset);
        throw std::runtime_error(msg.str());
    }
    }

    return ins;
}

std::string format_instruction(const Instruction &ins, const StringPool &literal_pool) {
    switch (ins.kind) {
    case InsKind::BinOp: {
        switch (ins.binop) {
        case BinOpCode::BinOpAdd:  return "BINOP add";
        case BinOpCode::BinOpSub:  return "BINOP sub";
        case BinOpCode::BinOpMul:  return "BINOP mul";
        case BinOpCode::BinOpDiv:  return "BINOP div";
        case BinOpCode::BinOpMod:  return "BINOP mod";
        case BinOpCode::BinOpLt:   return "BINOP lt";
        case BinOpCode::BinOpLe:   return "BINOP le";
        case BinOpCode::BinOpGt:   return "BINOP gt";
        case BinOpCode::BinOpGe:   return "BINOP ge";
        case BinOpCode::BinOpEq:   return "BINOP eq";
        case BinOpCode::BinOpNe:   return "BINOP ne";
        case BinOpCode::BinOpAnd:  return "BINOP and";
        case BinOpCode::BinOpOr:   return "BINOP or";
        default: return "BINOP ?";
        }
    }
    case InsKind::StorageConst:
        return "CONST " + std::to_string(ins.a);
    case InsKind::StorageString:
        return "STRING " + literal_pool.get(ins.str_id);
    case InsKind::StorageSexp:
        return "SEXP " + literal_pool.get(ins.str_id) + " " + std::to_string(ins.a);
    case InsKind::StorageStoreIndexed:
        return "STI";
    case InsKind::StorageStoreArray:
        return "STA";
    case InsKind::StorageJump:
        return "JMP " + hex_offset(static_cast<uint32_t>(ins.a));
    case InsKind::StorageEnd:
        return "END";
    case InsKind::StorageReturn:
        return "RET";
    case InsKind::StorageDrop:
        return "DROP";
    case InsKind::StorageDup:
        return "DUP";
    case InsKind::StorageSwap:
        return "SWAP";
    case InsKind::StorageElem:
        return "ELEM";
    case InsKind::Load:
        return "LD " + mode_name(ins.mode) + "(" + std::to_string(ins.a) + ")";
    case InsKind::LoadAddress:
        return "LDA " + mode_name(ins.mode) + "(" + std::to_string(ins.a) + ")";
    case InsKind::Store:
        return "ST " + mode_name(ins.mode) + "(" + std::to_string(ins.a) + ")";
    case InsKind::ControlJumpIfZero:
        return "CJMPZ " + hex_offset(static_cast<uint32_t>(ins.a));
    case InsKind::ControlJumpIfNotZero:
        return "CJMPNZ " + hex_offset(static_cast<uint32_t>(ins.a));
    case InsKind::ControlBegin: {
        std::ostringstream out;
        out << "BEGIN args=" << ins.a << " locals=" << ins.b;
        return out.str();
    }
    case InsKind::ControlBeginCaptured: {
        std::ostringstream out;
        out << "CBEGIN args=" << ins.a << " locals=" << ins.b;
        return out.str();
    }
    case InsKind::ControlClosure: {
        std::vector<std::string> caps;
        caps.reserve(ins.captures.size());
        for (const auto &c : ins.captures) {
            caps.push_back(mode_name(c.mode) + "(" + std::to_string(c.index) + ")");
        }
        std::ostringstream out;
        out << "CLOSURE " << hex_offset(static_cast<uint32_t>(ins.a)) << " captures=" << ins.b;
        if (!caps.empty()) {
            out << " [";
            for (size_t i = 0; i < caps.size(); ++i) {
                if (i > 0) {
                    out << ", ";
                }
                out << caps[i];
            }
            out << "]";
        }
        return out.str();
    }
    case InsKind::ControlCallClosure:
        return "CALLC args=" + std::to_string(ins.a);
    case InsKind::ControlCall: {
        std::ostringstream out;
        out << "CALL " << hex_offset(static_cast<uint32_t>(ins.a)) << " args=" << ins.b;
        return out.str();
    }
    case InsKind::ControlTag:
        return "TAG " + literal_pool.get(ins.str_id) + " " + std::to_string(ins.a);
    case InsKind::ControlArray:
        return "ARRAY " + std::to_string(ins.a);
    case InsKind::ControlFail: {
        std::ostringstream out;
        out << "FAIL " << ins.a << " " << ins.b;
        return out.str();
    }
    case InsKind::ControlLine:
        return "LINE " + std::to_string(ins.a);
    case InsKind::PatternStringMatch:
        return "PATT =str";
    case InsKind::PatternStringTag:
        return "PATT #string";
    case InsKind::PatternArrayTag:
        return "PATT #array";
    case InsKind::PatternUnboxed:
        return "PATT #val";
    case InsKind::PatternClosureTag:
        return "PATT #fun";
    case InsKind::BuiltinRead:
        return "BUILTIN read";
    case InsKind::BuiltinWrite:
        return "BUILTIN write";
    case InsKind::BuiltinLength:
        return "BUILTIN length";
    case InsKind::BuiltinToString:
        return "BUILTIN string";
    case InsKind::BuiltinMakeArray:
        return "BUILTIN make_array " + std::to_string(ins.a);
    case InsKind::Halt:
        return "HALT";
    }
    return "<unknown>";
}

struct FrequencyEntry {
    std::string text;
    size_t count;
};

std::vector<FrequencyEntry> collect_frequencies(const std::vector<Instruction> &program,
                                                const std::vector<uint32_t> &text_ids,
                                                const std::set<uint32_t> &labels,
                                                const StringPool &text_pool) {
    std::unordered_map<uint32_t, size_t> uni;
    std::unordered_map<uint64_t, size_t> bi;

    for (uint32_t id : text_ids) {
        ++uni[id];
    }

    for (size_t i = 0; i + 1 < program.size(); ++i) {
        if (labels.count(program[i + 1].offset) != 0U) {
            continue; 
        }
        uint32_t id1 = text_ids[i];
        uint32_t id2 = text_ids[i + 1];
        uint64_t key = (static_cast<uint64_t>(id1) << 32) | id2;
        ++bi[key];
    }

    std::vector<FrequencyEntry> out;
    out.reserve(uni.size() + bi.size());

    for (const auto &entry : uni) {
        FrequencyEntry e;
        e.count = entry.second;
        e.text = "[1] " + text_pool.get(entry.first);
        out.push_back(std::move(e));
    }

    for (const auto &entry : bi) {
        uint32_t id1 = static_cast<uint32_t>(entry.first >> 32);
        uint32_t id2 = static_cast<uint32_t>(entry.first & 0xFFFFFFFFu);
        FrequencyEntry e;
        e.count = entry.second;
        e.text = "[2] " + text_pool.get(id1) + " | " + text_pool.get(id2);
        out.push_back(std::move(e));
    }

    std::sort(out.begin(), out.end(), [](const FrequencyEntry &lhs, const FrequencyEntry &rhs) {
        if (lhs.count != rhs.count) {
            return lhs.count > rhs.count;
        }
        return lhs.text < rhs.text;
    });

    return out;
}

} 

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <bytecode.bc>\n";
        return 1;
    }

    try {
        bytefile *bf = read_file(argv[1]);
        Reader reader(bf);
        StringPool literal_pool;
        std::set<uint32_t> labels;
        std::vector<Instruction> program;

        while (reader.has_more()) {
            Instruction ins = decode_instruction(reader, labels, literal_pool);
            program.push_back(std::move(ins));
        }

        StringPool text_pool;
        std::vector<uint32_t> text_ids;
        text_ids.reserve(program.size());

        for (const auto &ins : program) {
            std::string text = format_instruction(ins, literal_pool);
            text_ids.push_back(text_pool.intern(text));
        }

        auto ordered = collect_frequencies(program, text_ids, labels, text_pool);
        for (const auto &entry : ordered) {
            std::cout << entry.count << '\t' << entry.text << '\n';
        }
    } catch (const std::exception &e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
