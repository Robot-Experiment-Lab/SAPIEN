#version 450
//
// Copyright 2025 Hillbot Inc.
// Copyright 2020-2024 UCSD SU Lab
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#define SET_NUM 0
#include "./camera_set.glsl"
#undef SET_NUM

#define SET_NUM 1
#include "./object_set.glsl"
#undef SET_NUM

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;

layout(location = 0) out vec4 outPositionRaw;
layout(location = 1) out vec2 outUV;
layout(location = 2) out flat uvec4 outSegmentation;
layout(location = 3) out vec3 objectCoord;
layout(location = 4) out mat3 outTbn;

void main() {
  outSegmentation = objectDataBuffer.segmentation;
  mat4 modelView = cameraBuffer.viewMatrix * objectTransformBuffer.modelMatrix;
  mat3 normalMatrix = mat3(transpose(inverse(modelView)));
  objectCoord = position;
  outPositionRaw = modelView * vec4(position, 1);
  outUV = uv;
  gl_Position = cameraBuffer.projectionMatrix * outPositionRaw;
  vec3 outTangent = normalize(normalMatrix * tangent);
  vec3 outBitangent = normalize(normalMatrix * bitangent);
  vec3 outNormal = normalize(normalMatrix * normal);
  outTbn = mat3(outTangent, outBitangent, outNormal);
}