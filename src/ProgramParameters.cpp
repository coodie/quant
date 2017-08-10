#include "ProgramParameters.hpp"

namespace {
std::unique_ptr<ProgramParameters> par = std::make_unique<ProgramParameters>();
}

ProgramParameters *getParams() { return par.get(); }
