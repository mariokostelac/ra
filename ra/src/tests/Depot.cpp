#include "gtest/gtest.h"
#include "../Depot.hpp"

TEST(Depot, Creation) {
  auto depot = new Depot("depot_dummy");
  delete depot;
}
