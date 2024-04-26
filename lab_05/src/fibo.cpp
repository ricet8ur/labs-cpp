#include "2_ppp_class.hpp"
int main(){
start_ppp;
// ppp_show_info;
auto a = var(1.);
auto b = var(1.);
start_pfor(k, 0, 10, 1);
    auto q = var(a);
    // a=b;
    // b=q+a;
    ppp_assign(b, a);
    ppp_assign(q + a, b);
end_pfor;
pcout << a;
pfout("fibonacci.out") << a;
run_ppp;return 0;
}