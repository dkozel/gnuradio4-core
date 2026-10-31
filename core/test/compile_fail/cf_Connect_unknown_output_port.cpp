#include <gnuradio-4.0/Graph.hpp>

struct FloatSource : gr::Block<FloatSource> {
    gr::PortOut<float> out;

    GR_MAKE_REFLECTABLE(FloatSource, out);

    [[nodiscard]] constexpr float processOne() const noexcept { return 0.0f; }
};

struct IntSink : gr::Block<IntSink> {
    gr::PortIn<int> in;

    GR_MAKE_REFLECTABLE(IntSink, in);

    constexpr void processOne(int) const noexcept {}
};

struct FloatSink : gr::Block<FloatSink> {
    gr::PortIn<float> in;

    GR_MAKE_REFLECTABLE(FloatSink, in);

    constexpr void processOne(float) const noexcept {}
};

// the source port name is misspelt
int main() {
    gr::Graph flow;
    auto&     source = flow.emplaceBlock<FloatSource>();
    auto&     sink   = flow.emplaceBlock<FloatSink>();
    std::ignore      = flow.connect<"otu", "in">(source, sink);
    return 0;
}
