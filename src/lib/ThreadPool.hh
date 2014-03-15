/*
 * $File: ThreadPool.hh
 * $Date: Sat Mar 15 12:49:01 2014 +0800
 * $Author: Xinyu Zhou <zxytim[at]gmail[dot]com>
 */

/**
 * A ``thread pool'' implementation for g++ 4.4 using callback
 */

#pragma once

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <stdexcept>

class ThreadPool;
namespace __ThreadPoolImpl {
	class runner_base {
		public:
			virtual void run() = 0;
	};

	template<class Function, class Callback>
	class runner : public runner_base {
		public:
			Function f;
			Callback &callback;

			runner(Function f, Callback &callback) :
				f(f), callback(callback)
			{
			}
			virtual void run() {
				callback(f());
			}
	};

	void runner_wrapper(std::shared_ptr<runner_base> rn) {
		rn->run();
	}

	void worker(ThreadPool *tp);
	void task_wrapper(std::shared_ptr<std::function<void()>> func) {
		(*func)();
	}
}

class ThreadPool {
private:

public:
	ThreadPool(size_t);
	template<class Function, class Callback>
		void enqueue(Function &&f, Callback &&callback) {
			if (stop)
				throw std::runtime_error("enqueue on stopped ThreadPool");

			auto task = std::make_shared<std::function<void()>>(
					std::bind(__ThreadPoolImpl::runner_wrapper,
						std::make_shared<__ThreadPoolImpl::runner<Function, Callback>>(f, callback)));
			{
				std::unique_lock<std::mutex> lock(queue_mutex);
				tasks.push(std::bind(__ThreadPoolImpl::task_wrapper, task));
			}
			condition.notify_one();
		}
	~ThreadPool();
private:
	friend void __ThreadPoolImpl::worker(ThreadPool *tp);

	// need to keep track of threads so we can join them
	std::vector< std::thread > workers;
	// the task queue
	std::queue< std::function<void()> > tasks;

	// synchronization
	std::mutex queue_mutex;
	std::condition_variable condition;
	bool stop;
};

// the constructor just launches some amount of workers
	inline ThreadPool::ThreadPool(size_t threads)
:   stop(false)
{
	for(size_t i = 0;i<threads;++i)
		workers.emplace_back(std::bind(__ThreadPoolImpl::worker, this));
}



// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		stop = true;
	}
	condition.notify_all();
	for(size_t i = 0;i<workers.size();++i)
		workers[i].join();
}

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
/*
 * vim: syntax=cpp11.doxygen foldmethod=marker
 */

