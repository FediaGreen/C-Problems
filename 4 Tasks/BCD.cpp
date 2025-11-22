#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

class BCD {
 public:
  BCD(const std::vector<int>& v, bool IsPositive, size_t precision) :
      digits_(v), IsPositive_(IsPositive), precision_(precision) {}

  BCD() : BCD(0) {}

  BCD(int num) {
    Set(num);
  }

  BCD(int num, bool IsPositive, size_t precision) :
    digits_({num}), IsPositive_(IsPositive), precision_(precision) { assert(Correct()); }

  BCD(const BCD& other) : 
    digits_(other.digits_), 
    IsPositive_(other.IsPositive_), 
    precision_(other.precision_) {}

  BCD(BCD&& other) :
    digits_(std::move(other.digits_)),
    IsPositive_(other.IsPositive_),
    precision_(other.precision_)
    {
      other.digits_.clear();
      other.IsPositive_ = true;
      other.precision_ = 0;
    }

  BCD& operator=(BCD&& other) noexcept {
    if (this != &other) {
      digits_ = std::move(other.digits_);
      IsPositive_ = other.IsPositive_;
      precision_ = other.precision_;

      other.digits_.clear();
      other.IsPositive_ = true;
      other.precision_ = 0;
    }
    return *this;
  }

  BCD Division(int num, size_t precision) { // example: 8643,23423 (BCD) / 123 = 70,270197...
    int cnt_num = 0;
    int divide = 0;
    bool IsPositive = true;
    std::vector <int> ans;
    if (num < 0) {
      IsPositive = false;
      num *= -1;
    }
    int term = 0;
    size_t count_after_point = 0;
    for (size_t i = 0; count_after_point <= precision; ++i) {
      if (i >= digits_.size()) {
        term = 0;
      } else {
        term = digits_[i];
      }
      cnt_num *= 10;
      cnt_num += term;
      divide = cnt_num / num;
      cnt_num -= divide * num;
      ans.push_back(divide);
      if (i >= digits_.size() - precision_) { 
        ++count_after_point;
        if (count_after_point == precision) {
          break;
        }
      }
    }
    return BCD(ans, IsPositive, precision);
  }

  std::string GetView() const {
    std::string view = "";
    if (!IsPositive_) {
      view += "-";
    }
    for (size_t i = 0; i < digits_.size(); ++i) {
      if (i == digits_.size() - precision_) {
        view += ",";
      }
      view += static_cast<char>(digits_[i] + 48);
    }
    return view;
  }

  BCD& operator=(const BCD& other) {
    digits_ = other.digits_;
    IsPositive_ = other.IsPositive_;
    precision_ = other.precision_;
    return *this;
  }

  BCD operator*(const BCD& other) {
    bool res_positive = IsPositive_ ^ other.IsPositive_;
    long long int this_ceil = GetCeil();
    long long int other_ceil = other.GetCeil();
    long long int max_ceil = std::max(this_ceil, other_ceil);
    int correct_precision = 1;
    correct_precision -= static_cast<int>(std::log10(max_ceil));
    correct_precision += std::min(int(precision_), int(other.precision_));
    std::vector<int> result(digits_.size() + other.digits_.size(), 0); // enought
    int product = 0;
    int sum = 0;

    for (int i = int(digits_.size() - 1); i >= 0; i--) {
      for (int j = int(other.digits_.size() - 1); j >= 0; j--) {
        product = digits_[size_t(i)] * other.digits_[size_t(j)];
        sum = product + result[size_t(i + j + 1)];
        result[size_t(i + j + 1)] = sum % 10;
        result[size_t(i + j)] += sum / 10;
      }
    }
    size_t start = 0;
    while (start < result.size() - 1 && result[start] == 0) {
      start++;
    }

    int lead_zeros = 0;
    for (size_t i = 0; i < result.size(); ++i) {
      if (result[i] == 0) {
        ++lead_zeros;
      } else {
        break;
      }
    }

    int cnt = int(result.size()) - lead_zeros;
    for (int i = 0; i < cnt - correct_precision; ++i) {
      result.pop_back();
    }

    return BCD(result, res_positive, size_t(correct_precision));
  }

