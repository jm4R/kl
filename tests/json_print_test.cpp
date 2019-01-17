#include "kl/json_print.hpp"
#include "kl/ctti.hpp"

#include <catch2/catch.hpp>
#include <sstream>

namespace {

enum enum_ { eA, eB, eC };
enum class enum_class { A, B, C };
enum enum_reflectable { erA, erB, erC };
enum class enum_class_reflectable { A, B, C };

struct optional_test
{
    boost::optional<int> opt;
    int non_opt;
};

struct inner_t
{
    int r = 1337;
    double d = 3.145926;
};

struct test_struct
{
    std::string hello = "world";
    bool t = true;
    bool f = false;
    int i = 123;
    double pi = 3.1416;
    std::vector<int> a = {1, 2, 3, 4};
};

struct struct_inner
{
    std::vector<std::vector<int>> ad = {std::vector<int>{1, 2},
                                        std::vector<int>{3, 4, 5}};
    std::vector<inner_t> inner_vec = {inner_t{}};
};
}

KL_DEFINE_ENUM_REFLECTOR(enum_reflectable, (erA, erB, erC))
KL_DEFINE_ENUM_REFLECTOR(enum_class_reflectable, (A, B, C))
KL_DEFINE_REFLECTABLE(optional_test, (opt, non_opt))
KL_DEFINE_REFLECTABLE(inner_t, (r, d))
KL_DEFINE_REFLECTABLE(test_struct, (hello, t, f, i, pi, a))
KL_DEFINE_REFLECTABLE(struct_inner, (ad, inner_vec))

TEST_CASE("json_print")
{
    std::ostringstream os;

    SECTION("simple types")
    {
        kl::json_print(os, 34);
        REQUIRE(os.str() == "34");

        os.str("");
        kl::json_print(os, true);
        REQUIRE(os.str() == "true");

        os.str("");
        kl::json_print(os, 3.14f);
        REQUIRE(os.str() == "3.14");

        os.str("");
        kl::json_print(os, eB);
        REQUIRE(os.str() == "1");

        os.str("");
        kl::json_print(os, enum_class::C);
        REQUIRE(os.str() == "2");

        os.str("");
        kl::json_print(os, erC);
        REQUIRE(os.str() == "\"erC\"");

        os.str("");
        kl::json_print(os, enum_class_reflectable::B);
        REQUIRE(os.str() == "\"B\"");

        os.str("");
        kl::json_print(os, "ASD");
        REQUIRE(os.str() == "\"ASD\"");

        os.str("");
        boost::optional<bool> opt;
        kl::json_print(os, opt);
        REQUIRE(os.str() == "null");

        os.str("");
        opt = false;
        kl::json_print(os, opt);
        REQUIRE(os.str() == "false");

        os.str("");
        kl::json_print(os, std::string("QWE"));
        REQUIRE(os.str() == "\"QWE\"");
    }

    SECTION("vector")
    {
        std::vector<int> vec = {6, 5, 4, 3};
        kl::json_print(os, vec);
        REQUIRE(os.str() == "[6,5,4,3]");

        os.str("");
        kl::json_print(os, vec, 4);
        REQUIRE(os.str() ==
R"([
    6,
    5,
    4,
    3
])");

        os.str("");
        kl::json_print(os, vec, 2);
        REQUIRE(os.str() ==
R"([
  6,
  5,
  4,
  3
])");

        os.str("");
        std::vector<std::string> vec_string = {"a", "B", "c"};
        kl::json_print(os, vec_string);
        REQUIRE(os.str() == R"(["a","B","c"])");

        os.str("");
        kl::json_print(os, vec_string, 4);
        REQUIRE(os.str() ==
            R"([
    "a",
    "B",
    "c"
])");
    }

    SECTION("reflectable struct")
    {
        test_struct t;
        kl::json_print(os, t);
        REQUIRE(os.str() == R"({"hello":"world","t":true,"f":false,"i":123,"pi":3.1416,"a":[1,2,3,4]})");

        os.str("");
        kl::json_print(os, t, 2);
        REQUIRE(os.str() ==
R"({
  "hello": "world",
  "t": true,
  "f": false,
  "i": 123,
  "pi": 3.1416,
  "a": [
    1,
    2,
    3,
    4
  ]
})");
    }

    SECTION("struct with optional field")
    {
        optional_test t2{boost::none, 32};
        kl::json_print(os, t2);
#if defined(KL_JSON_DONT_SKIP_NULL_VALUES)
        REQUIRE(os.str() == R"({"opt":null,"non_opt":32})");
#else
        REQUIRE(os.str() == R"({"non_opt":32})");
#endif

        os.str("");
        t2.opt = 13;
        kl::json_print(os, t2);
        REQUIRE(os.str() == R"({"opt":13,"non_opt":32})");
    }

    SECTION("struct with struct")
    {
        struct_inner ss;
        kl::json_print(os, ss);
        REQUIRE(os.str() == R"({"ad":[[1,2],[3,4,5]],"inner_vec":[{"r":1337,"d":3.14593}]})");

        os.str("");
        ss.inner_vec.emplace_back();
        kl::json_print(os, ss, 2);
        REQUIRE(os.str() ==
R"({
  "ad": [
    [
      1,
      2
    ],
    [
      3,
      4,
      5
    ]
  ],
  "inner_vec": [
    {
      "r": 1337,
      "d": 3.14593
    },
    {
      "r": 1337,
      "d": 3.14593
    }
  ]
})");
    }

    SECTION("tuple")
    {
        std::tuple<int, double, std::string> tup = std::make_tuple(1, 3.14f, "QWE");
        kl::json_print(os, tup);
        REQUIRE(os.str() == R"([1,3.14,"QWE"])");

        os.str("");
        kl::json_print(os, tup, 4);
        REQUIRE(os.str() ==
R"([
    1,
    3.14,
    "QWE"
])");
    }

    SECTION("map")
    {
        os.str("");
        std::map<std::string, int> map = {{"1", 1}, {"3", 3}, {"5", 5}, {"7", 7}};
        kl::json_print(os, map);
        REQUIRE(os.str() == R"({"1":1,"3":3,"5":5,"7":7})");

        os.str("");
        kl::json_print(os, map, 2);
        REQUIRE(os.str() ==
R"({
  "1": 1,
  "3": 3,
  "5": 5,
  "7": 7
})");
    }

    SECTION("escape control characters")
    {
        os.str("");
        kl::json_print(os, "te\nst");
        CHECK(os.str() == R"("te\nst")");

        os.str("");
        kl::json_print(os, "test=\"qqqq\"");
        const char* seq = R"("test=\"qqqq\"")";
        CHECK(os.str() == seq);

        os.str("");
        kl::json_print(os, "\f\b\n\r\t\\\"");
        seq = R"("\f\b\n\r\t\\\"")";
        CHECK(os.str() == seq);
    }
}

namespace {

enum class device_type
{
    default_ = (1 << 0),
    cpu = (1 << 1),
    gpu = (1 << 2),
    accelerator = (1 << 3),
    custom = (1 << 4)
};
using device_flags = kl::enum_flags<device_type>;
} // namespace

KL_DEFINE_ENUM_REFLECTOR(device_type, (
    (default_, default),
    cpu,
    gpu,
    accelerator,
    custom
))

TEST_CASE("json_print - enum_flags")
{
    std::ostringstream os;
    kl::json_print(os, kl::make_flags(device_type::cpu) | device_type::gpu, 2);
    REQUIRE(os.str() ==
R"([
  "cpu",
  "gpu"
])");
}