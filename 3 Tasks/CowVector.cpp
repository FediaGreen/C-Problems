#include <cassert>
#include <iostream>
#include <vector>

class Vector {
 public:
  Vector(size_t len) {
    Data data;
    data.capacity = len;
    data.write_ptr = new double[data.capacity];
    for_delete_.push_back(data.write_ptr);
    data.is_shared = false;
    version_ = data;
  }
    
  Vector(const std::vector<double>& other_vector) {
    Data data;
    data.capacity = other_vector.size();
    data.write_ptr = new double[data.capacity];
    for_delete_.push_back(data.write_ptr);
    std::copy(other_vector.begin(), other_vector.end(), data.write_ptr);
    data.is_shared = false;
    version_ = data;
  }
    
  Vector(const Vector& other_vector) { // copy-cow constructor
    const Data& other_data = other_vector.version_;
    CowCopy(other_data);
  }

  Vector(const Vector& other_vector, size_t offset, size_t size) { // offset constructor
    const Data& other_data = other_vector.version_;
    CowCopy(other_data, offset, size);
  }

  double operator[](size_t ind) const {
    const Data& data = version_;
    if (data.is_shared) {
      return data.read_ptr[ind];
    } else {
      return data.write_ptr[ind];
    }
  }

  double& Write(size_t ind) {
    if (version_.is_shared) {
      AddVersion(version_);
    }
    return version_.write_ptr[ind];
  }
    
  Vector& operator=(const Vector& other_vector) { // Cow assignment
    if (this != &other_vector) {
      const Data& other_data = other_vector.version_;
      CowCopy(other_data);
      }
    return *this;
  }

  size_t size() const {
    const Data& data = version_;
    return data.capacity;
  }

  ~Vector() {
    for (auto& ptr: for_delete_) {
      if (ptr) {
        delete[] ptr;
      }
    }
  }

  class Buffer {
   public:
    Buffer(Vector& v) : vector_(v), data_(v.AcquireBuffer()) {}

    ~Buffer() {
      vector_.ReleaseBuffer();
    }

    Buffer(const Buffer&) = delete;

    Buffer& operator=(const Buffer&) = delete;

    double* data() {
      return data_;
    }

    size_t size() {
      return vector_.size();
    }

   private:
    Vector& vector_;
    double* data_;
  };

  class ConstBuffer {
   public:
    ConstBuffer(const Vector& vec) : vector_(vec) {
      data_ = vector_.AcquireConstBuffer();
    }

    ~ConstBuffer() {
      vector_.ReleaseConstBuffer();
    }

    ConstBuffer(const Buffer&) = delete;

    ConstBuffer& operator=(const Buffer&) = delete;

    const double* data() {
      return data_;
    }

    size_t size() const { 
      return vector_.size();
    }

   private:
    const Vector& vector_;
    const double* data_;
  };

  double* AcquireBuffer() {
    check_lock();
    AddVersion(version_);
    write_locked_ = true;
    return version_.write_ptr;
  }

  const double* AcquireConstBuffer() const {
    check_lock();
    const Data& data = version_;
    const_locked_ = true;
    if (data.is_shared) {
      return data.read_ptr;
    }
    return data.write_ptr;
  }

  void ReleaseBuffer() {
    write_locked_ = false;
  }

  void ReleaseConstBuffer() const {
    const_locked_ = false;
  }

  Buffer acquire_buffer() { 
    return Buffer(*this);
  }

  ConstBuffer acquire_const_buffer() {
    return ConstBuffer(*this);
  }

 private:
  struct Data {
    double* write_ptr = nullptr;
    const double* read_ptr = nullptr;
    bool is_shared = true;
    size_t capacity = 0;
  };

  void AddVersion(Data& last_data) { // add standalone version of vector
    Data data;
    data.capacity = last_data.capacity;
    data.write_ptr = new double[data.capacity];
    if (last_data.is_shared) {
      std::copy(last_data.read_ptr, last_data.read_ptr + last_data.capacity, data.write_ptr);
    } else {
      std::copy(last_data.write_ptr, last_data.write_ptr + last_data.capacity, data.write_ptr);  
    }
    data.is_shared = false;
    for_delete_.push_back(data.write_ptr);
    version_ = data;
    std::cout << "AddVersion" << '\n';
  }

  void CowCopy(const Data& other_data, size_t offset = 0, size_t size = 0) {
    Data data;
    data.capacity = size;
    if (size == 0) {
      data.capacity = other_data.capacity;
    }
    data.is_shared = true;
    if (other_data.is_shared) {
      data.read_ptr = other_data.read_ptr + offset;
    } else {
      data.read_ptr = other_data.write_ptr + offset;
    }
    version_ = data;
    std::cout << "CowCopy" << '\n';
  }


  Data version_;
  std::vector<double*> for_delete_;
  mutable bool write_locked_ = false;
  mutable bool const_locked_ = false;

  void check_lock() const {
    assert(!write_locked_ && !const_locked_);
  }
};

int main() {
  std::vector<double> v = {1, 2, 3, 4, 5};
  Vector vec(v);
  Vector vec1 = vec;
  {
    Vector::ConstBuffer buf = vec.acquire_const_buffer();
    [[maybe_unused]] const double* raw_ptr = buf.data();
    // raw_ptr[0] = raw_ptr[0] + 1; // error;
  }
  {
    Vector::Buffer buf = vec.acquire_buffer();
    double* raw_ptr = buf.data();
    for (size_t i = 0; i < buf.size(); i++) {
      raw_ptr[i] = raw_ptr[i] + 1;
    }
  }


  assert(!(vec1[0] == vec[0])); 

  assert(vec[0] == 2);

  assert(vec1[0] == 1);

  Vector vec2(vec1);

  assert(vec2[0] == 1);

  vec2 = vec;

  assert(vec2[0] == 2);

  Vector vec3 = {1, 2, 3};
  

  std::cout << '\n';

  vec3.Write(0) = 0;
  vec3.Write(1) = 0;
  vec3.Write(2) = 0;
}
