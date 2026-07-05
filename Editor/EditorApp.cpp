#include "EditorApp.h"

#include <iostream>

EditorApp::EditorApp(const ApplicationConfig& config)
	: Application(config)
{

}

void EditorApp::OnInit()
{
	std::cout << "Hello Royal Engine!" << std::endl;
}

void EditorApp::OnUpdate(float deltaTime)
{

}

void EditorApp::OnRender()
{

}

void EditorApp::OnShutdown()
{

}