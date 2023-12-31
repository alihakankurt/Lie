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

      private:
        int _size;
        int _capacity;
        char* _data;

        auto Resize(int capacity) -> void;
    };
}