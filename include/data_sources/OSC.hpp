#pragma once

#include "DataSource.hpp"
#include <set>
#include <mutex>

namespace HeartBeat {

class HeartBeatOSCDataSource : public DataSource {
private:
    int recv_socket;
    volatile int the_heart;
    volatile bool has_unread_heart_data = false;

    int flush_pipe[2];
    std::string selected_addr;

    void CreateSocket();

public:
    HeartBeatOSCDataSource();
    bool GetData(int& heartbeat) override;

    static void* ServerThread(void* self);
    std::mutex mutex;

    std::set<std::string> received_addresses;
    const std::string& GetSelectedAddress() {
        std::lock_guard<std::mutex> g(mutex);
        return selected_addr;
    }
    void SetSelectedAddr(const std::string& mac);

    std::string mDnsName = "";

    void StartMDns();
    void StopMDns();
    void OnMDnsDevNameChanged(std::string name);

private:
    // in background thread
    void parseOscMessage(char*& thebuff, ssize_t& sz);
};


} // namespace HeartBeat