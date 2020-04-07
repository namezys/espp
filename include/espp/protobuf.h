#pragma once

#include <string>
#include <vector>

#define PROTO_NAME(x) x ## Msg

#define PROTO_CONSTRUCT(x, p) PROTO_NAME(x) (): x{} { p##__init(this); }
#define PROTO_CONSTRUCT_NESTED(x, p, xx) xx (): x{} { p##__init(this); }
#define PROTO_DECONSTRUCT(x, p) ~PROTO_NAME(x) () { if (_is_unpacked) { p##__free_unpacked(this, nullptr);} }
#define PROTO_NO_COPY(x) \
    PROTO_NAME(x) (const PROTO_NAME(x)&) = delete; \
    PROTO_NAME(x)& operator=(const PROTO_NAME(x)&) = delete;

#define PROTO_PACK(p, x) \
    std::vector<uint8_t> Pack() const { \
        std::size_t size = p##__get_packed_size(this); std::vector<uint8_t> buffer(size); \
        p##__pack(this, buffer.data()); return buffer; \
    } \
    void Unpack(const std::vector<uint8_t>& buffer) { \
        assert(!_is_unpacked); \
        *static_cast<x*>(this) = *p##__unpack(nullptr, buffer.size(), buffer.data()); \
        _is_unpacked = true; \
    }

#define PROTO(x, p) struct PROTO_NAME(x): public x { \
    bool _is_unpacked = false; \
    PROTO_CONSTRUCT(x, p) \
    PROTO_DECONSTRUCT(x, p) \
    PROTO_NO_COPY(x) \
    PROTO_PACK(p, x)

#define PROTO_NESTED(x, p, xx) struct xx: public x { \
    bool _is_unpacked = false; \
    PROTO_CONSTRUCT_NESTED(x, p, xx) \
    x* ptr() {return this;}

#define PROTO_STR(attr) \
    std::string _internal##attr; \
    void set_##attr(const std::string& v) { _internal##attr = v; attr = const_cast<char*>(_internal##attr.c_str()); } \
    void reset_##attr() { attr = nullptr; }

#define PROTO_VALUE(attr) \
    template<class T> void set_##attr(T v) { attr = v; has_##attr = true; } \
    void reset_##attr() { attr = {}; has_##attr = false; }

#define PROTO_MSG(attr, cls) \
    cls _internal##attr; \
    void set_##attr(const cls& v) { _internal##attr = v; attr = _internal##attr.ptr(); } \
    cls& get_##attr() { attr = _internal##attr.ptr(); return _internal##attr; } \
    void reset_##attr() { attr = nullptr; }
