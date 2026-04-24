#pragma once
#include <memory>

class IFileOperations;
class ICalculationEngine;
class ILogEvents;
class ITableRepository;
class ITIEngine;
class IBarsMDPEngine;
class ITableEvents;

struct EngineContext {
    std::shared_ptr<IFileOperations>    fileOps;
    std::shared_ptr<ICalculationEngine> calcEngine;
    std::shared_ptr<ILogEvents>         logEvents;
    std::shared_ptr<ITableRepository>   tables;
    std::shared_ptr<ITableEvents>       tableEvents;

    std::shared_ptr<ITIEngine>          ti;       // nullptr если не загружен
    std::shared_ptr<IBarsMDPEngine>     barsMDP;  // nullptr если не загружен

    bool hasTI()     const noexcept { return ti     != nullptr; }
    bool hasBarsMDP()const noexcept { return barsMDP != nullptr; }
};