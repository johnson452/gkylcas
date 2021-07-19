#include <acutest.h>
#include <modal_basis.h>

void
test_ser_1d()
{
  using namespace GiNaC;

  symbol x("x");
  std::vector<symbol> vars { x };
  Gkyl::ModalBasis mbasis(1, vars, 2);

  lst bc = mbasis.get_basis();
  TEST_CHECK( bc.nops() == 3 );
}

TEST_LIST = {
  { "ser_1d", test_ser_1d },
  { NULL, NULL },
};