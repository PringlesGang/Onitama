#pragma once

#include <bit>
#include <bitset>
#include <format>
#include <stdexcept>

namespace Base64 {

template <size_t N>
std::bitset<N> Decode(const char character) {
  switch (character) {
    case '+':
      return 62;
    case '/':
      return 63;

    default:
      if ('A' <= character && character <= 'Z') return character - 'A';
      if ('a' <= character && character <= 'z') return character - 'a' + 26;
      if ('0' <= character && character <= '9') return character - '0' + 26 * 2;
      throw std::runtime_error(
          std::format("Invalid base64 character '{}'", character));
  }
}

template <size_t N>
std::bitset<N> Decode(const std::string& string) {
  constexpr size_t charSize = std::bit_width(size_t{63});
  const size_t stringSize = string.size() * charSize;
  if (stringSize > N) {
    const std::string msg = std::format(
        "String contains {} > {} bits of information!", stringSize, N);
    throw std::runtime_error(msg);
  }

  std::bitset<N> bits;
  for (size_t i = 0; i < string.size(); i++) {
    const size_t offset = (string.size() - i - 1) * charSize;
    bits |= Decode<N>(string[i]) << offset;
  }

  return bits;
};

inline char Encode(const uint8_t bits) {
  switch (bits) {
    case 62:
      return '+';
    case 63:
      return '/';

    default:
      if (0 <= bits && bits <= 25) return 'A' + bits;
      if (26 <= bits && bits <= 51) return 'a' + bits - 26;
      if (52 <= bits && bits <= 61) return '0' + bits - 52;
      throw std::runtime_error(std::format("Invalid base64 id {}!", bits));
  }
}

template <size_t N>
std::string Encode(const std::bitset<N> bits) {
  if (bits.none()) return "A";

  constexpr size_t charSize = std::bit_width(size_t{63});

  size_t lastBit = bits.size() - 1;
  while (!bits[lastBit] && lastBit > 0) lastBit--;

  std::string string;

  uint8_t character = 0;
  for (size_t i = 0; i <= lastBit; i++) {
    character |= bits[i] << (i % charSize);

    if (i % charSize == charSize - 1 || i == lastBit) {
      string.insert(string.begin(), Encode(character));
      character = 0;
    }
  }

  return string;
}

}  // namespace Base64
