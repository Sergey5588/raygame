#include <ode/ode.h>
#include <raylib.h>

// Collision callback structure
typedef struct {
    dWorldID world;
    dJointGroupID contactGroup;
} CollisionData;


typedef struct {
    dBodyID body;
    dGeomID geometry;
    dMass mass;
    Mesh mesh;
} Actor;

// Near callback function for collisions
static void nearCallback(void *data, dGeomID o1, dGeomID o2) {
    CollisionData* colData = (CollisionData*)data;
    dWorldID world = colData->world;
    dJointGroupID contactGroup = colData->contactGroup;

    // Only collide if at least one body is dynamic
    dBodyID b1 = dGeomGetBody(o1);
    dBodyID b2 = dGeomGetBody(o2);
    if (b1 && b2 && dAreConnectedExcluding(b1, b2, dJointTypeContact)) return;

    // Set up contact parameters
    const int max_contacts = 4;
    dContact contact[max_contacts];
    for (int i = 0; i < max_contacts; i++) {
        contact[i].surface.mode = dContactBounce | dContactSoftCFM;
        contact[i].surface.mu = dInfinity;
        contact[i].surface.mu2 = 0;
        contact[i].surface.bounce = 0.5;
        contact[i].surface.bounce_vel = 0.1;
        contact[i].surface.soft_cfm = 0.001;
    }

    // Perform collision detection
    int numc = dCollide(o1, o2, max_contacts, &contact[0].geom, sizeof(dContact));
    
    // Create contact joints
    for (int i = 0; i < numc; i++) {
        dJointID c = dJointCreateContact(world, contactGroup, contact + i);
        dJointAttach(c, b1, b2);
    }
}

int main(void) {
    // Initialize raylib
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "ODE + raylib 3D Physics (Fixed Timestep)");
    SetTargetFPS(120);

    // Setup raylib camera
    Camera camera = { 0 };
    camera.position = (Vector3){ 5.0f, 3.0f, 5.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Initialize ODE
    dInitODE2(0);
    dWorldID world = dWorldCreate();
    dSpaceID space = dSimpleSpaceCreate(0);
    dJointGroupID contactGroup = dJointGroupCreate(0);
    
    // Configure ODE world
    dWorldSetGravity(world, 0, -9.8, 0);
    dWorldSetCFM(world, 1e-5);
    dWorldSetAutoDisableFlag(world, 0); // Disable auto-disabling
    dWorldSetContactMaxCorrectingVel(world, 0.1);
    
    // Create ground plane
    dCreatePlane(space, 0, 1, 0, 0);
    
    // Create dynamic boxes
    #define BODY_COUNT 5
    // dBodyID boxes[BODY_COUNT];
    // dGeomID boxGeoms[BODY_COUNT];
    
    // for (int i = 0; i < BODY_COUNT; i++) {
    //     // Create body
    //     boxes[i] = dBodyCreate(world);
    //     dMass mass;
    //     dMassSetBox(&mass, 1, 1.0, 1.0, 1.0);
    //     dBodySetMass(boxes[i], &mass);
    //     dBodySetPosition(boxes[i], i * 1.5f - 3.0f, 2 + i * 2, 0);
        
    //     // Create geometry
    //     boxGeoms[i] = dCreateBox(space, 1.0, 1.0, 1.0);
    //     dGeomSetBody(boxGeoms[i], boxes[i]);
    // }
    
    Actor scene[BODY_COUNT];
    for (int i = 0; i < BODY_COUNT; i++) {
        // Create body
        scene[i].body = dBodyCreate(world);
        dMass mass;
        dMassSetBox(&mass, 1, 1.0, 1.0, 1.0);
        dBodySetMass(scene[i].body, &mass);
        dBodySetPosition(scene[i].body, i *0.1, 2 + i * 2, 0);
        
        // Create geometry
        //boxGeoms[i] = dCreateBox(space, 1.0, 1.0, 1.0);
        scene[i].geometry = dCreateBox(space, 1.0, 1.0, 1.0);
        dGeomSetBody(scene[i].geometry, scene[i].body);
    }
    // Set up collision data
    CollisionData colData = { world, contactGroup };

    // Fixed timestep variables
    const float physicsStep = 1.0f / 60.0f; // 60 physics updates per second
    double accumulator = 0.0;
    double currentTime = GetTime();
    
    // Main loop
    while (!WindowShouldClose()) {
        // Time management
        double newTime = GetTime();
        double frameTime = newTime - currentTime;
        currentTime = newTime;
        
        // Prevent spiral of death
        if (frameTime > 0.25) frameTime = 0.25;
        
        accumulator += frameTime;
        
        // Physics updates with fixed timestep
        while (accumulator >= physicsStep) {
            dSpaceCollide(space, &colData, nearCallback);
            dWorldStep(world, physicsStep);
            dJointGroupEmpty(contactGroup);
            accumulator -= physicsStep;
        }
        
        // Update camera
        UpdateCamera(&camera, CAMERA_ORBITAL);
        
        // Begin drawing
        BeginDrawing();
            ClearBackground(RAYWHITE);
            
            // 3D rendering
            BeginMode3D(camera);
                // Draw ground
                DrawGrid(20, 1.0f);
                
                // Draw boxes at correct positions
                for (int i = 0; i < BODY_COUNT; i++) {
                    const dReal* pos = dBodyGetPosition(scene[i].body);
                    Color colors[] = { RED, GREEN, BLUE, YELLOW, PURPLE };
                    Vector3 position = { (float)pos[0], (float)pos[1], (float)pos[2] };
                    DrawCube(position, 1.0f, 1.0f, 1.0f, colors[i]);
                    DrawCubeWires(position, 1.0f, 1.0f, 1.0f, BLACK);
                }
            EndMode3D();
            
            // Draw info
            DrawText("ODE + raylib 3D Physics (Fixed Timestep)", 10, 10, 20, DARKGRAY);
            DrawText("Physics independent of FPS", 10, 40, 20, DARKGRAY);
            DrawFPS(10, 70);
        EndDrawing();
    }

    // Cleanup
    for (int i = 0; i < BODY_COUNT; i++) {
        dGeomDestroy(scene[i].geometry);
        dBodyDestroy(scene[i].body);
    }
    dJointGroupDestroy(contactGroup);
    dSpaceDestroy(space);
    dWorldDestroy(world);
    dCloseODE();
    CloseWindow();

    return 0;
}