#pragma once
#include <cstdint>

class IDemo
{
public:
    virtual ~IDemo() = default;

    virtual bool LoadContent() = 0;
    virtual void UnloadContent() = 0;
    virtual void OnUpdate(float deltaTime) = 0;
    virtual void OnRender() = 0;
    virtual void OnResize(uint32_t width, uint32_t height) {}
    virtual const wchar_t* GetName() const = 0;
};