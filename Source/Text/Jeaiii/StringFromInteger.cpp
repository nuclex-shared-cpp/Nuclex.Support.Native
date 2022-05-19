#include <cstdint>
#include <cstring>
#include <type_traits>

static constexpr char radix_100_table[] = {
  '0', '0', '0', '1', '0', '2', '0', '3', '0', '4',
  '0', '5', '0', '6', '0', '7', '0', '8', '0', '9',
  '1', '0', '1', '1', '1', '2', '1', '3', '1', '4',
  '1', '5', '1', '6', '1', '7', '1', '8', '1', '9',
  '2', '0', '2', '1', '2', '2', '2', '3', '2', '4',
  '2', '5', '2', '6', '2', '7', '2', '8', '2', '9',
  '3', '0', '3', '1', '3', '2', '3', '3', '3', '4',
  '3', '5', '3', '6', '3', '7', '3', '8', '3', '9',
  '4', '0', '4', '1', '4', '2', '4', '3', '4', '4',
  '4', '5', '4', '6', '4', '7', '4', '8', '4', '9',
  '5', '0', '5', '1', '5', '2', '5', '3', '5', '4',
  '5', '5', '5', '6', '5', '7', '5', '8', '5', '9',
  '6', '0', '6', '1', '6', '2', '6', '3', '6', '4',
  '6', '5', '6', '6', '6', '7', '6', '8', '6', '9',
  '7', '0', '7', '1', '7', '2', '7', '3', '7', '4',
  '7', '5', '7', '6', '7', '7', '7', '8', '7', '9',
  '8', '0', '8', '1', '8', '2', '8', '3', '8', '4',
  '8', '5', '8', '6', '8', '7', '8', '8', '8', '9',
  '9', '0', '9', '1', '9', '2', '9', '3', '9', '4',
  '9', '5', '9', '6', '9', '7', '9', '8', '9', '9'
};

//      /\____________
//     /  \______     \______
//    /\   \     \     \     \
//   0  1  /\    /\    /\    /\
//        2  3  4  5  6  7  8  9
char *itoa_better_y(std::uint32_t n, char *buffer) {
  std::uint64_t prod;

  auto get_next_two_digits = [&]() {
    prod = std::uint32_t(prod) * std::uint64_t(100);
    return int(prod >> 32);
  };
  auto print_1 = [&](int digit) {
    buffer[0] = char(digit + '0');
    buffer += 1;
  };
  auto print_2 = [&](int two_digits) {
    std::memcpy(buffer, radix_100_table + two_digits * 2, 2);
    buffer += 2;
  };
  auto print = [&](std::uint64_t magic_number, int extra_shift, auto remaining_count) {
    prod = n * magic_number;
    prod >>= extra_shift;
    auto two_digits = int(prod >> 32);

    if(two_digits < 10) {
      print_1(two_digits);
      for(int i = 0; i < remaining_count; ++i) {
        print_2(get_next_two_digits());
      }
    } else {
      print_2(two_digits);
      for(int i = 0; i < remaining_count; ++i) {
        print_2(get_next_two_digits());
      }
    }
  };

  if(n < 100) {
    if(n < 10) {
      // 1 digit.
      print_1(n);
    } else {
      // 2 digit.
      print_2(n);
    }
  } else {
    if(n < 100'0000) {
      if(n < 1'0000) {
        // 3 or 4 digits.
        // 42949673 = ceil(2^32 / 10^2)
        print(42949673, 0, std::integral_constant<int, 1>{});
      } else {
        // 5 or 6 digits.
        // 429497 = ceil(2^32 / 10^4)
        print(429497, 0, std::integral_constant<int, 2>{});
      }
    } else {
      if(n < 1'0000'0000) {
        // 7 or 8 digits.
        // 281474978 = ceil(2^48 / 10^6) + 1
        print(281474978, 16, std::integral_constant<int, 3>{});
      } else {
        if(n < 10'0000'0000) {
          // 9 digits.
          // 1441151882 = ceil(2^57 / 10^8) + 1
          prod = n * std::uint64_t(1441151882);
          prod >>= 25;
          print_1(int(prod >> 32));
          print_2(get_next_two_digits());
          print_2(get_next_two_digits());
          print_2(get_next_two_digits());
          print_2(get_next_two_digits());
        } else {
          // 10 digits.
          // 1441151881 = ceil(2^57 / 10^8)
          prod = n * std::uint64_t(1441151881);
          prod >>= 25;
          print_2(int(prod >> 32));
          print_2(get_next_two_digits());
          print_2(get_next_two_digits());
          print_2(get_next_two_digits());
          print_2(get_next_two_digits());
        }
      }
    }
  }
  return buffer;
}

/*
#include <iostream>

int main() {
  char buffer[11];
  *itoa_better_y(1, buffer) = '\0';
  std::cout << buffer << "\n";
  *itoa_better_y(12, buffer) = '\0';
  std::cout << buffer << "\n";
  *itoa_better_y(123, buffer) = '\0';
  std::cout << buffer << "\n";
  *itoa_better_y(1234, buffer) = '\0';
  std::cout << buffer << "\n";
  *itoa_better_y(12345, buffer) = '\0';
  std::cout << buffer << "\n";
  *itoa_better_y(123456, buffer) = '\0';
  std::cout << buffer << "\n";
  *itoa_better_y(1234567, buffer) = '\0';
  std::cout << buffer << "\n";
  *itoa_better_y(12345678, buffer) = '\0';
  std::cout << buffer << "\n";
  *itoa_better_y(123456789, buffer) = '\0';
  std::cout << buffer << "\n";
  *itoa_better_y(1234567890, buffer) = '\0';
  std::cout << buffer << "\n";
}
*/