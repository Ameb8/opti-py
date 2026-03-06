#ifndef FLOW_SHOP_RESULT_H
#define FLOW_SHOP_RESULT_H

#include <vector>

class FlowShopResult {
public:
    std::vector<size_t> sequence; // Order of job execution
    uint64_t makespan; // Total execution time
    std::vector<std::vector<uint64_t>> completionTimes; // Time each job finishes on a machine
    std::vector<uint64_t> tardiness; // Tardiness values for each job
};


#endif