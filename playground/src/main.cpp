#include <functional>
#include <memory>
#include <print>
#include <vector>


struct Handler {
    std::vector<std::move_only_function<void()>> callbacks{};
    void register_cb(std::move_only_function<void()> cb) { callbacks.emplace_back(std::move(cb)); }
    void run() {
        for(auto& cb : callbacks)
            cb();
    }
};


int main() {
    auto handler = std::make_unique<Handler>();
    auto* weakRef = handler.get();

    handler->register_cb([h = std::move(handler)] { std::println("i'm fine and the handler is too"); });

    weakRef->run();
}
