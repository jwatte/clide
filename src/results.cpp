#include "results.h"
#include <assert.h>
#include <iostream>

static std::vector<std::pair<Status, std::string> > results;
static bool verbosity[4] = { false, false, false, false };
static char const *tags[4] = {
    "SUCCESS: ",
    "INFO: ",
    "WARNING: ",
    "ERROR: ",
};

void result(std::string const &r, Status s)
{
    if (verbosity[s]) {
        std::cerr << tags[s] << r << std::endl;
    }
    results.push_back(std::pair<Status, std::string>(s, r));
}

void result_clear()
{
    results.clear();
}

void result_harvest(std::vector<std::pair<Status, std::string> > &oresults)
{
    std::vector<std::pair<Status, std::string> > tmp;
    tmp.swap(results);
    oresults.swap(tmp);
}

void result_verbose(Status level, bool print)
{
    assert(!(level < 0 || level > StatusError));
    if (level < 0 || level > StatusError) {
        return;
    }
    verbosity[level] = print;
}


