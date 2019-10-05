#include "actor_builder.h"
#include "articulation_builder.h"
#include "optifuser_renderer.h"
#include "simulation.h"
#include "urdf/urdf_loader.h"
#include <PxPhysicsAPI.h>
#include <experimental/filesystem>
#include <extensions/PxDefaultAllocator.h>
#include <extensions/PxDefaultCpuDispatcher.h>
#include <extensions/PxDefaultErrorCallback.h>
#include <extensions/PxDefaultSimulationFilterShader.h>
#include <extensions/PxExtensionsAPI.h>
#include <extensions/PxShapeExt.h>
#include <extensions/PxSimpleFactory.h>
#include <foundation/PxMat33.h>
#include <iostream>
#include <object.h>
#include <optifuser.h>
#include <vector>

#include <random>

using namespace physx;
namespace fs = std::experimental::filesystem;

using namespace sapien;

void test1() {
  Renderer::OptifuserRenderer renderer;
  renderer.init();

  Simulation sim;
  sim.setRenderer(&renderer);
  sim.setTimestep(1.f / 60.f);

  auto builder = sim.createActorBuilder();
  for (const auto entry :
       fs::directory_iterator("/home/fx/mobility_mesh/resources/46437-4/objs")) {
    builder->addConvexShapeFromObj(entry.path());
    builder->addObjVisual(entry.path());
  }
  builder->build(true)->setGlobalPose(PxTransform({0, 1, 0}, PxIdentity));

  while (true) {
    sim.step();
    sim.updateRenderer();
    renderer.render();
    if (Optifuser::getInput().getKeyState(GLFW_KEY_Q)) {
      break;
    }
  }
}

void test2() {
  Renderer::OptifuserRenderer renderer;
  renderer.init();

  Simulation sim;
  sim.setRenderer(&renderer);
  sim.setTimestep(1.f / 60.f);

  auto builder = sim.createArticulationBuilder();
  auto link1 = builder->addLink(nullptr);
  builder->addBoxShapeToLink(*link1);
  builder->addBoxVisualToLink(*link1);
  builder->updateLinkMassAndInertia(*link1);

  auto link2 = builder->addLink(link1);
  builder->addBoxShapeToLink(*link2);
  builder->addBoxVisualToLink(*link2);
  builder->updateLinkMassAndInertia(*link2);

  auto joint = static_cast<PxArticulationJointReducedCoordinate *>(link2->getInboundJoint());
  joint->setJointType(PxArticulationJointType::eREVOLUTE);
  joint->setMotion(PxArticulationAxis::eTWIST, PxArticulationMotion::eFREE);
  joint->setParentPose({{0, 0, 0}, PxIdentity});
  joint->setChildPose({{0, 3, 0}, PxIdentity});

  ArticulationWrapper *articulationWrapper = builder->build(true);
  auto articulation = articulationWrapper->articulation;
  auto cache = articulationWrapper->cache;

  auto mesh = Optifuser::NewMeshGrid();
  mesh->position = {0, 0.001, 0};
  renderer.mScene->addObject(std::move(mesh));

  sim.step();
  while (true) {
    sim.step();
    sim.updateRenderer();
    articulation->copyInternalStateToCache(*cache, PxArticulationCache::eALL);
    std::cout << cache->jointPosition[0] << std::endl;
    renderer.render();
    if (Optifuser::getInput().getKeyState(GLFW_KEY_Q)) {
      break;
    }
  }
}

void reset(ArticulationWrapper *wrapper) {
  wrapper->articulation->copyInternalStateToCache(*wrapper->cache, PxArticulationCache::eALL);
  for (size_t i = 0; i < wrapper->articulation->getDofs(); ++i) {
    wrapper->cache->jointPosition[i] = 0;
  }
  wrapper->articulation->applyCache(*wrapper->cache, PxArticulationCache::eALL);
}

float rand_float() {
  float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
  return 2 * r - 1;
}

void test3() {
  Renderer::OptifuserRenderer renderer;
  renderer.init();
  renderer.cam.position = {0, -2, 0.5};
  renderer.cam.setForward({0, 1, 0});
  renderer.cam.setUp({0, 0, 1});

  Simulation sim;
  sim.setRenderer(&renderer);
  sim.setTimestep(1.f / 500.f);

  auto loader = URDF::URDFLoader(sim);
  loader.fixLoadedObject = false;
  // auto *articulationWrapper =
  // loader.load("/home/fx/source/partnet-mobility-scripts/179/test.urdf");
  // auto *articulationWrapper = loader.load("/home/fx/source/partnet-mobility-scripts/46627/test.urdf");

  // int row = 2;
  // for (int i = 0; i < 4; ++i) {
  //   int x = i / row * 2;
  //   int y = i % row * 2;
  //   auto *articulationWrapper = loader.load("../assets/robot/all_robot.urdf");
  //   auto articulation = articulationWrapper->articulation;
  //   articulation->teleportRootLink({{(float)x, (float)y, 0}, PxQuat(rand_float()*3, {0, 0, 1})}, true);
  //   articulationWrapper->updateCache();
  // }

  auto chair = loader.load("../assets/179/test.urdf");
  chair->articulation->teleportRootLink({{1,0,0}, PxIdentity}, true);

  auto *articulationWrapper = loader.load("../assets/robot/all_robot.urdf");

  sim.addGround(-1);

  // reset(articulationWrapper);
  // articulationWrapper->updateArticulation();

  PxArticulationLink *chairLink;
  chair->articulation->getLinks(&chairLink, 1);

  printf("Simulation start\n");
  while (true) {
    chairLink ->addForce({100, 0, 0});

    sim.step();
    sim.updateRenderer();
    renderer.render();
    if (Optifuser::getInput().getKeyState(GLFW_KEY_Q)) {
      break;
    }
  }
}

void test4() {
  Renderer::OptifuserRenderer renderer;
  renderer.init();
  renderer.cam.position = {0, -2, 0.5};
  renderer.cam.setForward({0, 1, 0});
  renderer.cam.setUp({0, 0, 1});

  Simulation sim;
  sim.setRenderer(&renderer);
  sim.setTimestep(1.f / 500.f);

  sim.addGround(-2);

  auto loader = URDF::URDFLoader(sim);
  loader.fixLoadedObject = false;
  loader.loadJointSystem("/home/fx/source/partnet-mobility-scripts/179/test.urdf");

  // auto cache = articulationInfo.cache;
  

  sim.step();

  printf("Simulation start\n");
  while (true) {
    sim.step();
    sim.updateRenderer();
    renderer.render();
    if (Optifuser::getInput().getKeyState(GLFW_KEY_Q)) {
      break;
    }
    // auto &context  = renderer.getOffscreenContext(0);
    // context.renderer.renderScene(*renderer.mScene, renderer.cam);
    // context.renderer.saveLighting("lighting_offscreen.raw");
    // context.renderer.saveDepth("depth_offscreen.raw");
    // context.renderer.saveNormal("normal_offscreen.raw");
  }
}
int main(int argc, char **argv) { test3(); }