  BCD operator+(const BCD& other) {
    if (IsPositive_ != other.IsPositive_) {
      bool this_larger = true;        
      for (size_t i = 0; i < std::max(digits_.size(), other.digits_.size()); ++i) {
        int this_digit = 0;
        if (i < digits_.size()) {
          this_digit = digits_[i];
        }
        int other_digit = 0;
        if (i < other.digits_.size()) {
          other_digit = other.digits_[i];
        }
        if (this_digit > other_digit) {
          this_larger = true;
          break;
        } else if (this_digit < other_digit) {
          this_larger = false;
          break;
        }
      }
      BCD larger{};
      BCD smaller{};
      if (this_larger) {
        larger = *this;
        smaller = other;
      } else {
        larger = other;
        smaller = *this;
      }
      int large_idx = int(larger.digits_.size() - 1);
      int small_idx = int(smaller.digits_.size() - 1);
      if (larger.precision_ > smaller.precision_) {
        large_idx -= int(larger.precision_ - smaller.precision_);
      } else if (smaller.precision_ > larger.precision_) {
        small_idx -= int(smaller.precision_ - larger.precision_);
      }
      std::vector<int> result;
      int borrow = 0;
      int large_digit = 0;
      int small_digit = 0;
      int diff = 0;
      while (large_idx >= 0 || small_idx >= 0) {
        if (large_idx >= 0) {
          large_digit = larger.digits_[size_t(large_idx)];
        } else {
          large_digit = 0;
        }
        if (small_idx >= 0) {
          small_digit = smaller.digits_[size_t(small_idx)];
        } else {
          small_digit = 0;
        }   
        diff = large_digit - small_digit - borrow;      
        if (diff < 0) {
          diff += 10;
          borrow = 1;
        } else {
          borrow = 0;
        }         
        result.push_back(diff);
        large_idx--;
        small_idx--;
      }
      while (result.size() > 1 && result.back() == 0) {
        result.pop_back();
      }
      std::reverse(result.begin(), result.end());
      if (this_larger) {
        return BCD(result, IsPositive_, std::min(precision_, other.precision_) - 1);
      }
      return BCD(result, other.IsPositive_, std::min(precision_, other.precision_) - 1);
    } else {
      bool result_sign = IsPositive_;
      int this_idx = 0;
      int other_idx = 0;
      if (other.precision_ < precision_) {
        other_idx = int(other.digits_.size() - 1);
        this_idx = int(digits_.size() - 1 + other.precision_- precision_);
      } else {
        this_idx = int(digits_.size() - 1);
        other_idx = int(other.digits_.size() - 1 + precision_ - other.precision_);
      }
      std::vector<int> ans;
      int this_term = 0;
      int other_term = 0;
      int token = 0;
      int digit_ans = 0;
      if (other.digits_[size_t(other_idx)] + digits_[size_t(this_idx)] >= 10) {
        token = 1;
      }
      --this_idx;
      --other_idx;
      for (size_t i = 0; i < std::max(digits_.size(), other.digits_.size()); ++i) {
        if (this_idx < 0) {
          this_term = 0;
        } else {
          this_term = digits_[size_t(this_idx)];
        }
        if (other_idx < 0) {
          other_term = 0;
        } else {
          other_term = other.digits_[size_t(other_idx)];
        }
        digit_ans = this_term + other_term + token;
        ans.push_back(digit_ans % 10);
        token = 0;
        if (digit_ans >= 10) {
          token = 1;
        }
        --this_idx;
        --other_idx;
      }
      size_t idx = ans.size() - 1;
      while(ans[idx] == 0) {
        ans.pop_back();
        if (idx == 0) {
          break;
        } else {
          --idx;
        }
      }
      std::reverse(ans.begin(), ans.end());
      return BCD(ans, result_sign, std::min(precision_, other.precision_) - 1);
    }
  }

  bool operator<(const BCD& other) const {
    if (!IsPositive_ && other.IsPositive_) {
      return true;
    }
    if (IsPositive_ && !other.IsPositive_) {
      return false;
    }
    size_t this_int_digits = digits_.size() - precision_;
    size_t other_int_digits = other.digits_.size() - other.precision_;    
    if (this_int_digits < other_int_digits) {
      return IsPositive_;
    }
    if (this_int_digits > other_int_digits) {
      return !IsPositive_;
    }
    for (size_t i = 0; i < this_int_digits; ++i) { // lenght are equils
      if (digits_[i] < other.digits_[i]) {
        return IsPositive_;
      }
      if (digits_[i] > other.digits_[i]) {
        return !IsPositive_;
      }
    }

    size_t min_precision = std::min(precision_, other.precision_);
    size_t this_frac_start = this_int_digits;
    size_t other_frac_start = other_int_digits;

    for (size_t i = 0; i < min_precision; ++i) {
      int this_digit = digits_[this_frac_start + i];
      int other_digit = other.digits_[other_frac_start + i];          
      if (this_digit < other_digit) {
        return IsPositive_;
      }
      if (this_digit > other_digit) {
        return !IsPositive_;
      }
    }
    return false; // BCD are equils
  }

 private:
  bool Correct() { return digits_.size() <= size_t(10 + precision_); }
  void SetPositive(int& num) {
    if (num < 0) {
      IsPositive_ = false;
      num *= -1;
    } else {
      IsPositive_ = true;
    }
  }

  void Set(int num) {
    if (num == 0) {
      digits_.push_back(0);
      IsPositive_ = true;
      return;
    }
    int n = static_cast<int>(num);
    precision_ = 0;
    SetPositive(n);
    int digit = 0; 
    digits_.clear();
    while (n >= 1) {
      digit = n % 10;
      digits_.push_back(digit);
      n /= 10;
    }
    std::reverse(digits_.begin(), digits_.end());
    assert(Correct());
  }
  long long int GetCeil() const {
    long long int ans = 0;
    for (size_t i = 0; i < digits_.size() - precision_; ++i) {
      ans = ans * 10 + digits_[i];
    }
    return ans;
  }

  std::vector<int> digits_; // all digits
  bool IsPositive_; // The number is positive or negative 
  size_t precision_; // number of digits after comma-point
};

int main() {
  std::vector<int> num(201, 0);
  std::vector<int> num1(301, 0);
  num[0] = 1;
  num1[0] = 1;
  BCD e_num(num, true, 200);
  BCD curr_BCD(num1, true, 300);
  for (size_t i = 1; i <= 100; ++i) {
    curr_BCD = curr_BCD.Division(int(i), 300);
    e_num = e_num + curr_BCD;
  }
  std::cout << e_num.GetView();
}
