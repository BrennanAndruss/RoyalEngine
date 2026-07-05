#include "EditorApp.h"

using namespace Royal;

int main()
{
	ApplicationConfig config;
	config.name = "Royal Editor";

	EditorApp app(config);
	app.Run();

	return 0;
}
