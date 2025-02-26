#pragma once

#include "format/output/text.h"

namespace format::output {

class OutputFakePDBFile : public OutputTextFile {
public:
    void save(std::string_view path) const override;
};

} // namespace format::output
