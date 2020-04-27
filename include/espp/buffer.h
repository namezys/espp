#pragma once

#include <string>
#include <cstring>
#include <ostream>
#include <cassert>

namespace espp {

/**
 * Store buffer as pointer and length.
 *
 * Doesn't own it
 */
class Buffer{
public:
    template<class T>
    Buffer(T* data, std::size_t length)
    {
        Set(data, length);
    }

    Buffer(const char* c_str)
    {
        Set(c_str, std::strlen(c_str));
    }

    Buffer(const std::string& str, bool with_zero_terminal = false)
    {
        Set(str.c_str(), str.size() + (with_zero_terminal ? 1 : 0));
    }

    Buffer(const Buffer&) = delete;

    Buffer(Buffer&&) = delete;

    std::size_t CopyTo(uint8_t* buffer, std::size_t bufferLength, bool truncate = false) const
    {
        assert(truncate || length() <= bufferLength);
        const auto copyLength = std::min(bufferLength, length());
        std::memcpy(buffer, data(), copyLength);
        return copyLength;
    }

    std::size_t StringCopyTo(uint8_t* buffer, std::size_t bufferLength, bool truncate = false) const
    {
        const auto result = CopyTo(buffer, bufferLength - 1, truncate);
        buffer[result] = 0;
        return result;
    }

    const uint8_t* data() const
    {
        return _data;
    }

    const char* charData() const
    {
        return reinterpret_cast<const char*>(_data);
    }

    std::size_t length() const
    {
        return _length;
    }

    bool empty() const
    {
        return length() == 0;
    }

    std::string str() const
    {
        return {charData(), length()};
    }

    operator std::string() const
    {
        return str();
    }

protected:
    Buffer() = default;

    template<class T>
    void Set(T* data, std::size_t src_length)
    {
        _data = reinterpret_cast<const uint8_t*>(data);
        _length = sizeof(T) * src_length;
        assert(_length >= 0 && _data != nullptr);
    }

private:
    const uint8_t* _data = nullptr;
    std::size_t _length = 0;
};

inline
std::ostream& operator<<(std::ostream& s, const Buffer& buffer)
{
    if(!buffer.empty()) {
        s.write(buffer.charData(), buffer.length());
    }
    return s;
}

/**
 * Own string and store buffer
 */
class StringBuffer: public Buffer{
public:
    StringBuffer() = default;

    StringBuffer(std::string str):
        _str(std::move(str))
    {
        Set(_str.c_str(), _str.length());
    }

    StringBuffer(const uint8_t* data, std::size_t length):
        _str(reinterpret_cast<const char*>(data), length)
    {
        Set(_str.c_str(), _str.length());
    }

    StringBuffer(const char* data):
        _str(data)
    {
        Set(_str.c_str(), _str.length());
    }


    StringBuffer(const uint8_t* data):
        StringBuffer(reinterpret_cast<const char*>(data))
    {
    }

    StringBuffer(const StringBuffer& other):
        StringBuffer(other._str)
    {
    }

    StringBuffer(StringBuffer&& other) noexcept:
        StringBuffer(other._str)
    {
    }

private:
    const std::string _str;
};

}
