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
    Resize(_capacity + 1);
    _data[_size++] = c;
}

auto Lie::Buffer::Append(const std::string& string) -> void
{
    Resize(_size + string.size());

    for (auto c : string)
        _data[_size++] = c;
}

auto Lie::Buffer::Append(Buffer& other) -> void
{
    Resize(_size + other.Size());

    for (int i = 0; i < other.Size(); i++)
        _data[_size++] = other.Data()[i];
}

auto Lie::Buffer::Insert(int index, char c) -> void
{
    Resize(_size + 1);

    for (int i = _size; i > index; i--)
        _data[i] = _data[i - 1];

    _data[index] = c;
    _size++;
}

auto Lie::Buffer::Insert(int index, const std::string& string) -> void
{
    Resize(_size + string.size());

    for (int i = _size; i > index; i--)
        _data[i + string.size() - 1] = _data[i - 1];

    for (int i = 0; i < string.size(); i++)
        _data[index + i] = string[i];

    _size += string.size();
}

auto Lie::Buffer::Insert(int index, Buffer& other) -> void
{
    Resize(_size + other.Size());

    for (int i = _size; i > index; i--)
        _data[i + other.Size() - 1] = _data[i - 1];

    for (int i = 0; i < other.Size(); i++)
        _data[index + i] = other.Data()[i];

    _size += other.Size();
}

auto Lie::Buffer::Remove(int index) -> void
{
    for (int i = index; i < _size - 1; i++)
        _data[i] = _data[i + 1];

    _size--;
}

auto Lie::Buffer::Remove(int index, int count) -> void
{
    for (int i = index; i < _size - count; i++)
        _data[i] = _data[i + count];

    _size -= count;
}

auto Lie::Buffer::Resize(int capacity) -> void
{
    if (capacity <= _capacity)
        return;

    capacity = std::max(capacity, _capacity * 2);
    char* data = new char[capacity];
    for (int i = 0; i < _size; i++)
        data[i] = _data[i];

    delete[] _data;
    _data = data;
    _capacity = capacity;
}
