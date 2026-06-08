#include "WatchCommands.h"
#include "../Debugger.h"

void WatchCommand::execute(Debugger& debugger)   { debugger.addWatch(var_); }
void UnwatchCommand::execute(Debugger& debugger) { debugger.removeWatch(var_); }
void WatchesCommand::execute(Debugger& debugger) { debugger.printWatches(); }
void InspectCommand::execute(Debugger& debugger) { debugger.printInspect(); }
