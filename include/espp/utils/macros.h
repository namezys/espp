#include <cstdlib>

#include "espp/log.h"

#define ESPP_CHECK(x) if (!(x)) { ROW_LOG << "CRITICAL ASSERT AT" <<  __FILE__  << __LINE__;  std::abort(); }
#define ESPP_ASSERT ESPP_CHECK

#undef assert
#define assert ESPP_ASSERT
