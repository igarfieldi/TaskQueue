#ifndef TASKQUEUE_QUEUE_HPP_
#define TASKQUEUE_QUEUE_HPP_

#include <queue>

// TODO: lock-free queue?
namespace tskque {
	template < class T >
	using Queue = std::queue<T>;
} // namespace tskque

#endif //TASKQUEUE_QUEUE_HPP_