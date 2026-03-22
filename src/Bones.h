#pragma once

#include <array>
#include <glm/glm.hpp>
#include <utility>

constexpr std::ptrdiff_t BONE_ARRAY_OFFSET = 0x80; // CModelState -> m_pBoneArray

constexpr int MAX_BONES = 30;

// CBoneData
struct BoneData
{
    glm::vec3 location;
    float scale;
    glm::vec4 rotation;
};
static_assert(sizeof(BoneData) == 32, "BoneData must be 32 bytes to match Source 2 layout");

struct BoneMatrix
{
    std::array<BoneData, MAX_BONES> bones{};
    bool valid = false;

    const BoneData &operator[](int index) const
    {
        return bones[index];
    }
    BoneData &operator[](int index)
    {
        return bones[index];
    }
};

constexpr std::array<std::pair<int, int>, 18> SKELETON_BONES = {{
    // Spine
    {0, 2}, // pelvis -> spine lower
    {2, 4}, // spine lower -> spine upper
    {4, 5}, // spine upper -> neck
    {5, 6}, // neck -> head
    // Left arm
    {4, 8},  // spine upper -> left shoulder
    {8, 9},  // left shoulder -> left elbow
    {9, 10}, // left elbow -> left hand
    // Right arm
    {4, 13},  // spine upper -> right shoulder
    {13, 14}, // right shoulder -> right elbow
    {14, 15}, // right elbow -> right hand
    // Left leg
    {0, 22},  // pelvis -> left hip/knee
    {22, 23}, // left knee -> left ankle
    {23, 24}, // left ankle -> left foot
    // Right leg
    {0, 25},  // pelvis -> right hip/knee
    {25, 26}, // right knee -> right ankle
    {26, 27}, // right ankle -> right foot
}};