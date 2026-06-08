#include "BreakCommands.h"
#include "../Debugger.h"

void BreakCommand::execute(Debugger& debugger)  { debugger.addBreakpoint(line_); }
void RemoveCommand::execute(Debugger& debugger) { debugger.removeBreakpoint(line_); }
void BreakpointsCommand::execute(Debugger& debugger) { debugger.printBreakpoints(); }
