#pragma once

#include <chrono>
#include <iostream>
#include <string>
#include <iomanip>
#include <map>
#include <algorithm>
#include <vector>

struct CountTime {
    uint64_t cnt = 0;
    uint64_t cnt_greater_zero_time = 0;
    uint64_t time = 0;
};

class LogsSum {
public:
    ~LogsSum() {
        std::vector<std::pair<std::string, CountTime>> res(logs.begin(), logs.end());

        std::sort(res.begin(), res.end(),
            [](std::pair<std::string, CountTime> lhs, std::pair<std::string, CountTime> rhs) {
                return lhs.second.time < rhs.second.time;
            });

        std::ostringstream os;
        os << std::setw(60) << "name" << std::setw(20) << "time"
            << std::setw(20) << "cnt" << std::setw(20)
            << "cnt_gr_zero_time" << std::endl;

        for (const auto& item : res) {
            os << std::setw(60) << item.first << std::setw(20) << item.second.time
                << std::setw(20) << item.second.cnt << std::setw(20)
                << item.second.cnt_greater_zero_time << std::endl;
        }
        Sys_Printf("%s", os.str().c_str());
    }

    std::map<std::string, CountTime> logs;
};

extern LogsSum logs;

template <typename duration>
class LogDuration {
public:
    explicit LogDuration(const std::string& msg = "")
        : message(msg + ": ")
        , start(std::chrono::steady_clock::now())
        , logs_(nullptr)
    {
    }

    explicit LogDuration(const std::string& msg, LogsSum* logs)
        : message(msg + ": ")
        , start(std::chrono::steady_clock::now())
        , logs_(logs)
    {
    }

    ~LogDuration() {
        auto finish = std::chrono::steady_clock::now();
        auto dur = finish - start;
        auto d = std::chrono::duration_cast<duration>(dur).count();

        if (logs_ == nullptr) {
            std::cerr << std::setw(60) << message << std::setw(20) << d << std::endl;
        }
        else {
            logs_->logs[message].cnt++;
            if (d > 0) {
                logs_->logs[message].time += d;
                logs_->logs[message].cnt_greater_zero_time++;
            }
        }
    }

private:
    std::string message;
    std::chrono::steady_clock::time_point start;
    LogsSum* logs_ = nullptr;
};

#define UNIQ_ID_IMPL(lineno) _a_local_var_##lineno
#define UNIQ_ID(lineno) UNIQ_ID_IMPL(lineno)

#define LOG_DURATION(message) \
  LogDuration<chrono::microseconds> UNIQ_ID(__LINE__){std::string(message) + " microseconds"};

#define LOG_SUM_DURATION(message, duration) \
  LogDuration<std::chrono::duration> UNIQ_ID(__LINE__){std::string(message) + ' ' + #duration, &logs};

#define DEF_LOGS LogsSum logs;
#define DEF_EXTERN_LOGS extern LogsSum logs;

struct TotalDuration {
    std::string message;
    std::chrono::steady_clock::duration value;
    explicit TotalDuration(const std::string& msg = "")
        : message(msg + ": "), value(0) {}
    ~TotalDuration() {
        std::ostringstream os;
        os << message << std::chrono::duration_cast<std::chrono::milliseconds>(value).count() << " ms"
            << std::endl;
        //std::cerr << os.str();
        Sys_Printf("%s", os.str().c_str());
    }
};
class AddDuration {
public:
    explicit AddDuration(std::chrono::steady_clock::duration& dest)
        : add_to(dest), start(std::chrono::steady_clock::now()) {}
    explicit AddDuration(TotalDuration& dest) : AddDuration(dest.value) {}
    ~AddDuration() { add_to += std::chrono::steady_clock::now() - start; }

private:
    std::chrono::steady_clock::duration& add_to;
    std::chrono::steady_clock::time_point start;
};
#define ADD_DURATION(value) AddDuration UNIQ_ID(__LINE__){value};

#define UNUSED(expr)  \
    do {              \
        (void)(expr); \
    } while (0)