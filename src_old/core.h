#ifndef CORE_H
#define CORE_H
#include "statemachine.h"
#include "ruleengine.h"
#include "db_access.h"

class Core {
public:
    Core();

    [[noreturn]] void run();

private:
    StateMachine fsm;
    RuleEngine ruleEngine;
    Database db;

    void tick();
};

#endif
