struct DeviceBuffer {
  int marker;

  DeviceBuffer() : marker(7) {}

  void set(int value) {
    marker = value;
  }
};

template<typename T>
struct BaseFromMember {
  T member;

  BaseFromMember() : member() {}
};

struct PrimaryStream {
  virtual ~PrimaryStream() {}
  int sentinel;

  PrimaryStream() : sentinel(99) {}
};

struct StreamBase : protected BaseFromMember<DeviceBuffer>, public PrimaryStream {
  using BaseFromMember<DeviceBuffer>::member;

  StreamBase() : BaseFromMember<DeviceBuffer>(), PrimaryStream() {}

  long member_delta() {
    return (char *)&this->member - (char *)this;
  }

  int read() {
    return this->member.marker;
  }

  void write(int value) {
    this->member.set(value);
  }
};

struct Stream : StreamBase {
  long derived_delta() {
    return (char *)&this->member - (char *)this;
  }

  int read_derived() {
    return this->member.marker;
  }

  void write_derived(int value) {
    this->member.set(value);
  }
};

int main() {
  Stream stream;
  if(stream.member_delta() <= 0) {
    return 1;
  }
  if(stream.derived_delta() != stream.member_delta()) {
    return 2;
  }
  if(stream.read() != 7) {
    return 3;
  }
  stream.write_derived(42);
  if(stream.read() != 42) {
    return 4;
  }
  stream.write(13);
  if(stream.read_derived() != 13) {
    return 5;
  }
  return 0;
}
