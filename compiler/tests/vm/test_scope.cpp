#include <gtest/gtest.h>
#include "../../vm/vm.hpp"

using namespace VM;

TEST(VM, test_scope_access) {

  GScope myScope(NULL);

  myScope.addObject("foo", getInt32Type());
  EXPECT_EQ(myScope.getObject("foo")->type, getInt32Type());
}
