#include <boost/ut.hpp>

#include <cstddef>
#include <numeric>
#include <ranges>
#include <span>
#include <vector>

#include <gnuradio-4.0/Block.hpp>
#include <gnuradio-4.0/Graph.hpp>
#include <gnuradio-4.0/Scheduler.hpp>

namespace qa_stride {

constexpr std::size_t kNSamples = 1000UZ;

// publishes 0, 1, 2, ... up to kNSamples and ends the stream
struct RampSource : gr::Block<RampSource> {
    gr::PortOut<float> out;

    GR_MAKE_REFLECTABLE(RampSource, out);

    std::size_t _nPublished = 0UZ;

    gr::work::Status processBulk(gr::OutputSpanLike auto& outSpan) {
        const std::size_t n = std::min(outSpan.size(), kNSamples - _nPublished);
        std::iota(outSpan.begin(), outSpan.begin() + static_cast<std::ptrdiff_t>(n), static_cast<float>(_nPublished));
        outSpan.publish(n);
        _nPublished += n;
        return _nPublished == kNSamples ? gr::work::Status::DONE : gr::work::Status::OK;
    }
};

struct CollectingSink : gr::Block<CollectingSink> {
    gr::PortIn<float> in;

    GR_MAKE_REFLECTABLE(CollectingSink, in);

    std::vector<float> _received;

    void processOne(float value) { _received.push_back(value); }
};

template<gr::Size_t kStride>
struct StridedOne : gr::Block<StridedOne<kStride>, gr::Stride<kStride, true>> {
    gr::PortIn<float>  in;
    gr::PortOut<float> out;

    GR_MAKE_REFLECTABLE(StridedOne, in, out);

    [[nodiscard]] constexpr float processOne(float value) const noexcept { return value; }
};

struct StridedBulk : gr::Block<StridedBulk, gr::Stride<2U, true>> {
    gr::PortIn<float>  in;
    gr::PortOut<float> out;

    GR_MAKE_REFLECTABLE(StridedBulk, in, out);

    gr::work::Status processBulk(std::span<const float> input, std::span<float> output) {
        std::ranges::copy(input, output.begin());
        return gr::work::Status::OK;
    }
};

// declares a stride that is set at run time, zero unless the caller sets one
struct SettableStride : gr::Block<SettableStride, gr::Stride<0U, false>> {
    gr::PortIn<float>  in;
    gr::PortOut<float> out;

    GR_MAKE_REFLECTABLE(SettableStride, in, out);

    [[nodiscard]] constexpr float processOne(float value) const noexcept { return value; }
};

template<typename TBlock>
std::vector<float> runThrough(gr::property_map blockSettings = {}) {
    using namespace boost::ut;
    gr::Graph flow;
    auto&     source = flow.emplaceBlock<RampSource>();
    auto&     block  = flow.emplaceBlock<TBlock>(std::move(blockSettings));
    auto&     sink   = flow.emplaceBlock<CollectingSink>();
    expect(flow.connect<"out", "in">(source, block).has_value());
    expect(flow.connect<"out", "in">(block, sink).has_value());

    gr::scheduler::Simple<> scheduler;
    expect(scheduler.exchange(std::move(flow)).has_value());
    expect(scheduler.runAndWait().has_value());
    return sink._received;
}

std::vector<float> everyNth(std::size_t n) {
    return std::views::iota(0UZ, kNSamples) | std::views::stride(n) | std::views::transform([](std::size_t i) { return static_cast<float>(i); }) | std::ranges::to<std::vector>();
}

} // namespace qa_stride

const boost::ut::suite<"Block with a stride and a 1:1 ratio"> strideTests = [] {
    using namespace boost::ut;
    using namespace qa_stride;

    "a processOne() block declaring Stride<2> passes every other sample"_test = [] { expect(runThrough<StridedOne<2U>>() == everyNth(2UZ)); };

    "a processOne() block declaring Stride<3> passes every third sample"_test = [] { expect(runThrough<StridedOne<3U>>() == everyNth(3UZ)); };

    "a processBulk() block declaring Stride<2> and no Resampling<> passes every other sample"_test = [] { expect(runThrough<StridedBulk>() == everyNth(2UZ)); };

    "a stride equal to the chunk size passes every sample"_test = [] { expect(runThrough<StridedOne<1U>>() == everyNth(1UZ)); };

    "a stride set at run time passes every other sample"_test = [] { expect(runThrough<SettableStride>({{"stride", gr::Size_t{2U}}}) == everyNth(2UZ)); };

    "a stride left at zero passes every sample"_test = [] { expect(runThrough<SettableStride>() == everyNth(1UZ)); };
};

int main() { /* tests are statically registered */ }
