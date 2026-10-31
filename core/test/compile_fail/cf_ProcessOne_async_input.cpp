#include <gnuradio-4.0/Graph.hpp>

// processOne() reads one item from every input per call, but an asynchronous input is neither counted nor consumed.
struct AsyncInputProcessOne : gr::Block<AsyncInputProcessOne> {
    gr::PortIn<float, gr::Async> in;
    gr::PortOut<float>           out;

    GR_MAKE_REFLECTABLE(AsyncInputProcessOne, in, out);

    [[nodiscard]] constexpr float processOne(float value) const noexcept { return value; }
};

int main() {
    gr::Graph flow;
    std::ignore = flow.emplaceBlock<AsyncInputProcessOne>();
    return 0;
}
