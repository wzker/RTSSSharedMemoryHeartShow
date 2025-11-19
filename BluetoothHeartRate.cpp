#include "stdafx.h"
#include "BluetoothHeartRate.h"
#include <windows.h>
#include <winhttp.h>
#include <thread>
#include <atomic>
#include <iostream>
#include <cstring>

#pragma comment(lib, "winhttp.lib")

BluetoothHeartRate::BluetoothHeartRate() : m_latestHeartRate(-1), m_running(false) {}
BluetoothHeartRate::~BluetoothHeartRate() { Stop(); }

bool BluetoothHeartRate::Start()
{
    if (m_running) return false;
    m_running = true;
    m_worker = std::thread(&BluetoothHeartRate::PollHeartRate, this);
    return true;
}

void BluetoothHeartRate::Stop()
{
    m_running = false;
    if (m_worker.joinable())
        m_worker.join();
}

int BluetoothHeartRate::GetLatestHeartRate() const
{
    return m_latestHeartRate.load();
}
int BluetoothHeartRate::GetLatestHeartRateo() const
{
    return o_latestHeartRate.load();
}

// 初始化函数，或放在构造函数中
bool BluetoothHeartRate::Initialize()
{
    m_running = true;
    m_latestHeartRate = -1;

    // 默认值，如果读取失败就使用 127.0.0.1
    m_host = L"127.0.0.1";

    // 尝试从 host.txt 读取主机名
    std::ifstream file("host.txt");
    if (file.is_open())
    {
        std::string hostStr;
        if (std::getline(file, hostStr))
        {
            // 将 std::string (ANSI 或 UTF-8) 转为 std::wstring
            // 简单处理：假设 host.txt 是 ANSI 编码（如 ASCII 或 UTF-8 无 BOM）
            // 若有中文或特殊字符，建议确保 host.txt 是 UTF-8 无 BOM，或使用更健壮的转换方式
            int size_needed = MultiByteToWideChar(CP_UTF8, 0, hostStr.c_str(), (int)hostStr.size(), NULL, 0);
            if (size_needed > 0)
            {
                std::wstring wstrTo(size_needed, 0);
                MultiByteToWideChar(CP_UTF8, 0, hostStr.c_str(), (int)hostStr.size(), &wstrTo[0], size_needed);
                m_host = wstrTo;
            }
        }
        file.close();
    }
    else
    {
        // 文件不存在，使用默认值 127.0.0.1
        // 可以输出日志提示，例如 OutputDebugString
    }

    return true;
}

// 通过 HTTP 轮询获取心率数据
void BluetoothHeartRate::PollHeartRate()
{
    while (m_running)
    {
        int heartRate = -1;
        int heartRateo = -1;
        HINTERNET hSession = WinHttpOpen(L"HeartRateClient/1.0", WINHTTP_ACCESS_TYPE_NO_PROXY, NULL, NULL, 0);
        if (hSession)
        {
            HINTERNET hConnect = WinHttpConnect(hSession, L"127.0.0.1", 3030, 0);
            if (hConnect)
            {
                HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", L"/heartrate", NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
                if (hRequest)
                {
                    if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
                        WinHttpReceiveResponse(hRequest, NULL))
                    {
                        DWORD dwSize = 0;
                        if (WinHttpQueryDataAvailable(hRequest, &dwSize) && dwSize > 0)
                        {
                            char* buffer = new char[dwSize + 1];
                            DWORD dwDownloaded = 0;
                            if (WinHttpReadData(hRequest, buffer, dwSize, &dwDownloaded))
                            {
                                buffer[dwDownloaded] = 0;
                                try {
                                    heartRate = std::stoi(buffer);
                                } catch (...) {
                                    heartRate = -1;
                                }
                            }
                            delete[] buffer;
                        }
                    }
                    WinHttpCloseHandle(hRequest);
                }
                WinHttpCloseHandle(hConnect);
            }
            WinHttpCloseHandle(hSession);
        }

        HINTERNET hSessiono = WinHttpOpen(L"HeartRateClient/1.0", WINHTTP_ACCESS_TYPE_NO_PROXY, NULL, NULL, 0);
        if (hSessiono)
        {
            HINTERNET hConnect = WinHttpConnect(hSessiono, m_host.c_str(), 3030, 0);
            if (hConnect)
            {
                HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", L"/heartrate", NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
                if (hRequest)
                {
                    if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) &&
                        WinHttpReceiveResponse(hRequest, NULL))
                    {
                        DWORD dwSize = 0;
                        if (WinHttpQueryDataAvailable(hRequest, &dwSize) && dwSize > 0)
                        {
                            char* buffer = new char[dwSize + 1];
                            DWORD dwDownloaded = 0;
                            if (WinHttpReadData(hRequest, buffer, dwSize, &dwDownloaded))
                            {
                                buffer[dwDownloaded] = 0;
                                try {
                                    heartRateo = std::stoi(buffer);
                                } catch (...) {
                                    heartRateo = -1;
                                }
                            }
                            delete[] buffer;
                        }
                    }
                    WinHttpCloseHandle(hRequest);
                }
                WinHttpCloseHandle(hConnect);
            }
            WinHttpCloseHandle(hSession);
        }
        
        if (heartRate > 0)
        {
            m_latestHeartRate = heartRate;
        }
        
        if (heartRateo > 0)
        {
            o_latestHeartRate = heartRateo;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 刷新间隔
    }
}
