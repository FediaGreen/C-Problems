#include <array>
#include <cstdint>
#include <iostream>
#include <string>

class IPv4 {
 public:
  void UpdateNotation() {
    notation_ = (static_cast<uint32_t>(octets_[0]) << offsets_[0]) |
                (static_cast<uint32_t>(octets_[1]) << offsets_[1]) |
                (static_cast<uint32_t>(octets_[2]) << offsets_[2]) |
                (static_cast<uint32_t>(octets_[3]) << offsets_[3]);
  }

  void Normalise() {
    octets_[0] = uint8_t((notation_ >> offsets_[0]) & 0xFF);
    octets_[1] = uint8_t((notation_ >> offsets_[1]) & 0xFF);
    octets_[2] = uint8_t((notation_ >> offsets_[2]) & 0xFF);
    octets_[3] = uint8_t((notation_ >> offsets_[3]) & 0xFF);
  }

  IPv4& operator++() {
    notation_++;
    Normalise();
    return *this;
  }

  IPv4 operator++(int) {
    IPv4 old = *this;
    ++notation_;
    Normalise();
    return old;
  }

  IPv4& operator--() {
    notation_--;
    Normalise();
    return *this;
  }

  IPv4 operator--(int) {
    IPv4 old = *this;
    --notation_;
    Normalise();
    return old;
  }

  friend bool operator<(IPv4& ip1, IPv4& ip2);

  friend std::ostream& operator<<(std::ostream& os, IPv4& ip);
  
  friend std::istream& operator>>(std::istream& in, IPv4& ip);

 private:
  std::array<std::uint8_t, 4> octets_{0, 0, 0, 0};
  std::array<std::uint8_t, 4> offsets_{24, 16, 8, 0};
  uint32_t notation_ = 0;
};

bool operator<(IPv4& ip1, IPv4& ip2) {
  return (ip1.notation_) < (ip2.notation_);
}

std::ostream& operator<<(std::ostream& os, IPv4& ip) {
  for (size_t i = 0; i < ip.octets_.size(); ++i) {
    os << static_cast<int>(ip.octets_[i]);
    if (i + 1 != ip.octets_.size()) {
      os << ".";
    }
  }
  return os;
}

uint8_t Tokenizer(std::string notation, size_t comma_idx) {
  const int cOffset = 48;
  uint8_t value = 0;
  size_t idx = comma_idx + 1;

  while (idx < notation.size()) {
    if (notation[idx] == '.') {
      break;
    }
    value = uint8_t(value * 10 + (static_cast<uint8_t>(notation[idx]) - cOffset));
    ++idx;
  }

  return value;
}

std::istream& operator>>(std::istream& in, IPv4& ip) {
  std::string notation = "";
  std::string notation_modified = ".";
  size_t octet_idx = 0;

  in >> notation;
  notation_modified += notation;
  notation_modified += '.';

  for (size_t i = 0; i < notation_modified.size() - 1; ++i) {
    if (notation_modified[i] == '.') {
      ip.octets_[octet_idx] = Tokenizer(notation_modified, i);
      ++octet_idx;
    }
  }

  ip.UpdateNotation();

  return in;
}

int main() {
  IPv4 ip;

  std::cin >> ip;
  std::cout << ip << '\n';

  ip++;
  std::cout << ip << '\n';

  ip--;
  std::cout << ip << '\n'; // same with first
}
