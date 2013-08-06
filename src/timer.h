#ifndef _TIMER_H_
#define _TIMER_H_

#include <string>
#include "boost/chrono.hpp"
#include "boost/lexical_cast.hpp"

class timer {
public:
	void reset() {
		start_();
	}
	
	void start() {
		start_();	
	}

	// 毫秒
	double ms() {
		boost::chrono::duration<double> sec = 
			boost::chrono::system_clock::now() - s_;
		return 1000 * sec.count();
	}
private:
	void start_() {
		s_ = boost::chrono::system_clock::now();
	}
	
	boost::chrono::system_clock::time_point s_;
};

#endif
