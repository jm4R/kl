#include "kl/yaml.hpp"

namespace kl::yaml {

void deserialize_error::add(const char* message)
{
    messages_.insert(end(messages_), '\n');
    messages_.append(message);
}

deserialize_error::~deserialize_error() noexcept = default;
parse_error::~parse_error() noexcept = default;

void expect_scalar(const YAML::Node& value)
{
    if (value.IsScalar())
        return;

    throw deserialize_error{"type must be a scalar but is a " +
                            yaml::type_name(value)};
}

void expect_sequence(const YAML::Node& value)
{
    if (value.IsSequence())
        return;

    throw deserialize_error{"type must be a sequence but is a " +
                            yaml::type_name(value)};
}

void expect_map(const YAML::Node& value)
{
    if (value.IsMap())
        return;

    throw deserialize_error{"type must be a map but is a " +
                            yaml::type_name(value)};
}
} // namespace kl::yaml
