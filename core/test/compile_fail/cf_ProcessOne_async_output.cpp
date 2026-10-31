#include <gnuradio-4.0/Graph.hpp>

// An asynchronous output publishes only what the block requests through its span, which processOne() cannot do.
struct AsyncOutputProcessOne : gr::Block<AsyncOutputProcessOne> {
    gr::PortIn<float>             in;
    gr::PortOut<float, gr::Async> out;

    GR_MAKE_REFLECTABLE(AsyncOutputProcessOne, in, out);

    [[nodiscard]] constexpr float processOne(float value) const noexcept { return value; }
};

int main() {
    gr::Graph flow;
    std::ignore = flow.emplaceBlock<AsyncOutputProcessOne>();
    return 0;
}
