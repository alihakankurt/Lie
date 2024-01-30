#pragma once

#include <Core.hh>

namespace Lie
{
    class Buffer
    {
      public:
        Buffer(int capacity = 16);
        ~Buffer();

        auto Size() const -> int;
        auto Capacity() const -> int;
        auto Data() const -> const char*;

        auto Clear() -> void;

        auto Append(char c) -> void;
        auto Append(const std::string& string) -> void;
        auto Append(Buffer& other) -> void;

        auto Insert(int index, char c) -> void;
        auto Insert(int index, const std::string& string) -> void;
        auto Insert(int index, Buffer& other) -> void;

        auto Remove(int index) -> void;
        auto Remove(int index, int count) -> void;

      private:
        int _size;
        int _capacity;
        char* _data;

        auto Resize(int capacity) -> void;
    };
}