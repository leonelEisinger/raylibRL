#include "raylib.h"
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <raymath.h>

#pragma region imgui
#include "imgui.h"
#include "rlImGui.h"
#include "imguiThemes.h"
#pragma endregion

const int MAP_WIDTH = 80;
const int MAP_HEIGHT = 45;
char map[MAP_HEIGHT][MAP_WIDTH]; // Fixed size array

struct GameCamera {
	Camera2D camera;
	float followSpeed;
	float deadzoneRadius;
	Vector2 playerVelocity;
	Vector2 previousPlayerPos;
};

// Initialize camera system
GameCamera InitGameCamera(int playerX, int playerY, int tileSize) {
	GameCamera gameCam;

	gameCam.camera = { 0 };
	gameCam.camera.target = Vector2{
		static_cast<float>(playerX * tileSize),
		static_cast<float>(playerY * tileSize)
	};
	gameCam.camera.offset = Vector2{
		GetScreenWidth() / 2.0f,
		GetScreenHeight() / 2.0f
	};
	gameCam.camera.rotation = 0.0f;
	gameCam.camera.zoom = 1.0f;

	gameCam.followSpeed = 5.0f;       // Adjust for faster/slower follow
	gameCam.deadzoneRadius = 20.0f;   // Pixels where camera won't move
	gameCam.playerVelocity = { 0, 0 };
	gameCam.previousPlayerPos = gameCam.camera.target;

	return gameCam;
}

// Update camera system
void UpdateGameCamera(GameCamera& gameCam, int playerX, int playerY, int tileSize) {
	// Calculate current and target positions
	Vector2 currentPlayerPos = {
		static_cast<float>(playerX * tileSize),
		static_cast<float>(playerY * tileSize)
	};

	// Calculate player velocity (for prediction)
	gameCam.playerVelocity = Vector2Scale(
		Vector2Subtract(currentPlayerPos, gameCam.previousPlayerPos),
		1.0f / GetFrameTime()
	);
	gameCam.previousPlayerPos = currentPlayerPos;

	// Apply prediction (look ahead)
	Vector2 targetPosition = Vector2Add(
		currentPlayerPos,
		Vector2Scale(gameCam.playerVelocity, 0.1f) // 0.1s prediction
	);

	// Calculate distance from camera to player
	Vector2 cameraToPlayer = Vector2Subtract(targetPosition, gameCam.camera.target);
	float distance = Vector2Length(cameraToPlayer);

	// Only move camera if outside deadzone
	if (distance > gameCam.deadzoneRadius) {
		// Normalize the direction vector
		if (distance > 0) {
			cameraToPlayer = Vector2Scale(cameraToPlayer, 1.0f / distance);
		}

		// Calculate new target position at edge of deadzone
		Vector2 deadzoneEdgePos = Vector2Add(
			gameCam.camera.target,
			Vector2Scale(cameraToPlayer, distance - gameCam.deadzoneRadius)
		);

		// Smoothly move camera toward the deadzone edge position
		float smoothFactor = gameCam.followSpeed * GetFrameTime();
		gameCam.camera.target.x = Lerp(gameCam.camera.target.x, deadzoneEdgePos.x, smoothFactor);
		gameCam.camera.target.y = Lerp(gameCam.camera.target.y, deadzoneEdgePos.y, smoothFactor);
	}

	// Handle window resizing
	if (IsWindowResized()) {
		gameCam.camera.offset = Vector2{
			GetScreenWidth() / 2.0f,
			GetScreenHeight() / 2.0f
		};
	}
}

struct Room {
	int x, y, width, height;

	bool intersects(const Room& other) {
		return (x < other.x + other.width &&
			x + width > other.x &&
			y < other.y + other.height &&
			y + height > other.y);
	}
};

void GenerateMap(int startX, int startY, int maxRooms, int minRoomSize) {
	std::srand(std::time(0));
	std::vector<Room> rooms;

	for (int i = 0; i < maxRooms; i++) {
		int w = minRoomSize + (std::rand() % 6);
		int h = minRoomSize + (std::rand() % 6);
		int x = std::rand() % (MAP_WIDTH - w - 1) + 1;
		int y = std::rand() % (MAP_HEIGHT - h - 1) + 1;

		Room newRoom = { x, y, w, h };
		bool failed = false;

		for (const Room& other : rooms) {
			if (newRoom.intersects(other)) {
				failed = true;
				break;
			}
		}

		if (!failed) {
			// Carve room
			for (int ry = y; ry < y + h; ry++) {
				for (int rx = x; rx < x + w; rx++) {
					map[ry][rx] = '.';
				}
			}

			if (!rooms.empty()) {
				// Connect to previous room
				int prevX = rooms.back().x + rooms.back().width / 2;
				int prevY = rooms.back().y + rooms.back().height / 2;
				int newX = newRoom.x + newRoom.width / 2;
				int newY = newRoom.y + newRoom.height / 2;

				// Horizontal then vertical
				if (std::rand() % 2 == 0) {
					for (int tx = std::min(prevX, newX); tx <= std::max(prevX, newX); tx++) {
						map[prevY][tx] = '.';
					}
					for (int ty = std::min(prevY, newY); ty <= std::max(prevY, newY); ty++) {
						map[ty][newX] = '.';
					}
				}
				// Vertical then horizontal
				else {
					for (int ty = std::min(prevY, newY); ty <= std::max(prevY, newY); ty++) {
						map[ty][prevX] = '.';
					}
					for (int tx = std::min(prevX, newX); tx <= std::max(prevX, newX); tx++) {
						map[newY][tx] = '.';
					}
				}
			}

			rooms.push_back(newRoom);
		}
	}
}

