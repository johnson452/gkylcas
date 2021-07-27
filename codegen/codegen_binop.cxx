#include <modal_basis.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <gkyl_util.h>

using namespace GiNaC;

static struct gkyl_kern_op_count
total_op(const ex& expr)
{
  struct gkyl_kern_op_count count = { 0 };

  count.num_sum = expr.nops()-1;
  for (int i=0; i<expr.nops(); ++i)
    count.num_prod += expr.op(i).nops()-1;

  return count;
}

static void
gen_ser_mul_op(std::ostream& fh, std::ostream& fc, const Gkyl::ModalBasis& basis)
{
  int ndim = basis.get_ndim(), polyOrder = basis.get_polyOrder();
  lst bc = basis.get_basis();
  
  // function declaration
  fh << std::endl
     << "GKYL_CU_DH void binop_mul_" << ndim << "d_ser_" << "p" << polyOrder
     << "(const double *f, const double *g, double *fg );" << std::endl;

  // declare function returning op counts
  fh << "struct gkyl_kern_op_count op_count_binop_mul_" << ndim << "d_ser_" << "p" << polyOrder
     << "(void);" << std::endl;

  // function definition
  fc << "GKYL_CU_DH" << std::endl;
  fc << "void" << std::endl;
  fc << "binop_mul_" << ndim << "d_ser_" << "p" << polyOrder
       << "(const double *f, const double *g, double *fg )" << std::endl;
  fc << "{" << std::endl;  

  int nsum = 0, nprod = 0;
  symbol f("f"), g("g");
  auto fg = basis.expand(f)*basis.expand(g);

  for (int i=0; i<basis.get_numbasis(); ++i) {
    std::cout << i << " " << std::flush;
    auto out = basis.innerProd(bc[i], fg).expand().evalf();
    fc << "  fg[" << i << "] = " << csrc << out << ";" << std::endl;
    struct gkyl_kern_op_count count = total_op(out);
    nsum += count.num_sum;
    nprod += count.num_prod;
  }
  std::cout << std::endl;

  fc << "  // nsum = " << nsum << ", nprod = " << nprod << std::endl;
  // close function
  fc << "}" << std::endl << std::endl;

  // write out function to return op counts
  fc << "struct gkyl_kern_op_count op_count_binop_mul_" << ndim << "d_ser_" << "p" << polyOrder
     << "(void)" << std::endl;
  fc << "{" << std::endl;
  fc << "  return (struct gkyl_kern_op_count) { .num_sum = " << nsum << ", .num_prod = " << nprod
     << " };" << std::endl;
  fc << "}" << std::endl;
}

int
main(int argc, char **argv)
{
  // compute time-stamp
  char buff[70];
  time_t t = time(NULL);
  struct tm curr_tm = *localtime(&t);
  strftime(buff, sizeof buff, "%c", &curr_tm);
  
  int dims[] = { 1, 2, 3 };
  int max_order[] = { 3, 3, 3 };

  symbol z0("z0"), z1("z1"), z2("z2"), z3("z3"), z4("z4"), z5("z5");
  std::vector<symbol> vars { z0, z1, z2, z3, z4, z5 };

  std::ofstream mul_file_h("kernels/bin_op/gkyl_binop_mul_ser.h", std::ofstream::out);
  mul_file_h << "// " << buff << std::endl;
  mul_file_h << "#pragma once" << std::endl;
  mul_file_h << "#include <gkyl_util.h>" << std::endl;
  mul_file_h << "EXTERN_C_BEG" << std::endl;
  
  struct timespec tstart = gkyl_wall_clock();

  for (int d=0; d<3; ++d) {
    int dim = dims[d];
    for (int p=0; p<=max_order[d]; ++p) {
      std::cout << dim << "dp" << p << " " << std::flush;
      Gkyl::ModalBasis mbasis(dim, vars, p);

      // each function is written to its own file to allow building
      // kernels in parallel
      std::ostringstream fn;
      fn << "kernels/bin_op/binop_mul_" << dim << "d_ser_" << "p" << p << ".c";
      std::ofstream mul_file_c(fn.str().c_str(), std::ofstream::out);
      mul_file_c << "// " << buff << std::endl;
      mul_file_c << "#include <gkyl_binop_mul_ser.h>" << std::endl;
      
      // generate multiply method
      gen_ser_mul_op(mul_file_h, mul_file_c, mbasis);
    }
    std::cout << std::endl;
  }

  mul_file_h << "EXTERN_C_END" << std::endl;

  double tm = gkyl_time_diff_now_sec(tstart);
  std::cout << "Took " << tm << " seconds" << std::endl;  
  
  return 1;
}
