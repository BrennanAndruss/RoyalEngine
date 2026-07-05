#pragma once

#include "Engine/Core/Application.h"

using namespace Royal;

class EditorApp : public Application
{
public:
	explicit EditorApp(const ApplicationConfig& config);

protected:
	void OnInit() override;
	void OnUpdate(float deltaTime) override;
	void OnRender() override;
	void OnShutdown() override;
};