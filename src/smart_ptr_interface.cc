
#include <memory>
#include <vector>
#include "smart_ptr_interface.h"

using std::vector;
using std::shared_ptr;

namespace clidoc {

// definition of static data member.
vector<shared_ptr<void>> SPIStaticDataMember::cached_container_;

void SPIStaticDataMember::FreeCached() {
  cached_container_.clear();
}

}  // namespace clidoc