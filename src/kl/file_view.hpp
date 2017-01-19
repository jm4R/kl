#pragma once

#include "kl/byte.hpp"

#include <gsl/span>
#include <gsl/string_span>

#include <memory>

namespace kl {

class file_view
{
public:
    explicit file_view(gsl::cstring_span<> file_path);
    ~file_view();

    gsl::span<const byte> get_bytes() const;

private:
    struct impl;
    std::unique_ptr<impl> impl_;
};

} // namespace kl
