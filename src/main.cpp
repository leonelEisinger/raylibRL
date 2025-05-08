#include "raylib.h"
#include <iostream>

#pragma region imgui
#include "imgui.h"
#include "rlImGui.h"
#include "imguiThemes.h"
#pragma endregion



int main(void)
{

	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(800, 450, "raylib [core] example - basic window");

#pragma region imgui
	rlImGuiSetup(true);

	//you can use whatever imgui theme you like!
	//ImGui::StyleColorsDark();
	//imguiThemes::yellow();
	//imguiThemes::gray();
	//imguiThemes::green();
	//imguiThemes::red();
	imguiThemes::embraceTheDarkness();


	ImGuiIO &io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.FontGlobalScale = 2;

	ImGuiStyle &style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		//style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 0.5f;
		//style.Colors[ImGuiCol_DockingEmptyBg].w = 0.f;
	}

#pragma endregion



	while (!WindowShouldClose())
	{
		const int TILE_SIZE = 20; // tamanho do "caractere" na tela

		// Mapa ASCII
		const char* map[] = {
			"##########",
			"#........#",
			"#........#",
			"#........#",
			"##########"
		};

		int mapWidth = strlen(map[0]);
		int mapHeight = sizeof(map) / sizeof(map[0]);

		int playerX = 4; // posição inicial
		int playerY = 2;

		SetTargetFPS(60); // opcional

		while (!WindowShouldClose())
		{
			// Input
			if (IsKeyPressed(KEY_RIGHT) && map[playerY][playerX + 1] != '#') playerX++;
			if (IsKeyPressed(KEY_LEFT) && map[playerY][playerX - 1] != '#') playerX--;
			if (IsKeyPressed(KEY_UP) && map[playerY - 1][playerX] != '#') playerY--;
			if (IsKeyPressed(KEY_DOWN) && map[playerY + 1][playerX] != '#') playerY++;

			BeginDrawing();
			ClearBackground(RAYWHITE);

			rlImGuiBegin(); // Se quiser manter o ImGui funcionando

			// Desenha o mapa
			for (int y = 0; y < mapHeight; y++) {
				for (int x = 0; x < mapWidth; x++) {
					char tile = (x == playerX && y == playerY) ? '@' : map[y][x];
					DrawText(TextFormat("%c", tile), x * TILE_SIZE + 100, y * TILE_SIZE + 100, TILE_SIZE, DARKGRAY);
				}
			}

			rlImGuiEnd();
			EndDrawing();
		}
	}


#pragma region imgui
	rlImGuiShutdown();
#pragma endregion



	CloseWindow();

	return 0;
}