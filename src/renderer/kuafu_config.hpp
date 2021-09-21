//
// Created by jet on 8/30/21.
//

#pragma once
#include <spdlog/spdlog.h>

namespace sapien::Renderer {
struct KuafuConfig {
  bool mUseViewer = false;
  size_t mViewerWidth = 640;
  size_t mViewerHeight = 640;

  size_t mMaxGeometry = 256;
  size_t mMaxGeometryInstances = 256;
  size_t mMaxTextures = 256;

  std::string mAssetsPath;

  glm::vec4 mClearColor = glm::vec4(0.F, 0.F, 0.F, 1.F);

  uint32_t mPerPixelSampleRate = 32;
  bool mAccumulateFrames = false;

  uint32_t mPathDepth = 5;
  bool mUseDenoiser = false;

//  uint32_t mMaxPathDepth = 10;
//  bool mRussianRoulette = true;
//  uint32_t mRussianRouletteMinBounces = 3;
//
//
//  bool mNextEventEstimation = false;
//  uint32_t mNextEventEstimationMinBounces = 0; // temporary for debugging
//
//  float mVariance = 0.0F;
//  bool mUpdateVariance = false;
//

  std::shared_ptr<kuafu::Config> generate() {
    auto ret = std::make_shared<kuafu::Config>();
    ret->setGeometryLimit(mMaxGeometry);
    ret->setGeometryInstanceLimit(mMaxGeometryInstances);
    ret->setTextureLimit(mMaxTextures);
    ret->setAssetsPath(mAssetsPath);
    ret->setClearColor(mClearColor);
    ret->setPerPixelSampleRate(mPerPixelSampleRate);
    ret->setAccumulatingFrames(mAccumulateFrames);
    ret->setPathDepth(mPathDepth);
    ret->setUseDenoiser(mUseDenoiser);
    ret->setPresent(mUseViewer);
    return ret;
  }
};
}
