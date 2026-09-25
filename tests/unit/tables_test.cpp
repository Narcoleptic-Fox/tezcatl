#include "report/tables.hpp"
#include "report_sample.hpp"

#include <catch2/catch_test_macros.hpp>

#include <sstream>
#include <string>

using namespace tezcatl;

TEST_CASE("the file table names each file's module and role", "[report]") {
    std::ostringstream out;
    report::write_file_table(out, test::sample_report().files, test::sample_naming());
    CHECK(out.str() == "file,module,role,physical,blank,comment,code\n"
                       "a/x.c,a,production,10,1,2,7\n"
                       "a/tests/t.c,a,test,5,0,1,4\n"
                       "b/y.c,(unassigned),production,4,0,0,4\n");
}
