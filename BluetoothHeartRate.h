#pragma once
#include <atomic>
#include <thread>
#include <string>

class BluetoothHeartRate
{
public:
    BluetoothHeartRate();
    ~BluetoothHeartRate();
    bool Start();
    void Stop();
    bool Initialize();
    int GetLatestHeartRate() const;
    int GetLatestHeartRateo() const;

private:
    void PollHeartRate();
    std::wstring m_host;       // 保存从 host.txt 读取的主机名，如 L"127.0.0.1"
    std::atomic<int> m_latestHeartRate;
    std::atomic<int> o_latestHeartRate;
    std::thread m_worker;
    std::thread o_worker;
    bool m_running;
};
