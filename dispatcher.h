#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "common.h"

void dispatcherRun(pid_t terminalPid, pid_t securityPid, pid_t departuresPid);

enum Event {
	EVACUATE,
	FORCED_DEPART,
	NO_CHECKIN
};

#endif //DISPATCHER_H
