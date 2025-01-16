#pragma once
#include <gtest/gtest.h>
#include <sl_log.h>
#include <nlohmann/json.hpp>
#include <sstream>
namespace unify::matter_bridge::test {


/** Assert two Strings containing JSON code are equal (ignoring whitespaces and order of JSON entries)*/
#define EXPECT_EQ_JSON(_expected, _actual) {\
    try {\
        auto _expected_json = nlohmann::json::parse(_expected);\
        auto _actual_json = nlohmann::json::parse(_actual);\
        if (_expected_json != _actual_json) {\
            sl_log_error("ASSERTION FAILED", "\nExpected: %s\nActual: %s\nDiff: %s", _expected_json.dump().c_str(), _actual_json.dump().c_str(), nlohmann::json::diff(_expected_json, _actual_json).dump().c_str());\
            EXPECT_EQ(_expected, _actual);\
        }\
    } catch (const nlohmann::detail::exception &_ex) {\
        sl_log_error("JSON PARSE FAILED", "%s", _ex.what());\
        EXPECT_FALSE(false);\
    }\
}

/** Assert two of any type are equal. Should work for all non-custom types*/
#define EXPECT_EQ_GEN(_expected, _actual) {\
    if (_expected != _actual) {\
        std::ostringstream _dbg_out;\
        _dbg_out << "Expected: " << _expected << std::endl << "Actual: " << _actual;\
        sl_log_error("ASSERTION FAILED", "\n%s", _dbg_out.str().c_str());\
        EXPECT_EQ(_expected, _actual);\
    }\
}
};
