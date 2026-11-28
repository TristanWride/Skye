#include "skye/Component.hpp"

#include <atomic>

namespace skye::detail {

ComponentTypeID NextComponentTypeID() noexcept {
  // Atomic because this counter is process-wide, shared by every registry and system on every thread.
  static auto next = std::atomic<ComponentTypeID>{ 0 };
  return next.fetch_add( 1, std::memory_order_relaxed );
}

}
