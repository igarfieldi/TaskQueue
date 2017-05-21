#ifndef TASKQUEUE_TASKQUEUE_HPP_
#define TASKQUEUE_TASKQUEUE_HPP_

#include <algorithm>
#include <functional>
#include <future>
#include <thread>
#include <vector>
#include <queue>

namespace tskque {

	class TaskQueue {
	private:
		const size_t WORKER_COUNT;
		std::vector<std::thread> m_workerPool;
		std::queue<std::function<void()>> m_taskQueue;

		std::atomic<bool> exit;
		std::atomic<size_t> busyWorkers;
		std::mutex queueMutex;
		std::condition_variable queueEmpty;
		std::condition_variable workersIdle;

	public:
		TaskQueue(size_t workerCount) : WORKER_COUNT(workerCount), m_workerPool(WORKER_COUNT), m_taskQueue(),
										exit(false), busyWorkers(0), queueMutex(), queueEmpty(), workersIdle() {
			// Initialize all threads
			for (auto &t : m_workerPool) {
				t = std::thread(&TaskQueue::workerRun, this);
			}
		}

		TaskQueue() : TaskQueue(std::max<size_t>(1, std::thread::hardware_concurrency())) {
		}

		~TaskQueue() {
			// Set the exit flag to get the workers out of their loops
			exit = true;

			// Notify the condition variables to wake up the workers
			queueEmpty.notify_all();

			// Join all workers to be able to properly exit
			for (auto &t : m_workerPool) {
				if (t.joinable()) {
					t.join();
				}
			}
		}

		// Waits until the task queue is empty
		void join() {
			std::unique_lock<std::mutex> lock(queueMutex);

			workersIdle.wait(lock, [this]() {
				return (busyWorkers == 0) && m_taskQueue.empty();
			});
		}


		// Enqueues a new task into the task queue; this returns a future object
		template < class R, class... Args >
		std::future<R> enqueue(std::function<R(Args...)> func, Args... args) {
			// Create both promise and future
			// Don't forget to delete the promise inside the emplaced function!
			auto promise = new std::promise<R>();
			auto result(promise->get_future());

			// Lock the queue for emplacing the lambda
			std::unique_lock<std::mutex> lock(queueMutex);

			// We emplace a lambda into the queue to keep the execution simpler
			m_taskQueue.emplace([this, promise, args..., func]() {
				std::function<R()> bound = std::bind(func, std::forward<Args>(args)...);
				execute(bound, *promise);

				// DON'T FORGET THIS!
				delete promise;
			});

			// We can manually unlock here
			lock.unlock();

			// Notify a worker that we got a new task
			queueEmpty.notify_one();
			
			return std::move(result);
		}

	private:
		// Actual function execution; also sets the promise value and deletes it afterwards
		template < class R >
		void execute(const std::function<R()> &func, std::promise<R> &promise) {
			promise.set_value(func());
		}

		void workerRun() {
			while (!exit) {
				// First lock the queue
				std::unique_lock<std::mutex> lock(queueMutex);

				if (m_taskQueue.empty()) {
					// Wait until we got stuff in the queue
					queueEmpty.wait(lock, [this]() {
						return !m_taskQueue.empty() || exit;
					});

					if (exit) {
						break ;
					}
				}

				// Get the next task to be executed
				++busyWorkers;
				std::function<void()> curr = m_taskQueue.front();
				m_taskQueue.pop();

				// We can unlock the queue now
				lock.unlock();

				// Execute the task
				curr();
				--busyWorkers;

				// Notify the idle-checker
				workersIdle.notify_one();
			}
		}
	};

	// Specialization of execution for 'void' return type; we cannot use the normal form as above
	template <>
	void TaskQueue::execute<void>(const std::function<void()> &func, std::promise<void> &promise) {
		func();
		promise.set_value();
	}

} // namespace tskque

#endif //TASKQUEUE_TASKQUEUE_HPP_