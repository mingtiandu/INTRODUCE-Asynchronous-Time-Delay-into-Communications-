#include <iostream>
#include <chrono>
#include <thread>
#include <coroutine>

// Custom awaitable type for delay
class DelayAwaitable {
public:
    explicit DelayAwaitable(std::chrono::milliseconds duration) : duration_(duration) {}

    bool await_ready() const noexcept {
        return false; // Always suspend coroutine
    }

    void await_suspend(std::coroutine_handle<> handle) {
        // Set up a thread to resume the coroutine after the delay
        std::thread([handle, this]() {
            std::this_thread::sleep_for(duration_);
            handle.resume();
        }).detach();
    }

    void await_resume() const noexcept {
        // No result to return, just resume
    }

private:
    std::chrono::milliseconds duration_;
};

// Define the promise type for the coroutine
struct TaskPromise; // Forward declaration

// Define the Task type that the coroutine will return
struct Task {
    struct promise_type {
        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_never initial_suspend() noexcept {
            return {};
        }

        std::suspend_never final_suspend() noexcept {
            return {};
        }

        void return_void() noexcept {}

        void unhandled_exception() {
            std::exit(1);
        }
    };

    explicit Task(std::coroutine_handle<promise_type> handle) : handle_(handle) {}

    std::coroutine_handle<promise_type> handle_;
};

// Coroutine that uses a while loop with async delay
Task delayedLoopCoroutine() {
    using namespace std::chrono_literals;

    int count = 0;
    while (count < 10) { // Run the loop 10 times
        std::cout << "Loop iteration " << count << std::endl;
        co_await DelayAwaitable(1s); // Delay for 1 second
        ++count; // Increment count
    }
    std::cout << "Loop finished" << std::endl;
}

int main() {
    auto task = delayedLoopCoroutine();
    task.handle_.resume();

    // Sleep the main thread to allow the coroutine to run
    std::this_thread::sleep_for(std::chrono::seconds(15)); // Ensure the coroutine completes
    return 0;
}

