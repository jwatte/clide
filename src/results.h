#if !defined(clide_results_h)
#define clide_results_h

#include <string>
#include <vector>

class BracketResult {
    public:
        BracketResult(std::string const &name);
        ~BracketResult();
    private:
        BracketResult(BracketResult const &) = delete;
        BracketResult &operator=(BracketResult const &) = delete;
        void *bracket;
};

enum Status {
    StatusSuccess,
    StatusInfo,
    StatusWarning,
    StatusError
};

void result(std::string const &r, Status s = StatusInfo);
void result_clear();
void result_harvest(std::vector<std::pair<Status, std::string> > &oresults);
void result_verbose(Status level, bool print);

#endif  //  clide_results_h
