#pragma once
#include <memory>

namespace vka {
class RenderContext;

class RenderObject {
public:
  void validate() { validateImpl(); }
  void invalidate() { invalidateImpl(); }

protected:
  bool isValid = false;

private:
  virtual void validateImpl() = 0;
  virtual void invalidateImpl() { isValid = false; }
};
}  // namespace vka