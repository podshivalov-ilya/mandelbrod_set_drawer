#pragma once

#include <atomic>
#include <memory>
#include <functional>
#include <Eigen/Dense>

namespace MTL {
    class Device;
    class CommandQueue;
    class Library;
    class ComputePipelineState;
    class Texture;
    class Function;
    class Buffer;
} // namespace MTL

using MTLDevicePtr = std::unique_ptr<MTL::Device, std::function<void(MTL::Device*)>>;
using MTLCommandQueuePtr = std::unique_ptr<MTL::CommandQueue, std::function<void(MTL::CommandQueue*)>>;
using MTLLibraryPtr = std::unique_ptr<MTL::Library, std::function<void(MTL::Library*)>>;
using MTLComputePipelineStatePtr = std::unique_ptr<MTL::ComputePipelineState, std::function<void(MTL::ComputePipelineState*)>>;
using MTLTexturePtr = std::unique_ptr<MTL::Texture, std::function<void(MTL::Texture*)>>;
using MTLFunctionPtr = std::unique_ptr<MTL::Function, std::function<void(MTL::Function*)>>;
using MTLBufferPtr = std::unique_ptr<MTL::Buffer, std::function<void(MTL::Buffer*)>>;
using RawBufferPtr = std::unique_ptr<uint8_t, std::function<void(uint8_t*)>>;

namespace NS {
    class Error;
} // namespace NS

using NSErrorPtr = std::unique_ptr<NS::Error, std::function<void(NS::Error*)>>;

class MandelbrotSetGenerator final
{
public:
    MandelbrotSetGenerator();
    ~MandelbrotSetGenerator() = default;
    void setSize(const Eigen::Vector2i& size);

    void setScale(float s);
    void setCenter(float x, float y);
    void setMaxIterations(unsigned long maxIt);
    int width() const;
    int height() const;
    float scale() const;
    std::tuple<float, float> center() const;
    unsigned long maxIterations() const;
    bool valid() const;
    RawBufferPtr getImage();
private:
    void initLibrary();
    void initFunction();
    void initComputePipeline();
    void initCommandQueue();
    void initBuffersTextures();

    void setPositionBuffer();
    void setMaxItBuffer();
    void executeKernel();
private:
    MTLDevicePtr device_;
    MTLLibraryPtr library_;
    MTLFunctionPtr mandelbrotMetalFunc_;
    MTLComputePipelineStatePtr computePipeline_;
    MTLCommandQueuePtr commandQueue_;
    MTLTexturePtr texture_;
    MTLBufferPtr positionBuffer_;
    MTLBufferPtr maxItBuffer_;
    NSErrorPtr error_;
    std::atomic_flag condAtomicFlag_;
    Eigen::Vector2i size_;
    float scale_;
    float centerX_;
    float centerY_;
    unsigned long maxIterations_;
    bool initialized_;
};
