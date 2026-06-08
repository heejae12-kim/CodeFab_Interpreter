#include "FlowCommands.h"
#include "../Debugger.h"

void StepCommand::execute(Debugger& debugger)     { debugger.step(); }
void NextCommand::execute(Debugger& debugger)     { debugger.next(); }
void ContinueCommand::execute(Debugger& debugger) { debugger.continueRun(); }
