#pragma once

#include "DataSource.hpp"

namespace HeartBeat {

class RandomDataSource : public DataSource {
public:
    RandomDataSource()
        : DataSource(DataSourceType::DS_RANDOM) {}

    bool GetData(int& heartbeat) override {
        static int x = 0;
        if (x++ % 13 == 0) {
            heartbeat = x % 200;
            return true;
        }
        return false;
    }
};


} // namespace HeartBeat