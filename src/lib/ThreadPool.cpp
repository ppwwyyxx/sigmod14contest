//File: ThreadPool.cpp
//Date: Mon Mar 17 21:28:10 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include "ThreadPool.hh"

namespace __ThreadPoolImpl
{
	void worker(ThreadPool *tp) {
		for (; ;) {
			std::unique_lock<std::mutex> lock(tp->queue_mutex);
			while (!tp->stop && tp->tasks.empty())
				tp->condition.wait(lock);
			if (tp->stop && tp->tasks.empty())
				return;
			std::function<void()> task(tp->tasks.front());
			tp->tasks.pop();
			lock.unlock();
			task();
		}
	}
}
