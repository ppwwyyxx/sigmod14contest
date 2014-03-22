/*
 * $File: ThreadPool.hh
 * $Date: Sat Mar 22 17:00:57 2014 +0800
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
			Function &&f;
			Callback &&callback;

			runner(Function &&f, Callback &&callback) :
				f(std::forward<Function>(f)), callback(std::forward<Callback>(callback))
			{ }

			virtual void run() {
				callback(f());
			}
	};

	inline void runner_wrapper(std::shared_ptr<runner_base> rn) {
		rn->run();
	}

	void worker(ThreadPool *tp);
	inline void task_wrapper(std::shared_ptr<std::function<void()>> func) {
		(*func)();
	}
}

class ThreadPool {
private:

public:
	ThreadPool(size_t);
	template<class Function, class Callback>
		void enqueue(Function &&f, Callback &&callback) {
			/*
			 *if (stop)
			 *    throw std::runtime_error("enqueue on stopped ThreadPool");
			 */

			auto task = std::make_shared<std::function<void()>>(
					std::bind(__ThreadPoolImpl::runner_wrapper,
						std::make_shared<__ThreadPoolImpl::runner<Function, Callback>>(
							std::forward<Function>(f), std::forward<Callback>(callback))));
			//auto task = std::make_shared<std::function<void()>>(std::bind(std::forward<Callback>(callback), (std::forward<Function>(f))()));
			{
				std::unique_lock<std::mutex> lock(queue_mutex);
				tasks.push(std::bind(__ThreadPoolImpl::task_wrapper, task));

			}
			condition.notify_one();
		}

	template<class Function>
		void enqueue(Function &&f) {
			/*
			 *if (stop)
			 *    throw std::runtime_error("enqueue on stopped ThreadPool");
			 */

			auto task = std::make_shared<std::function<void()>>(f);
			{
				std::unique_lock<std::mutex> lock(queue_mutex);
				tasks.push(std::bind(__ThreadPoolImpl::task_wrapper, task));
			}
			condition.notify_one();
		}

	~ThreadPool();
	// the task queue
	std::queue<std::function<void()> > tasks;
private:
	friend void __ThreadPoolImpl::worker(ThreadPool *tp);

	// need to keep track of threads so we can join them
	std::vector< std::thread > workers;

	// synchronization
	std::mutex queue_mutex;
	std::condition_variable condition;
	std::mutex nr_active_thread_mutex;

	int nr_active_thread;
	bool stop;
};

// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t threads)
	: nr_active_thread(0), stop(false)
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
	//condition.notify_all();
	for(size_t i = 0;i<workers.size();++i)
		workers[i].join();
}

namespace __ThreadPoolImpl
{ void worker(ThreadPool *tp); }
/*
 * vim: syntax=cpp11.doxygen foldmethod=marker
 */

/*
 *#include <cstdio>
 *int fint(int k) {
 *    std::this_thread::sleep_for(std::chrono::seconds(2));
 *    return k + 1;
 *}
 *
 *void getint(int a) {
 *    printf("%d\n", a);
 *}
 *
 *void fvoid(int k) {
 *    std::this_thread::sleep_for(std::chrono::seconds(2));
 *    printf("haha%d\n", k + 100);
 *}
 *
 *int main() {
 *    ThreadPool p(4);
 *    int a = 1;
 *    p.enqueue(std::bind(fint, std::ref( a )), getint);
 *
 *    p.enqueue(std::bind(fvoid, std::ref(a)));
 *    a = 2;
 *}
 */
