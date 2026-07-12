#include "EditorApp.h"

using namespace Royal;
using namespace Editor;

int main()
{
	ApplicationConfig config;
	config.name = "Editor";
	config.window.width = 1280;
	config.window.height = 720;
	config.window.title = "Royal Editor";

	EditorApp app(config);
	app.Run();

	return 0;
}
