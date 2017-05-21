#include "taskqueue.hpp"
#include <iostream>

using namespace std::chrono;

class Timer {
private:
	high_resolution_clock::time_point startPoint;

public:
	Timer() : startPoint(high_resolution_clock::now()) {
	}

	void start() {
		startPoint = high_resolution_clock::now();
	}

	template < class T >
	size_t getTime() {
		return duration_cast<T>(high_resolution_clock::now() - startPoint).count();
	}
};

template < class R, class... Args >
size_t testTaskQueue(std::function<R(Args...)> task, size_t count, size_t threads) {
	tskque::TaskQueue queue(threads);
	Timer timer;

	for (size_t i = 0; i < count; ++i) {
		queue.enqueue(task);
	}

	queue.join();

	return timer.getTime<milliseconds>();
}

template < class R, class... Args >
size_t testNormal(std::function<R(Args...)> task, size_t count) {
	Timer timer;

	for (size_t i = 0; i < count; ++i) {
		task();
	}

	return timer.getTime<milliseconds>();
}

int main() {
	const size_t TASK_COUNT = 500;
	const size_t WORKER_COUNT = 4;
	std::function<void()> task = []() {
		volatile double d = 0.0;
		for (size_t i = 1; i < 1000000; ++i) {
			d += std::log10(i);
		}
	};

	size_t normalTime = testNormal(task, TASK_COUNT);
	std::cout << "Normal: " << normalTime << "ms" << std::endl;
	size_t queueTime = testTaskQueue(task, TASK_COUNT, WORKER_COUNT);
	std::cout << "Task queue: " << queueTime << "ms" << std::endl;
	std::cout << "Scaling (" << WORKER_COUNT << " thread(s)): " << normalTime / static_cast<double>(WORKER_COUNT * queueTime) << std::endl;
	return 0;
}