#include "ProgramParameters.hpp"

std::unique_ptr<ProgramParameters> par;

ProgramParameters* getParams()
{
  return par.get();
}

void initialize()
{

}
