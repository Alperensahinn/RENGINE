#pragma once
#include <UI/Panel.h>
#include <EngineUI/EngineUI.h>

namespace REditor
{
    class DockSpace : public Panel
    {
    public:
        DockSpace();
        ~DockSpace();

        void BeginRender() override;
        void Render() override;
        void EndRender() override;

    private:
        void RenderDockspaceContent();

    };
}