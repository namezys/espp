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
    Buffer(T* data, std::size_t length):
        _data(reinterpret_cast<const uint8_t*>(data)),
        _length(sizeof(T) * length)
    {
        assert(_length >= 0 && _data != nullptr);
    }

    /** Expect zero terminated string */
    Buffer(const char* c_str):
        Buffer(c_str, std::strlen(c_str))
    {
    }

    Buffer(const std::string& str):
        Buffer(str.c_str(), str.size())
    {
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

    bool operator==(const Buffer& o) const
    {
        return length() == o.length() && std::strncmp(charData(), o.charData(), std::min(length(), o.length())) == 0;
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

/**
 * Contains zero-terminated string. Length include zero terminate
 *
 */
class StringBuffer: public Buffer {
public:
    StringBuffer(const std::string& str):
        Buffer(str.c_str(), str.size() + 1)
    {}

    StringBuffer(const char* str):
        Buffer(str, std::strlen(str) + 1)
    {}

    Buffer buffer() const
    {
        return {data(), length() - 1};
    }

protected:
    StringBuffer() = delete;
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
class Data: public Buffer {
public:
    Data() = default;

    Data(std::string str):
        _str(std::move(str))
    {
        Set(_str.c_str(), _str.length());
    }

    Data(const uint8_t* data, std::size_t length):
        _str(reinterpret_cast<const char*>(data), length)
    {
        Set(_str.c_str(), _str.length());
    }

    Data(const char* data):
        _str(data)
    {
        Set(_str.c_str(), _str.length());
    }


    Data(const uint8_t* data):
        Data(reinterpret_cast<const char*>(data))
    {
    }

    Data(const Data& other):
        Data(other._str)
    {
    }

    Data(Data&& other) noexcept:
        Data(other._str)
    {
    }

private:
    const std::string _str;
};

}
