#include "MandelbrotSetGenerator.hpp"
#include <SDL_log.h> // TODO: wrap to C++ logger
#include <fmt/core.h>

#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define MTK_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#include <Metal/Metal.hpp>
#include <Foundation/Foundation.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <cmath>
#include <iostream>
#include <functional>

namespace {
    const std::string functionName{"mandelbrot"};
} // namespace

template<typename T>
void refDeleter(NS::Referencing<T> * ref) {
    ref->release();
}

template<typename T>
void refResourceDeleter(NS::Referencing<T, MTL::Resource> * ref) {
    ref->release();
}

template<typename T>
void refCopyingDeleter(NS::Copying<T> * ref) {
    ref->release();
}

void rawBufferDeleter(uint8_t *rawBuffer) {
    delete [] rawBuffer;
}

MandelbrotSetGenerator::MandelbrotSetGenerator()
    : device_(MTL::CreateSystemDefaultDevice(), refDeleter<MTL::Device>),
      library_(nullptr, refDeleter<MTL::Library>),
      mandelbrotMetalFunc_(nullptr, refDeleter<MTL::Function>),
      computePipeline_(nullptr, refDeleter<MTL::ComputePipelineState>),
      commandQueue_(nullptr, refDeleter<MTL::CommandQueue>),
      texture_(nullptr, refResourceDeleter<MTL::Texture>),
      positionBuffer_(nullptr, refResourceDeleter<MTL::Buffer>),
      maxItBuffer_(nullptr, refResourceDeleter<MTL::Buffer>),
      error_(NS::Error::alloc()->init(NS::CocoaErrorDomain, 99, NS::Dictionary::dictionary()), refCopyingDeleter<NS::Error>),
      size_({0, 0}), scale_(0.0), center_({0.0f, 0.0f}),
      maxIterations_(0), initialized_(false) {
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Initializing metal...");
    if (device_ == nullptr)
        throw std::runtime_error("Unable to create device");
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "Device name: %s",
                device_->name()->cString(NS::UTF8StringEncoding));

    initLibrary();
    initFunction();
    initComputePipeline();
    initCommandQueue();

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Metal has been initialized");
}

void MandelbrotSetGenerator::setSize(const Eigen::Vector2i& size) {
    size_ = size;
    initBuffersTextures();
}

void MandelbrotSetGenerator::setScale(float s) {
    scale_ = s;
}

void MandelbrotSetGenerator::setCenter(const Eigen::Vector2f& center) {
    center_ = center;
}

void MandelbrotSetGenerator::setMaxIterations(unsigned long maxIt) {
    maxIterations_ = maxIt;
}

Eigen::Vector2i MandelbrotSetGenerator::size() const {
    return size_;
}

float MandelbrotSetGenerator::scale() const {
    return scale_;
}

Eigen::Vector2f MandelbrotSetGenerator::center() const {
    return center_;
}

unsigned long MandelbrotSetGenerator::maxIterations() const {
    return maxIterations_;
}

bool MandelbrotSetGenerator::valid() const {
    return initialized_ && size_[0] > 0 && size_[1] > 0 && maxIterations_ > 0;
}

void MandelbrotSetGenerator::initLibrary() {
    // METALLIB is a path to binary compiled with macosx sdk and forwarded to C++ from CMake
    auto libPath = NS::String::string(METALLIB, NS::UTF8StringEncoding);
    if (libPath == nullptr)
        throw std::runtime_error("Unable to create string");
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "Path to metal library: %s",
                libPath->cString(NS::UTF8StringEncoding));
    auto url = NS::URL::fileURLWithPath(libPath);

    NS::Error *errRawPtr = error_.get();
    library_.reset(device_->newLibrary(url, &errRawPtr));
    if (library_ == nullptr)
        throw std::runtime_error(error_->localizedDescription()->utf8String());
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "Library type: %ld",
                library_->type());
}

void MandelbrotSetGenerator::initFunction() {
    auto funcName = NS::String::string(functionName.c_str(), NS::UTF8StringEncoding);
    if (funcName == nullptr)
        throw std::runtime_error("Unable to create string");
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "Function name: %s",
                funcName->cString(NS::UTF8StringEncoding));
    mandelbrotMetalFunc_.reset(library_->newFunction(funcName));
    if (mandelbrotMetalFunc_ == nullptr)
        throw std::runtime_error("Unable to create function");
}

void MandelbrotSetGenerator::initComputePipeline() {
    NS::Error *errRawPtr = error_.get();
    computePipeline_.reset(device_->newComputePipelineState(mandelbrotMetalFunc_.get(), &(errRawPtr)));
    if (computePipeline_ == nullptr)
        throw std::runtime_error(error_->localizedDescription()->utf8String());
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "Thread execution width: %lu",
                computePipeline_->threadExecutionWidth());
}

