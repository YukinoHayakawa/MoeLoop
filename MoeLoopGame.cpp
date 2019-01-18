﻿#include "MoeLoopGame.hpp"

#include <fstream>

#include <Usagi/Asset/AssetRoot.hpp>
#include <Usagi/Asset/Package/Filesystem/FilesystemAssetPackage.hpp>
#include <Usagi/Asset/Converter/Uncached/StringAssetConverter.hpp>
#include <Usagi/Core/Logging.hpp>
#include <Usagi/Runtime/Input/InputManager.hpp>
#include <Usagi/Runtime/Input/Mouse/Mouse.hpp>
#include <Usagi/Runtime/Input/Keyboard/Keyboard.hpp>
#include <Usagi/Runtime/Runtime.hpp>
#include <Usagi/Game/GameStateManager.hpp>
#include <Usagi/Interactive/InputMapping.hpp>

#include "Scene/Scene.hpp"
#include "Scene/Character.hpp"
#include "Scene/ImageLayer.hpp"
#include "Game/SceneState.hpp"

namespace usagi::moeloop
{
MoeLoopGame::MoeLoopGame(std::shared_ptr<Runtime> runtime)
    : GraphicalGame(std::move(runtime))
{
    mInputMapping = mRootElement.addChild<InputMapping>("InputMapping");
    auto mouse = mRuntime->inputManager()->virtualMouse();
    auto kb = mRuntime->inputManager()->virtualKeyboard();
    mouse->addEventListener(mInputMapping);
    kb->addEventListener(mInputMapping);

    {
        auto msg = mInputMapping->addActionGroup("Message");
        // todo allow user configuration
        msg->bindKey("NextMessage", KeyCode::ENTER);
        msg->bindMouseButton("NextMessage", MouseButtonCode::LEFT);
        msg->bindMouseButton("NextMessage", MouseButtonCode::WHEEL_DOWN);
    }
}

MoeLoopGame::~MoeLoopGame()
{
    auto mouse = mRuntime->inputManager()->virtualMouse();
    auto kb = mRuntime->inputManager()->virtualKeyboard();
    mouse->removeEventListener(mInputMapping);
    kb->removeEventListener(mInputMapping);

    // remove all states that may reference this game instance.
    mRootElement.removeChild(mStateManager);
}

kaguya::LuaFunction MoeLoopGame::loadScript(const std::string &locator)
{
    LOG(info, "Loading script from asset manager: {}", locator);
    return mLuaContext.loadstring(
        assets()->uncachedRes<StringAssetConverter>(locator)
    );
}

void MoeLoopGame::executeFileScript(const std::string &path)
{
    LOG(info, "Executing script from file: {}", path);
    std::ifstream in(std::filesystem::u8path(path));
    mLuaContext.dostream(in);
}

void MoeLoopGame::executeScript(const std::string &locator)
{
    LOG(info, "Executing script from asset manager: {}", locator);
    mLuaContext.dostring(
        assets()->uncachedRes<StringAssetConverter>(locator)
    );
}

void MoeLoopGame::unimplemented(const std::string &msg)
{
    LOG(warn, "Script: unimplemented code: {}", msg);
}

void MoeLoopGame::bindScript()
{
    using namespace std::placeholders;

    mLuaContext.setErrorHandler([](const int error, const char *msg) {
        LOG(error, "Lua error {}: {}", error, msg);
        throw std::runtime_error("Error occurred in script.");
    });

    mLuaContext["unimplemented"].setFunction(&MoeLoopGame::unimplemented);
    mLuaContext["MoeLoop"].setClass(kaguya::UserdataMetatable<MoeLoopGame>()
        .addFunction("addFilesystemPackage", &MoeLoopGame::addFilesystemPackage)
        .addFunction("changeState", &MoeLoopGame::changeState)
        .addFunction("pushState", &MoeLoopGame::pushState)
        .addFunction("popState", &MoeLoopGame::popState)
        .addFunction("createSceneState", &MoeLoopGame::createSceneState)
        .addFunction("currentScene", &MoeLoopGame::currentScene)
    );

    Scene::exportScript(mLuaContext);
    Character::exportScript(mLuaContext);
    ImageLayer::exportScript(mLuaContext);
}

void MoeLoopGame::setupInput()
{
}

void MoeLoopGame::init()
{
    // the init script should bootstrap the engine, load the assets, then
    // switch to other states that is supposed to interact with the player.
    executeFileScript("init.lua");
}

void MoeLoopGame::addFilesystemPackage(
    std::string name,
    const std::string &path)
{
    assets()->addChild<FilesystemAssetPackage>(
        std::move(name), std::filesystem::u8path(path));
}

void MoeLoopGame::changeState(GameState *state)
{
    mInputMapping->disableAllActionGroups();
    mStateManager->changeState(state);
}

void MoeLoopGame::pushState(GameState *state)
{
    mInputMapping->disableAllActionGroups();
    mStateManager->pushState(state);
}

void MoeLoopGame::popState()
{
    mStateManager->popState();
}

Scene * MoeLoopGame::currentScene() const
{
    if(const auto state = dynamic_cast<SceneState*>(mStateManager->topState()))
    {
        return state->scene();
    }
    return nullptr;
}
}
