#pragma once

template <class T>
struct App {

    virtual void update(float dt) = 0;
    virtual void imgui_render()   = 0;
    virtual void render()         = 0;
};