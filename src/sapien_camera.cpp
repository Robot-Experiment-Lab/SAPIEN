#include "sapien/sapien_camera.h"
#include "sapien/sapien_scene.h"
#include <glm/gtx/quaternion.hpp>
#include <spdlog/spdlog.h>

namespace sapien {

SCamera::SCamera(SScene *scene, uint32_t width, uint32_t height, float fovy, float near, float far)
    : SEntity(scene), mWidth(width), mHeight(height) {
  if (!scene || !scene->getRendererScene()) {
    throw std::runtime_error("failed to create camera: renderer is not attached");
  }
  mCamera = scene->getRendererScene()->addCamera(width, height, fovy, near, far);
}

void SCamera::setLocalPose(PxTransform const &pose) {
  mLocalPose = pose;
  mPose = getParentPose() * mLocalPose;
}

void SCamera::setParent(SActorBase *actor, bool keepPose) {
  PxTransform p2w{PxIdentity};
  mParent = actor;
  if (actor) {
    p2w = actor->getPose();
  }
  if (keepPose) {
    mLocalPose = p2w.getInverse() * mPose;
  } else {
    mPose = p2w * mLocalPose;
  }
}

void SCamera::update() {
  mPose = getParentPose() * mLocalPose;
  if (mCamera) {
    static PxTransform gl2ros({0, 0, 0}, {-0.5, 0.5, 0.5, -0.5});
    mCamera->setPose(mPose * gl2ros);
  }
}

uint32_t SCamera::getWidth() const { return mWidth; }
uint32_t SCamera::getHeight() const { return mHeight; }

float SCamera::getFocalLengthX() const { return mCamera->getFocalX(); }
float SCamera::getFocalLengthY() const { return mCamera->getFocalY(); }
float SCamera::getFovX() const { return mCamera->getFovX(); }
float SCamera::getFovY() const { return mCamera->getFovY(); }
float SCamera::getNear() const { return mCamera->getNear(); }
float SCamera::getFar() const { return mCamera->getFar(); }
float SCamera::getPrincipalPointX() const { return mCamera->getPrincipalPointX(); }
float SCamera::getPrincipalPointY() const { return mCamera->getPrincipalPointY(); }
float SCamera::getSkew() const { return mCamera->getSkew(); }

void SCamera::setFocalLengths(float fx, float fy) {
  mCamera->setPerspectiveCameraParameters(mCamera->getNear(), mCamera->getFar(), fx, fy,
                                          mCamera->getPrincipalPointX(),
                                          mCamera->getPrincipalPointY(), mCamera->getSkew());
}

void SCamera::setFovX(float fovx, bool computeY) {
  float fx = getWidth() / 2.f / std::tan(fovx / 2);
  float fy = computeY ? fx : mCamera->getFocalY();
  setFocalLengths(fx, fy);
}

void SCamera::setFovY(float fovy, bool computeX) {
  float fy = getHeight() / 2.f / std::tan(fovy / 2);
  float fx = computeX ? fy : mCamera->getFocalX();
  setFocalLengths(fx, fy);
}

void SCamera::setNear(float near) {
  mCamera->setPerspectiveCameraParameters(near, mCamera->getFar(), mCamera->getFocalX(),
                                          mCamera->getFocalY(), mCamera->getPrincipalPointX(),
                                          mCamera->getPrincipalPointY(), mCamera->getSkew());
}

void SCamera::setFar(float far) {
  mCamera->setPerspectiveCameraParameters(mCamera->getNear(), far, mCamera->getFocalX(),
                                          mCamera->getFocalY(), mCamera->getPrincipalPointX(),
                                          mCamera->getPrincipalPointY(), mCamera->getSkew());
}

void SCamera::setPrincipalPoint(float cx, float cy) {
  mCamera->setPerspectiveCameraParameters(mCamera->getNear(), mCamera->getFar(),
                                          mCamera->getFocalX(), mCamera->getFocalY(), cx, cy,
                                          mCamera->getSkew());
}

void SCamera::setSkew(float s) {
  mCamera->setPerspectiveCameraParameters(
      mCamera->getNear(), mCamera->getFar(), mCamera->getFocalX(), mCamera->getFocalY(),
      mCamera->getPrincipalPointX(), mCamera->getPrincipalPointY(), s);
}

void SCamera::setPerspectiveParameters(float near, float far, float fx, float fy, float cx,
                                       float cy, float skew) {
  mCamera->setPerspectiveCameraParameters(near, far, fx, fy, cx, cy, skew);
}

SCamera::~SCamera() {
  // if (mCamera && mParentScene && mParentScene->getRendererScene()) {
  //   mParentScene->getRendererScene()->removeCamera(mCamera);
  // }
}

PxTransform SCamera::getParentPose() const {
  return mParent ? mParent->getPose() : PxTransform(PxIdentity);
}

Mat4 SCamera::getProjectionMatrix() const {
  Mat4 mat = Mat4::Identity();
  float fx = getFocalLengthX();
  float fy = getFocalLengthY();
  float width = getWidth();
  float height = getHeight();
  float far = getFar();
  float near = getNear();
  float cx = getPrincipalPointX();
  float cy = getPrincipalPointY();
  float skew = getSkew();

  mat(0, 0) = (2.f * fx) / width;
  mat(1, 1) = -(2.f * fy) / height;
  mat(2, 2) = -far / (far - near);
  mat(2, 3) = -far * near / (far - near);
  mat(3, 2) = -1.f;
  mat(0, 2) = -2.f * cx / width + 1;
  mat(1, 2) = -2.f * cy / height + 1;
  mat(3, 3) = 0.f;
  mat(0, 1) = -2 * skew / width;
  return mat;
}

Mat4 SCamera::getCameraMatrix() const {
  spdlog::get("SAPIEN")->warn("getCameraMatrix is deprecated, use getIntrinsicMatrix instead");
  Mat4 mat = Mat4::Identity();
  mat(0, 0) = getFocalLengthX();
  mat(1, 1) = getFocalLengthY();
  mat(0, 2) = getPrincipalPointX();
  mat(1, 2) = getPrincipalPointY();
  mat(0, 1) = getSkew();
  return mat;
}

Mat3 SCamera::getIntrinsicMatrix() const {
  Mat3 mat = Mat3::Identity();
  mat(0, 0) = getFocalLengthX();
  mat(1, 1) = getFocalLengthY();
  mat(0, 2) = getPrincipalPointX();
  mat(1, 2) = getPrincipalPointY();
  mat(0, 1) = getSkew();
  return mat;
}

Mat4 SCamera::getExtrinsicMatrix() const {
  // auto pose = getPose().getInverse(); // world2ros
  // glm::quat q(pose.q.w, pose.q.x, pose.q.y, pose.q.z);
  // glm::vec3 p(pose.p.x, pose.p.y, pose.p.z);
  // auto world2ros = glm::translate(glm::mat4(1.0f), p) * glm::toMat4(q);
  // const glm::mat4 ros2opencv{0, 0, 1, 0, -1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 1};
  // return ros2opencv * world2ros;
  return ros2opencv() * toTransformationMatrix(getPose().getInverse());
}

Mat4 SCamera::getModelMatrix() const {
  // auto pose = mCamera->getPose();
  // glm::quat q(pose.q.w, pose.q.x, pose.q.y, pose.q.z);
  // glm::vec3 p(pose.p.x, pose.p.y, pose.p.z);
  // return glm::translate(glm::mat4(1.0f), p) * glm::toMat4(q);
  return toTransformationMatrix(mCamera->getPose());
}

void SCamera::takePicture() { mCamera->takePicture(); }

#ifdef SAPIEN_DLPACK
std::shared_ptr<IAwaitable<std::vector<DLManagedTensor *>>>
SCamera::takePictureAndGetDLTensorsAsync(std::vector<std::string> const &names) {
  return mCamera->takePictureAndGetDLTensorsAsync(mParentScene->getThread(), names);
}
#endif

} // namespace sapien
