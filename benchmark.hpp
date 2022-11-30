#pragma once

#include <iostream>
#include <string_view>
#include <functional>
#include <chrono>
#include <initializer_list>
#include <optional>

#ifndef BENCHMARK_REPEAT_NUM
#define BENCHMARK_REPEAT_NUM 1000
#endif

namespace benchmark {

class Measure;

using BenchmarkFunc = std::function<void(void)>;
using BenchmarkFuncWithOp = std::function<void(Measure)>;

struct Unit final {
    Unit(std::string_view name, BenchmarkFunc func): name(name), func(func), time(0) {}
    Unit(std::string_view name, BenchmarkFuncWithOp func): name(name), funcWithOp(func), time(0) {}

    std::string_view name;
    BenchmarkFunc func;
    BenchmarkFuncWithOp funcWithOp;
    long long time;
};

class Measure final {
public:
    Measure(Unit& unit): unit_(unit) {}

    void operator()(BenchmarkFunc func) const {
        auto begin = std::chrono::steady_clock::now();
        if (func) {
            for (int i = 0; i < BENCHMARK_REPEAT_NUM; i++) {
                (void)func();
            }
        }
        unit_.time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin).count();
    }

private:
    Unit& unit_;
};

class Group final {
public:
    friend class BenchmarkMgr;

    Group(std::string_view name): name_(name) {}

    void Add(const Unit& unit) {
        units_.push_back(unit);
    }

    void DoBenchmark() {
        std::cout << "running group " << name_ << std::endl;
        for (auto& unit : units_) {
            std::cout << "measuring " << unit.name << " ...";
            if (unit.func) {
                measureOneUnit(unit);
            } else {
                measureOneUnitWithExtraOp(unit);
            }
            std::cout << std::endl;
        }
    }

    void ShowResult() {
        for (auto& unit : units_) {
            std::cout << unit.name << ":" << std::endl;
            std::cout << "\ttotle time: " << unit.time << "ms" << std::endl;
            std::cout << "\taverage time: " << unit.time / static_cast<float>(BENCHMARK_REPEAT_NUM) << "ms" << std::endl;
            std::cout << std::endl;
        }
    }

private:
    std::vector<Unit> units_;
    std::string_view name_;

    void measureOneUnit(Unit& unit) {
        if (unit.func) {
            Measure measure(unit);
            measure(unit.func);
        }
    }

    void measureOneUnitWithExtraOp(Unit& unit) {
        if (unit.funcWithOp) {
            unit.funcWithOp(Measure{unit});
        }
    }
};

class BenchmarkMgr final {
public:
    static BenchmarkMgr& Instance() {
        static std::unique_ptr<BenchmarkMgr> instance;

        if (!instance) {
            instance = std::make_unique<BenchmarkMgr>();
        }

        return *instance;
    }

    template <typename... Names>
    void Run(Names... names) {
        doRun(names...);
    }

    void BeginGroup( std::string_view name) {
        group_ = Group(name);
    }

    void PushCurrentGroup() {
        if (group_) {
            groups_.emplace(group_->name_, group_.value());
        }
    }

    void AddUnit2CurrentGroup(const Unit& unit) {
        if (group_) {
            group_->Add(unit);
        }
    }

private:
    std::unordered_map<std::string_view, Group> groups_;
    std::optional<Group> group_;

    template <typename Name, typename... Names>
    void doRun(Name name, Names... names) {
        auto it = groups_.find(name);
        if (it != groups_.end()) {
            auto& group = it->second;
            group.DoBenchmark();
            std::cout << std::endl;
            group.ShowResult();
            std::cout << "--------------------------------" << std::endl;
        }
        doRun(names...);
    }

    void doRun(std::string_view name) {
        auto it = groups_.find(name);
        if (it != groups_.end()) {
            auto& group = it->second;
            group.DoBenchmark();
            std::cout << std::endl;
            group.ShowResult();
        }
    }

    void doRun(const char* name) {
        auto it = groups_.find(name);
        if (it != groups_.end()) {
            auto& group = it->second;
            group.DoBenchmark();
            std::cout << std::endl;
            group.ShowResult();
        }
    }
    
};

}


#define BENCHMARK_MAIN int main(int argc, char** argv)
#define BENCHMARK_GROUP(name)  benchmark::BenchmarkMgr::Instance().PushCurrentGroup(); benchmark::BenchmarkMgr::Instance().BeginGroup(name);
#define BENCHMARK_ADD(name, func) benchmark::BenchmarkMgr::Instance().AddUnit2CurrentGroup(benchmark::Unit(name, func));
#define BENCHMARK_RUN(...) benchmark::BenchmarkMgr::Instance().PushCurrentGroup(); benchmark::BenchmarkMgr::Instance().Run(__VA_ARGS__);