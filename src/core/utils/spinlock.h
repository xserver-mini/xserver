#pragma once

#include <atomic>

class XSpinlock
{
public:
	XSpinlock():flag_(false){}
	inline void lock() 
	{ 
		bool expect = false;
        while (!flag_.compare_exchange_weak(expect, true)) {expect = false;}
	}
	inline void unlock() { flag_.store(false); }
protected:
    std::atomic<bool> flag_;
};

//std::atomic_flag lock_(ATOMIC_FLAG_INIT);
//while (lock_.test_and_set(std::memory_order_acquire));
//lock_.clear(std::memory_order_release);