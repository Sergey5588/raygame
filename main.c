
#include "raylib.h"
#include "raymath.h"
#include "include/tinyphysicsengine.h"
#include "math.h"

#define virtualScreenWidth 640
#define virtualScreenHeight 360
#define aspectRatio (float)virtualScreenWidth/(float)virtualScreenHeight

void UpdateResolution(int newHeight, Rectangle *dest) {
	int newWidth = newHeight*aspectRatio;
	float virtualRatio = (float)newWidth/(float)virtualScreenWidth;
	*dest = (Rectangle){ -virtualRatio, -virtualRatio, newWidth + (virtualRatio*2), newHeight + (virtualRatio*2) };
}
typedef struct Player {
    Rectangle hitbox;
} Player;


int main(void)
{
    int screenWidth = 800;
    int screenHeight = 450;
	
	

	float virtualRatio = (float)screenWidth/(float)virtualScreenWidth;
    
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "game");
	
	RenderTexture2D target = LoadRenderTexture(virtualScreenWidth, virtualScreenHeight);
    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 75.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type
	
	Camera2D screenSpaceCamera = { 0 }; // Smoothing camera
    screenSpaceCamera.zoom = 1.0f;
    Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };
	Vector2 origin = (Vector2){0,0};

	Rectangle sourceRec = { 0.0f, 0.0f, (float)target.texture.width, -(float)target.texture.height };
    Rectangle destRec = { -virtualRatio, -virtualRatio, screenWidth + (virtualRatio*2), screenHeight + (virtualRatio*2) };



    
    




	DisableCursor();
    SetTargetFPS(120);                   

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera, CAMERA_FREE);
		if(IsWindowResized()) UpdateResolution(GetScreenHeight(), &destRec);
		if(IsKeyPressed(KEY_F11)) ToggleBorderlessWindowed();

		


		BeginTextureMode(target);
			ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, RED);
                DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, MAROON);

                DrawGrid(100, 1.0f);
				
            EndMode3D();
            DrawFPS(GetScreenWidth() - 95, 10);
		EndTextureMode();

        BeginDrawing();

            ClearBackground(BLACK);
			BeginMode2D(screenSpaceCamera);
                DrawTexturePro(target.texture, sourceRec, destRec, origin, 0.0f, WHITE);
            EndMode2D();
			//DrawFPS(GetScreenWidth() - 95, 10);
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}