void MandelbrotSetGenerator::initCommandQueue() {
    auto label = NS::String::string(functionName.c_str(), NS::UTF8StringEncoding);
    if (label == nullptr)
        throw std::runtime_error("Unable to create string");
    commandQueue_.reset(device_->newCommandQueue());
    if (commandQueue_ == nullptr)
        throw std::runtime_error("Unable to create command queue");
    commandQueue_->setLabel(label);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
                "Command queue label: %s",
                commandQueue_->label()->cString(NS::UTF8StringEncoding));
}

using MTLTextureDescriptorPtr = std::unique_ptr<MTL::TextureDescriptor, std::function<void(MTL::TextureDescriptor*)>>;

void MandelbrotSetGenerator::initBuffersTextures() {
    MTLTextureDescriptorPtr textureDesc(MTL::TextureDescriptor::alloc()->init(), refDeleter<MTL::TextureDescriptor>);
    if (textureDesc == nullptr)
        throw std::bad_alloc();
    textureDesc->setWidth(size_[0]);
    textureDesc->setHeight(size_[1]);
    textureDesc->setPixelFormat(MTL::PixelFormatRGBA8Uint);
    textureDesc->setTextureType(MTL::TextureType2D);
    textureDesc->setAllowGPUOptimizedContents(true);
    textureDesc->setStorageMode(MTL::StorageModeManaged);
    textureDesc->setUsage(MTL::ResourceUsageSample | MTL::ResourceUsageRead | MTL::ResourceUsageWrite);
    texture_.reset(device_->newTexture(textureDesc.get()));
    if (texture_ == nullptr)
        throw std::bad_alloc();

    positionBuffer_.reset(device_->newBuffer(sizeof(float) * 3, MTL::ResourceStorageModeManaged));
    if (positionBuffer_ == nullptr)
        throw std::bad_alloc();
    maxItBuffer_.reset(device_->newBuffer(sizeof(unsigned long), MTL::ResourceStorageModeManaged));
    if (maxItBuffer_ == nullptr)
        throw std::bad_alloc();
}

void MandelbrotSetGenerator::setPositionBuffer() {
    float* position = reinterpret_cast<float*>(positionBuffer_->contents());
    position[0] = center_[0];
    position[1] = center_[1];
    position[2] = scale_;
    positionBuffer_->didModifyRange(NS::Range::Make(0, sizeof(float) * 3));
}

void MandelbrotSetGenerator::setMaxItBuffer() {
    unsigned long* maxIt = reinterpret_cast<unsigned long*>(maxItBuffer_->contents());
    *maxIt = maxIterations_;
    maxItBuffer_->didModifyRange(NS::Range::Make(0, sizeof(unsigned long)));
}

void MandelbrotSetGenerator::executeKernel() {
    // Command buffer initialization
    auto commandBuf = commandQueue_->commandBuffer();
    if (commandBuf == nullptr)
        throw std::runtime_error("Unable to get command buffer");
    condAtomicFlag_.clear();
    commandBuf->addCompletedHandler([&condAtomicFlag = condAtomicFlag_](MTL::CommandBuffer*) -> void {
                                    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Notifying metal has completed computing");
                                    condAtomicFlag.test_and_set();
                                    condAtomicFlag.notify_one();
                                    });

    setPositionBuffer();
    setMaxItBuffer();

    // Put all the parameters to encoder and start execution of the kernel
    auto computeEncoder = commandBuf->computeCommandEncoder();
    if (computeEncoder == nullptr)
        throw std::runtime_error("Unable to get compute command encoder");
    computeEncoder->setComputePipelineState(computePipeline_.get());
    computeEncoder->setTexture(texture_.get(), 0);
    computeEncoder->setBuffer(positionBuffer_.get(), 0, 0);
    computeEncoder->setBuffer(maxItBuffer_.get(), 0, 1);
    MTL::Size gridSize(texture_->width(), texture_->height(), 1);
    NS::UInteger threadCount = computePipeline_->maxTotalThreadsPerThreadgroup();
    MTL::Size threadGroupSize(threadCount, 1, 1);
    computeEncoder->dispatchThreads(gridSize, threadGroupSize);
    computeEncoder->endEncoding();
    commandBuf->commit();

    // Waiting the computation is done
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Wait for metal finishes computing");
    condAtomicFlag_.wait(false);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Metal has finished computing");
}

RawBufferPtr MandelbrotSetGenerator::getImage() {
    // Lazy initialization and validity check
    if (!initialized_) {
        initBuffersTextures();
        initialized_ = true;
    }
    if (!valid())
        throw std::runtime_error("Drawer wasn't properly initialized");

    executeKernel();
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Texture parameters: width=%lu, height=%lu, bytesPerRow=%lu, bpp=%lu",
                texture_->width(), texture_->height(),
                texture_->bufferBytesPerRow(),
                texture_->bufferBytesPerRow() / texture_->width());

    // Preparing the result
    size_t bytesPerRow = texture_->width() * 4;
    size_t dataSize = bytesPerRow * texture_->height();
    RawBufferPtr data(new uint8_t[dataSize], rawBufferDeleter);
    texture_->getBytes(data.get(), bytesPerRow, MTL::Region(0, 0, texture_->width(), texture_->height()), 0);
    return data;
}