int main(void)
{

		SetConfigFlags(FLAG_WINDOW_RESIZABLE);
		InitWindow(800, 450, "Procedural Map Generator");

#pragma region imgui
		rlImGuiSetup(true);
		imguiThemes::embraceTheDarkness();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.FontGlobalScale = 2;
#pragma endregion

		const int TILE_SIZE = 20;
		int playerX = MAP_WIDTH / 2;
		int playerY = MAP_HEIGHT / 2;

		// Initialize camera
		GameCamera gameCam = InitGameCamera(playerX, playerY, TILE_SIZE);

		// Initialize map with walls
		for (int y = 0; y < MAP_HEIGHT; y++) {
			for (int x = 0; x < MAP_WIDTH; x++) {
				map[y][x] = '#';
			}
		}

		GenerateMap(playerX, playerY, 100, 10);
		map[playerY][playerX] = '.'; // Ensure start position is walkable

		SetTargetFPS(60);

		while (!WindowShouldClose())
		{
			int KeyPressed = GetKeyPressed();
			
			//movement
			switch (KeyPressed) {
				
			//arroy keys movment
			case KEY_RIGHT:
				if (map[playerY][playerX + 1] != '#') {
					playerX++;
				}
				break;
			case KEY_LEFT:
				if (map[playerY][playerX - 1] != '#'){
					playerX--;
				}
				break;
			case KEY_UP:
				if (map[playerY - 1][playerX] != '#') {
					playerY--; 
				}
				break;
			case KEY_DOWN:
				if (map[playerY + 1][playerX] != '#') {
					playerY++; 
				}
				break;

			
			//wasd movement
			case 'D':
				if (map[playerY][playerX + 1] != '#') {
					playerX++;
				}
				break;
			case 'A':
				if (map[playerY][playerX - 1] != '#') {
					playerX--;
				}
				break;
			case 'W':
				if (map[playerY - 1][playerX] != '#') {
					playerY--;
				}
				break;
			case 'S':
				if (map[playerY + 1][playerX] != '#') {
					playerY++;
				}
				break;


			//numpad movement
			case KEY_KP_6:
				if (map[playerY][playerX + 1] != '#') {
					playerX++;
				}
				break;
			case KEY_KP_4:
				if (map[playerY][playerX - 1] != '#') {
					playerX--;
				}
				break;
			case KEY_KP_8:
				if (map[playerY - 1][playerX] != '#') {
					playerY--;
				}
				break;
			case KEY_KP_2:
				if (map[playerY + 1][playerX] != '#') {
					playerY++;
				}
				break;
			case KEY_KP_9:
				if (map[playerY - 1][playerX + 1] != '#') {
					playerX++;
					playerY--;
				}
				break;
			case KEY_KP_7:
				if (map[playerY - 1][playerX - 1] != '#') {
					playerX--;
					playerY--;
				}
				break;
			case KEY_KP_1:
				if (map[playerY + 1][playerX - 1] != '#') {
					playerY++;
					playerX--;
				}
				break;
			case KEY_KP_3:
				if (map[playerY + 1][playerX + 1] != '#') {
					playerY++;
					playerX++;
				}
				break;
			case KEY_KP_5:
				break;
			}

			// Update camera
			UpdateGameCamera(gameCam, playerX, playerY, TILE_SIZE);

			BeginDrawing();
			ClearBackground(BLACK);
			BeginMode2D(gameCam.camera);  // Start 2D mode with camera

			rlImGuiBegin(); // Se quiser manter o ImGui funcionando

			// Desenha o mapa
			// Draw the map with bounds checking
			for (int y = 0; y < MAP_HEIGHT; y++) {
				for (int x = 0; x < MAP_WIDTH; x++) {
					char tile = (x == playerX && y == playerY) ? '@' : map[y][x];
					Color color = (tile == '#') ? DARKGRAY : WHITE;
					DrawText(TextFormat("%c", tile), x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, color);
				}
			}

			rlImGuiEnd();
			EndMode2D();  // End 2D mode with camera
			EndDrawing();
	}

#pragma region imgui
	rlImGuiShutdown();
#pragma endregion



	CloseWindow();

	return 0;
}
