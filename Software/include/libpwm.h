#ifndef LIBPWM_H
#define LIBPWM_H

class LibPWM {
public:
	LibPWM();
	bool setPeriod(int period);
	bool setDuty(float duty);
	bool run();
	bool stop();

};

#endif
