#include "Buffer.hpp"
#include <memory>

struct CommandBufferMock : public vka::BufferDependent{
  void notify() {
  }
  std::shared_ptr<vka::Buffer<int, 5>> buffer;
};

int main()
{

  vka::Buffer<int, 5> buffer;
  buffer.setData(0, 0, 5);

  return 0; 
}