#include <Buffer.hh>

Lie::Buffer::Buffer(int capacity)
{
    _size = 0;
    _capacity = capacity;
    _data = new char[_capacity];
}

Lie::Buffer::~Buffer()
{
    delete[] _data;
}

auto Lie::Buffer::Size() const -> int
{
    return _size;
}

auto Lie::Buffer::Capacity() const -> int
{
    return _capacity;
}

auto Lie::Buffer::Data() const -> const char*
{
    return _data;
}

auto Lie::Buffer::Clear() -> void
{
    _size = 0;
}

auto Lie::Buffer::Append(char c) -> void
{
    if (_size >= _capacity)
        Resize(_capacity * 2);

    _data[_size++] = c;
}

auto Lie::Buffer::Append(const std::string& string) -> void
{
    if (_size + string.size() > _capacity)
        Resize(_capacity * 2);

    for (auto c : string)
        _data[_size++] = c;
}

auto Lie::Buffer::Resize(int capacity) -> void
{
    char* data = new char[capacity];
    for (int i = 0; i < _size; i++)
        data[i] = _data[i];

    delete[] _data;
    _data = data;
    _capacity = capacity;
}
