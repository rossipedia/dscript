#include "dscript.h"
#include "stdlib.h"

using namespace std;
using namespace dscript;

int main(int argc,char* argv[])
{
    context ctx;
    ctx.enable_logging(&cout);
    link_stdlib(ctx);

    ctx.dump_file(cout, "test.txt");
    /*ctx.compile("test.txt");
    ctx.exec_compiled("test.txt");*/

    return 0;
}