#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

int main(int argc, char** argv) {
    doctest::Context context;

    // defaults
    //context.addFilter("test-case-exclude", "*tracking*"); 
    //context.setOption("order-by", "name");            // sort the test cases by their name
    //context.addFilter("test-case", "*Ring Buffer*"); 

    context.applyCommandLine(argc, argv);

    // overrides
    context.setOption("duration", true);            // Show duration for each test
    context.setOption("success", false);             // Show successful test
    context.setOption("no-path-filenames", true);   // Don't show path
    context.setOption("no-breaks", true);           // don't break in the debugger when assertions fail

    int res = context.run(); // run

    if(context.shouldExit()) // important - query flags (and --exit) rely on the user doing this
        return res;          // propagate the result of the tests
    return res;
}
