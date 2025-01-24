/*
 * Copyright 2025 Hillbot Inc.
 * Copyright 2020-2024 UCSD SU Lab
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "sapien/physx/material.h"
#include "sapien/physx/physx_system.h"

using namespace physx;
namespace sapien::physx {

PhysxMaterial::PhysxMaterial(float staticFriction, float dynamicFriction, float restitution)
    : mEngine(PhysxEngine::Get()) {
  mMaterial =
      mEngine->getPxPhysics()->createMaterial(staticFriction, dynamicFriction, restitution);
  mMaterial->setFlag(PxMaterialFlag::eIMPROVED_PATCH_FRICTION, true);
}

PhysxMaterial::~PhysxMaterial() { mMaterial->release(); }

} // namespace sapien::physx
