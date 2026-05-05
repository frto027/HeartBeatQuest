
#include "data_sources/DataSource.hpp"
namespace HeartBeat {
class UIManager {
private:
    int readerCount = 0;

public:
    void addReader() {
        if (readerCount == 0)
            DataSource::getInstance()->OnNewReader();
        readerCount++;
    }
    void decReader() { readerCount--; }
    bool hasReader() { return !!readerCount; }

    static UIManager* getInstance();
};
} // namespace HeartBeat