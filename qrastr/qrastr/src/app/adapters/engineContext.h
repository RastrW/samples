#pragma once
#include <memory>

class IFileOperations;
class ICalculationEngine;
class ILogSource;
class ITableRepository;
class ITIEngine;
class IBarsMDPEngine;

struct EngineContext {
    std::shared_ptr<IFileOperations>    fileOps;
    std::shared_ptr<ICalculationEngine> calcEngine;
    std::shared_ptr<ILogSource>         logSource;
    std::shared_ptr<ITableRepository>   tables;
    std::shared_ptr<ITIEngine>          ti;       // nullptr если не загружен
    std::shared_ptr<IBarsMDPEngine>     barsMDP;  // nullptr если не загружен

    bool hasTI()     const noexcept { return ti     != nullptr; }
    bool hasBarsMDP()const noexcept { return barsMDP != nullptr; }
};