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
size_t testTaskQueue(std::function<R(Args...)> task, size_t count, size_t threads, Args... args) {
	tskque::TaskQueue queue(threads);
	Timer timer;

	for (size_t i = 0; i < count; ++i) {
		queue.enqueue(task, std::forward<Args>(args)...);
	}

	queue.join();

	return timer.getTime<milliseconds>();
}

template < class R, class... Args >
size_t testTaskQueue(R(*task)(Args...), size_t count, size_t threads, Args... args) {
	tskque::TaskQueue queue(threads);
	Timer timer;

	for (size_t i = 0; i < count; ++i) {
		queue.enqueue(task, std::forward<Args>(args)...);
	}

	queue.join();

	return timer.getTime<milliseconds>();
}

template < class R, class... Args >
size_t testNormal(std::function<R(Args...)> task, size_t count, Args... args) {
	Timer timer;

	for (size_t i = 0; i < count; ++i) {
		task(std::forward<Args>(args)...);
	}

	return timer.getTime<milliseconds>();
}

template < class R, class... Args >
size_t testNormal(R(*task)(Args...), size_t count, Args... args) {
	Timer timer;

	for (size_t i = 0; i < count; ++i) {
		task(std::forward<Args>(args)...);
	}

	return timer.getTime<milliseconds>();
}

void testTask() {
	volatile double d = 0.0;
	for (size_t i = 1; i < 1000000; ++i) {
		d += std::log10(i);
	}
}

int main() {
	const size_t TASK_COUNT = 500;
	const size_t WORKER_COUNT = 4;
	std::function<void()> task = &testTask;

	std::cout << "Testing task queue with " << WORKER_COUNT << " thread" << ((WORKER_COUNT > 1) ? "s" : "")
			  << " and " << TASK_COUNT << " executions" << std::endl;
	std::cout << "--------------------------------" << std::endl;
	std::cout << std::endl << "With function pointer" << std::endl;
	std::cout << "--------------------------------" << std::endl;
	size_t normalTime = testNormal(&testTask, TASK_COUNT);
	std::cout << "Normal: " << normalTime << "ms" << std::endl;
	size_t queueTime = testTaskQueue(&testTask, TASK_COUNT, WORKER_COUNT);
	std::cout << "Task queue: " << queueTime << "ms" << std::endl;
	std::cout << "Scaling: " << normalTime / static_cast<double>(WORKER_COUNT * queueTime) << std::endl;
	std::cout << "--------------------------------" << std::endl;

	std::cout << std::endl << "with std::function" << std::endl;
	std::cout << "--------------------------------" << std::endl;
	normalTime = testNormal(task, TASK_COUNT);
	std::cout << "Normal: " << normalTime << "ms" << std::endl;
	queueTime = testTaskQueue(task, TASK_COUNT, WORKER_COUNT);
	std::cout << "Task queue: " << queueTime << "ms" << std::endl;
	std::cout << "Scaling: " << normalTime / static_cast<double>(WORKER_COUNT * queueTime) << std::endl;
	std::cout << "--------------------------------" << std::endl;
	return 0;
}