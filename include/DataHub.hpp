#pragma once

namespace HeartBeat {
struct DataHub {
    // this function will be called one or more times per frame by UI objects.
    void Update();

    bool GetData(int& heartrate);

    static DataHub* getInstance();

private:
    int lastUpdateFrame = 0;
    bool hasNewData = false;
    int data;
};
} // namespace HeartBeat