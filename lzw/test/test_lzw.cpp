#define CATCH_CONFIG_MAIN // TODO remove

#include <catch2/catch.hpp>
#include "lzw.hpp"

TEST_CASE("Dummy test case") {
    REQUIRE(lzw::dummy_function() == 1);
}
