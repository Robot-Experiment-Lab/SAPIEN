#pragma once
#include "camera.h"
#include "render_interface.h"
#include <camera_spec.h>
#include <memory>
#include <optifuser.h>

namespace sapien {
namespace Renderer {

class OptifuserRenderer;
class OptifuserScene;

class OptifuserRigidbody : public IPxrRigidbody {
  OptifuserScene *mParentScene = nullptr;
  physx::PxTransform mInitialPose = {{0, 0, 0}, physx::PxIdentity};
  std::vector<Optifuser::Object *> mObjects;

  uint32_t mUniqueId = 0;
  uint32_t mSegmentationId = 0;

public:
  OptifuserRigidbody(OptifuserScene *scene, std::vector<Optifuser::Object *> const &objects);
  OptifuserRigidbody(OptifuserRigidbody const &other) = delete;

  OptifuserRigidbody &operator=(OptifuserRigidbody const &other) = delete;

  void setUniqueId(uint32_t uniqueId) override;
  uint32_t getUniqueId() const override;
  void setSegmentationId(uint32_t segmentationId) override;
  uint32_t getSegmentationId() const override;
  void setSegmentationCustomData(const std::vector<float> &customData) override;
  void setInitialPose(const physx::PxTransform &transform) override;
  void update(const physx::PxTransform &transform) override;

  void destroy() override;
};

class OptifuserScene : public IPxrScene {
  OptifuserRenderer *mParentRenderer;
  std::unique_ptr<Optifuser::Scene> mScene;
  std::vector<std::unique_ptr<OptifuserRigidbody>> mBodies;
  std::vector<std::unique_ptr<OptifuserCamera>> mCameras;
  std::string mName;

public:
  OptifuserScene(OptifuserRenderer *renderer, std::string const &name = "");

  inline std::string getName() { return mName; }

  Optifuser::Scene *getScene();

  // IPxrScene
  virtual IPxrRigidbody *addRigidbody(const std::string &meshFile,
                                      const physx::PxVec3 &scale) override;
  virtual IPxrRigidbody *addRigidbody(physx::PxGeometryType::Enum type, const physx::PxVec3 &scale,
                                      const physx::PxVec3 &color) override;
  virtual void removeRigidbody(IPxrRigidbody *body) override;

  virtual ICamera *addCamera(std::string const &name, uint32_t width, uint32_t height, float fovx,
                             float fovy, float near, float far,
                             std::string const &shaderDir = "glsl_shader/130") override;
  virtual void removeCamera(ICamera *camera) override;
  virtual std::vector<ICamera *> getCameras() override;

  void destroy() override;
  //======== Lights ========//

  /* For OpenGL only */
  void setAmbientLight(std::array<float, 3> color);

  /* For OpenGL only */
  void setShadowLight(std::array<float, 3> direction, std::array<float, 3> color);

  /* For both OpenGL and OptiX */
  void addPointLight(std::array<float, 3> position, std::array<float, 3> color);

  /* For OptiX Only */
  void addDirectionalLight(std::array<float, 3> direction, std::array<float, 3> color);
};

class OptifuserRenderer : public IPxrRenderer {

  std::vector<std::unique_ptr<OptifuserScene>> mScenes;

public:
  const std::string mGlslDir;

  Optifuser::GLFWRenderContext *mContext = nullptr;
  OptifuserRenderer(std::string const &glslDir = "glsl_shader/130",
                    std::string const &glslVersion = "130");

  // IPxrRenderer
  IPxrScene *createScene(std::string const &name) override;
  void removeScene(IPxrScene *scene) override;
};

} // namespace Renderer
} // namespace sapien
