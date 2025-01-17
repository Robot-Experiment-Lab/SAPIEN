#include "sapien/physx/physx_engine.h"
#include "../logger.h"
#include "sapien/physx/physx_default.h"

#ifdef SAPIEN_CUDA
#include "../utils/cuda_lib.h"
#include "sapien/utils/cuda.h"
#endif

using namespace physx;

namespace sapien {
namespace physx {

static PxDefaultAllocator gDefaultAllocatorCallback;

class SapienErrorCallback : public PxErrorCallback {
  PxErrorCode::Enum mLastErrorCode = PxErrorCode::eNO_ERROR;

public:
  void reportError(PxErrorCode::Enum code, const char *message, const char *file,
                   int line) override {
    mLastErrorCode = code;

#ifdef NDEBUG
    logger::critical("{}", message);
#else
    logger::critical("{}:{}: {}", file, line, message);
#endif
  }
  PxErrorCode::Enum getLastErrorCode() {
    auto code = mLastErrorCode;
    mLastErrorCode = PxErrorCode::eNO_ERROR;
    return code;
  }
};

static SapienErrorCallback gDefaultErrorCallback;

static std::weak_ptr<PhysxEngine> gEngine;
std::shared_ptr<PhysxEngine> PhysxEngine::Get(float toleranceLength, float toleranceSpeed) {
  auto engine = gEngine.lock();
  if (!engine) {
    gEngine = engine = std::make_shared<PhysxEngine>(toleranceLength, toleranceSpeed);
  }
  return engine;
}

std::shared_ptr<PhysxEngine> PhysxEngine::GetIfExists() { return gEngine.lock(); }

PhysxEngine::PhysxEngine(float toleranceLength, float toleranceSpeed) {
  logger::getLogger();

  mPxFoundation =
      PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
  if (!mPxFoundation) {
    throw std::runtime_error("PhysX foundation creation failed");
  }

  PxTolerancesScale toleranceScale(toleranceLength, toleranceSpeed);

  // if (PhysxDefault::GetGPUEnabled()) {
  //   PxCudaContextManagerDesc cudaContextManagerDesc;

  //   CUcontext context{};
  //   checkCudaDriverErrors(CudaLib::Get().cuCtxGetCurrent(&context));
  //   if (!context) {
  //     throw std::runtime_error("failed to get CUDA context.");
  //   }
  //   cudaContextManagerDesc.ctx = &context;
  //   mCudaContextManager = PxCreateCudaContextManager(*mPxFoundation, cudaContextManagerDesc,
  //                                                    PxGetProfilerCallback());
  // }

  mPxPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mPxFoundation, toleranceScale);
  if (!mPxPhysics) {
    throw std::runtime_error("PhysX creation failed");
  }
  if (!PxInitExtensions(*mPxPhysics, nullptr)) {
    throw std::runtime_error("PhysX extension initialization failed");
  }
}

::physx::PxCudaContextManager *PhysxEngine::getCudaContextManager(int cudaId) {
  if (!PhysxDefault::GetGPUEnabled()) {
    throw std::runtime_error("Using CUDA is not allowed when PhysX GPU is not enabled.");
  }
#if PX_SUPPORT_GPU_PHYSX
  if (mCudaContextManagers.contains(cudaId)) {
    return mCudaContextManagers.at(cudaId);
  }

  PxCudaContextManagerDesc cudaContextManagerDesc;
  CUcontext context{};

  checkCudaErrors(cudaSetDevice(cudaId));

  // NOTE: context initialization seems inconsistent across cuda versions
  // cudaFree(0) is guaranteed to establish context
  checkCudaErrors(cudaFree(0));

  checkCudaDriverErrors(CudaLib::Get().cuCtxGetCurrent(&context));
  if (!context) {
    throw std::runtime_error("failed to get CUDA context.");
  }

  // NOTE: PhysX API really suggests it supports multiple GPUs, but no it doesn't.
  if (!mCudaContextManagers.empty() && mCudaContextManagers.begin()->first != cudaId) {
    throw std::runtime_error(
        "failed to create PhysX on cuda:" + std::to_string(cudaId) +
        ". PhysX only supports a single GPU and a scene has previously been created on cuda:" +
        std::to_string(cudaId) + ".");
  }

  cudaContextManagerDesc.ctx = &context;
  mCudaContextManagers[cudaId] =
      PxCreateCudaContextManager(*mPxFoundation, cudaContextManagerDesc, PxGetProfilerCallback());

  return mCudaContextManagers[cudaId];
#else
  return nullptr;
#endif
  // TODO clean up
}

PhysxEngine::~PhysxEngine() {
  PxCloseExtensions();
  mPxPhysics->release();
  mPxFoundation->release();
}

} // namespace physx
} // namespace sapien
