﻿#pragma once

#include <vector>

#include <Usagi/Game/CollectionSystem.hpp>
#include <Usagi/Graphics/Game/ProjectiveRenderingSystem.hpp>
#include <Usagi/Transform/TransformComponent.hpp>

#include "SpriteComponent.hpp"

namespace usagi
{
class GpuCommandPool;
class GraphicsPipeline;
class Camera;
class GpuBuffer;
class GpuSampler;
class Game;
}

namespace usagi::negi
{
/**
 * \brief
 * Requires elements to have TransformComponent and SpriteComponent.
 *
 * Sprites are rendered using the texture color as output (unlit material).
 * Prior to rendering, sprites are sorted using provided compare function.
 */
class SortedSpriteRenderingSystem final
    : public ProjectiveRenderingSystem
    , public CollectionSystem<TransformComponent, SpriteComponent>
{
public:
    typedef std::function<bool(
        TransformComponent *lt, SpriteComponent *ls,
        TransformComponent *rt, SpriteComponent *rs
    )> CompareFunc;

private:
    Game *mGame;
    // todo is this really useful?
    CompareFunc mCompareFunc;
    std::vector<Registry::const_iterator> mSortedElements;
    std::shared_ptr<GraphicsPipeline> mPipeline;
    std::shared_ptr<GpuCommandPool> mCommandPool;
    std::shared_ptr<GpuSampler> mSampler;

    struct SpriteIn;

    std::shared_ptr<GpuBuffer> mVertexBuffer;
    std::shared_ptr<GpuBuffer> mIndexBuffer;

    void createBuffers();

public:
    SortedSpriteRenderingSystem(Game *game, CompareFunc compare_func);

    void createRenderTarget(RenderTargetDescriptor &descriptor) override;
    void createPipelines() override;

    void update(const Clock &clock) override;
    std::shared_ptr<GraphicsCommandList> render(const Clock &clock) override;

    const std::type_info & type() override
    {
        return typeid(decltype(*this));
    }
};
}
