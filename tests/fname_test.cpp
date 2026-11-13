#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

using std::min;
using PVOID = void*;
constexpr std::uint64_t va_text = 0x10000000;

namespace memory {
struct Read {
    std::uint64_t address;
    std::vector<unsigned char> bytes;
};
std::vector<Read> expected;
std::size_t next = 0;

void reset()
{
    assert(next == expected.size());
    expected.clear();
    next = 0;
}

template <typename T>
void expect(std::uint64_t address, T value)
{
    Read read{address, std::vector<unsigned char>(sizeof(T))};
    std::memcpy(read.bytes.data(), &value, sizeof(T));
    expected.push_back(read);
}

void copy(std::uint64_t address, void* output, std::size_t size)
{
    assert(next < expected.size());
    const auto& read = expected[next++];
    assert(address == read.address);
    assert(size == read.bytes.size());
    if (size != 0) std::memcpy(output, read.bytes.data(), size);
}
}

namespace DotMem {
template <typename T>
T Read(std::uint64_t address)
{
    T value{};
    memory::copy(address, &value, sizeof(T));
    return value;
}
}

namespace Driver {
void ReadPhysical(PVOID address, void* output, int size)
{
    assert(size >= 0);
    memory::copy(reinterpret_cast<std::uintptr_t>(address), output,
                 static_cast<std::size_t>(size));
}
}

#include "../FName.h"

namespace {
constexpr std::uint64_t primary_pool = va_text + 0x123FAFC0 + 16;
constexpr std::uint64_t redirect_pool = va_text + 0x124AE500 + 16;
const std::vector<unsigned char> alice{0xa2, 0x9b, 0x92, 0x58, 0xef};

void direct_name()
{
    memory::reset();
    memory::expect<std::uint64_t>(primary_pool + 8 * 2, 0x20000000);
    memory::expect<std::uint16_t>(0x20000000 + 2 * 7, (5 << 6) | 63);
    memory::expected.push_back({0x20000000 + 2 * 7 + 2, alice});
    assert(FName{(2 << 16) | 7}.ToString() == "Alice");
    memory::reset();
}

void redirected_name()
{
    memory::expect<std::uint64_t>(primary_pool + 8, 0x20000000);
    memory::expect<std::uint16_t>(0x20000000 + 2 * 3, 63);
    memory::expect<int>(0x20000000 + 2 * 3 + 2, (4 << 16) | 9);
    memory::expect<std::uint64_t>(redirect_pool + 8 * 4, 0x30000000);
    memory::expect<std::uint16_t>(0x30000000 + 2 * 9, 5 << 6);
    memory::expected.push_back({0x30000000 + 2 * 9 + 2, alice});
    assert(FName{(1 << 16) | 3}.ToString() == "Alice");
    memory::reset();
}

void empty_redirected_name()
{
    memory::expect<std::uint64_t>(primary_pool, 0x20000000);
    memory::expect<std::uint16_t>(0x20000000, 0);
    memory::expect<int>(0x20000000 + 2, 0);
    memory::expect<std::uint64_t>(redirect_pool, 0x30000000);
    memory::expect<std::uint16_t>(0x30000000, 0);
    memory::expected.push_back({0x30000000 + 2, {}});
    assert(FName{0}.ToString().empty());
    memory::reset();
}

void single_character()
{
    memory::expect<std::uint64_t>(primary_pool, 0x20000000);
    memory::expect<std::uint16_t>(0x20000000, 64);
    memory::expected.push_back({0x20000000 + 2, {0xa2}});
    assert(FName{0}.ToString() == "A");
    memory::reset();
}

void maximum_length()
{
    // A 16-bit header leaves ten bits for length: 1023, not NAME_SIZE (2048).
    const std::string plaintext(1023, 'A');
    std::vector<unsigned char> encrypted;
    unsigned int state = 28;
    for (unsigned int i = 0; i < plaintext.size(); ++i) {
        const auto key = (i | 0xB000u) + state;
        encrypted.push_back(static_cast<unsigned char>(~plaintext[i] ^ key));
        state = key >> 2;
    }
    memory::expect<std::uint64_t>(primary_pool, 0x20000000);
    memory::expect<std::uint16_t>(0x20000000, 0xffff);
    memory::expected.push_back({0x20000000 + 2, encrypted});
    assert(FName{0}.ToString() == plaintext);
    memory::reset();
}
}

int main()
{
    direct_name();
    redirected_name();
    empty_redirected_name();
    single_character();
    maximum_length();
    std::cout << "5 FName tests passed\n";
}
