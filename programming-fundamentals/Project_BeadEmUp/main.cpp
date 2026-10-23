#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <string.h>

extern "C" {
#include "./SDL2-2.0.10/include/SDL.h"
#include "./SDL2-2.0.10/include/SDL_main.h"
}

// CONFIGURATION AND CONSTANTS

// Screen settings
#define SCREEN_WIDTH      640
#define SCREEN_HEIGHT     480
#define WINDOW_TITLE      "Project 2 - Beat'em Up - Final"

// Gameplay settings
#define LEVEL_WIDTH       2000
#define PLAYER_SPEED      300.0
#define ATTACK_DURATION   0.3

// Movement zone boundaries 
#define ZONE_TOP          385
#define ZONE_BOTTOM       430

// Interface settings 
#define HUD_HEIGHT        52
#define HUD_MARGIN        4
#define HUD_TEXT_Y_1      8
#define HUD_TEXT_Y_2      22
#define HUD_TEXT_Y_3      36
#define FONT_SIZE         8

// DATA STRUCTURES

enum PlayerState {
    STATE_IDLE,
    STATE_MOVE,
    STATE_ATTACK
};

struct Player {
    double x, y;
    PlayerState currentState;
    double attackTimer;

    int facingRight; // 1 = Right, 0 = Left

    // Graphics - Right (Originals)
    SDL_Surface* spriteIdle;
    SDL_Surface* spriteWalk;
    SDL_Surface* spriteAttack;

    // Graphics - Left (Mirrored)
    SDL_Surface* spriteIdleL;
    SDL_Surface* spriteWalkL;
    SDL_Surface* spriteAttackL;
};

struct Game {
    int quit;
    double worldTime;
    int camera_x;

    Player player;

    SDL_Surface* background;

    // SDL Resources
    SDL_Surface* screen;
    SDL_Surface* charset;
    SDL_Texture* scrtex;
    SDL_Window* window;
    SDL_Renderer* renderer;

    // Colors
    int colorBlack, colorRed, colorBlue;
};

// GRAPHIC PRIMITIVES & HELPER FUNCTIONS

/*
Creates a mirror image of the surface (for left-facing sprites).
Copies pixels row by row in reverse order.
 */
SDL_Surface* FlipHorizontal(SDL_Surface* sfc) {
    if (!sfc) return NULL;

    SDL_Surface* result = SDL_CreateRGBSurface(0, sfc->w, sfc->h,
        sfc->format->BitsPerPixel,
        sfc->format->Rmask, sfc->format->Gmask, sfc->format->Bmask, sfc->format->Amask);

    // Lock surface if required (SDL thread safety)
    if (SDL_MUSTLOCK(sfc)) SDL_LockSurface(sfc);
    if (SDL_MUSTLOCK(result)) SDL_LockSurface(result);

    int pitch = sfc->pitch;
    int bpp = sfc->format->BytesPerPixel;

    for (int y = 0; y < sfc->h; y++) {
        for (int x = 0; x < sfc->w; x++) {
            Uint8* srcPixel = (Uint8*)sfc->pixels + y * pitch + x * bpp;
            Uint8* dstPixel = (Uint8*)result->pixels + y * pitch + (sfc->w - 1 - x) * bpp;
            memcpy(dstPixel, srcPixel, bpp);
        }
    }

    if (SDL_MUSTLOCK(sfc)) SDL_UnlockSurface(sfc);
    if (SDL_MUSTLOCK(result)) SDL_UnlockSurface(result);

    // Copy color key (transparency)
    if (SDL_HasColorKey(sfc)) {
        Uint32 key;
        SDL_GetColorKey(sfc, &key);
        SDL_SetColorKey(result, SDL_TRUE, key);
    }

    return result;
}


// Draws text on the screen using a bitmap font.
void DrawString(SDL_Surface* screen, int x, int y, const char* text, SDL_Surface* charset) {
    int px, py, c;
    SDL_Rect s, d;
    s.w = FONT_SIZE;
    s.h = FONT_SIZE;
    d.w = FONT_SIZE;
    d.h = FONT_SIZE;

    while (*text) {
        c = *text & 255;
        px = (c % 16) * FONT_SIZE;
        py = (c / 16) * FONT_SIZE;
        s.x = px;
        s.y = py;
        d.x = x;
        d.y = y;
        SDL_BlitSurface(charset, &s, screen, &d);
        x += FONT_SIZE;
        text++;
    }
}

/*
Draws a single image (sprite) on the screen.
Note: x, y coordinates represent the center of the image.
 */
void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y) {
    if (!sprite) return;
    SDL_Rect dest;
    dest.x = x - sprite->w / 2;
    dest.y = y - sprite->h / 2;
    dest.w = sprite->w;
    dest.h = sprite->h;
    SDL_BlitSurface(sprite, NULL, screen, &dest);
}

void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
    if (x >= 0 && x < surface->w && y >= 0 && y < surface->h) {
        int bpp = surface->format->BytesPerPixel;
        Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
        *(Uint32*)p = color;
    }
}

void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color) {
    for (int i = 0; i < l; i++) {
        DrawPixel(screen, x, y, color);
        x += dx;
        y += dy;
    }
}

void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k, Uint32 outlineColor, Uint32 fillColor) {
    int i;
    DrawLine(screen, x, y, k, 0, 1, outlineColor);
    DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
    DrawLine(screen, x, y, l, 1, 0, outlineColor);
    DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
    for (i = y + 1; i < y + k - 1; i++) {
        DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
    }
}

// GAME LOGIC

/*
Initializes SDL library, window, and renderer.
Returns 0 on success, 1 on error.
 */
int InitGame(Game* game) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    // Fullscreen Desktop mode
    int rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &game->window, &game->renderer);
    if (rc != 0) {
        SDL_Quit();
        printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
        return 1;
    }

    // Logical scaling (game 'sees' 640x480, GPU scales to full screen)
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    SDL_RenderSetLogicalSize(game->renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
    SDL_SetWindowTitle(game->window, WINDOW_TITLE);

    game->screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    game->scrtex = SDL_CreateTexture(game->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

    SDL_ShowCursor(SDL_DISABLE);

    // Initialize colors
    game->colorBlack = SDL_MapRGB(game->screen->format, 0x00, 0x00, 0x00);
    game->colorRed = SDL_MapRGB(game->screen->format, 0xFF, 0x00, 0x00);
    game->colorBlue = SDL_MapRGB(game->screen->format, 0x11, 0x11, 0xCC);

    return 0;
}


//Helper function to load BMP files with error handling.

SDL_Surface* LoadBMPHelper(const char* file, bool transparent) {
    SDL_Surface* temp = SDL_LoadBMP(file);
    if (!temp) {
        printf("Error loading bitmap: %s\n", file);
        return NULL;
    }
    if (transparent) {
        SDL_SetColorKey(temp, true, 0x000000);
    }
    return temp;
}

//Loads graphics from disk.

int LoadResources(Game* game) {
    game->charset = LoadBMPHelper("./cs8x8.bmp", true);
    if (!game->charset) return 1;

    game->background = LoadBMPHelper("./bg.bmp", false);

    // Load and generate character sprites
    game->player.spriteIdle = LoadBMPHelper("./idle.bmp", true);
    if (!game->player.spriteIdle) return 1;
    game->player.spriteIdleL = FlipHorizontal(game->player.spriteIdle);

    game->player.spriteWalk = LoadBMPHelper("./walk.bmp", true);
    if (!game->player.spriteWalk) return 1;
    game->player.spriteWalkL = FlipHorizontal(game->player.spriteWalk);

    game->player.spriteAttack = LoadBMPHelper("./attack.bmp", true);
    if (!game->player.spriteAttack) return 1;
    game->player.spriteAttackL = FlipHorizontal(game->player.spriteAttack);

    return 0;
}


//Resets the game state to initial values.

void ResetGame(Game* game) {
    game->worldTime = 0;
    game->player.x = 100;
    game->player.y = (ZONE_TOP + ZONE_BOTTOM) / 2;
    game->player.facingRight = 1;
    game->camera_x = 0;
    game->player.currentState = STATE_IDLE;
}


//Frees memory

void FreeResources(Game* game) {
    if (game->charset) SDL_FreeSurface(game->charset);
    if (game->screen) SDL_FreeSurface(game->screen);
    if (game->background) SDL_FreeSurface(game->background);

    if (game->player.spriteIdle) SDL_FreeSurface(game->player.spriteIdle);
    if (game->player.spriteWalk) SDL_FreeSurface(game->player.spriteWalk);
    if (game->player.spriteAttack) SDL_FreeSurface(game->player.spriteAttack);

    if (game->player.spriteIdleL) SDL_FreeSurface(game->player.spriteIdleL);
    if (game->player.spriteWalkL) SDL_FreeSurface(game->player.spriteWalkL);
    if (game->player.spriteAttackL) SDL_FreeSurface(game->player.spriteAttackL);

    SDL_DestroyTexture(game->scrtex);
    SDL_DestroyRenderer(game->renderer);
    SDL_DestroyWindow(game->window);
    SDL_Quit();
}

//Physics logic: character movement, world collisions, camera handling.

void UpdatePhysics(Game* game, double delta) {
    // 1. Handle attack timer
    if (game->player.currentState == STATE_ATTACK) {
        game->player.attackTimer -= delta;
        if (game->player.attackTimer <= 0) {
            game->player.currentState = STATE_IDLE;
        }
    }
    else {
        // 2. Handle input (keyboard)
        game->player.currentState = STATE_IDLE;
        const Uint8* state = SDL_GetKeyboardState(NULL);

        if (state[SDL_SCANCODE_SPACE]) {
            game->player.currentState = STATE_ATTACK;
            game->player.attackTimer = ATTACK_DURATION;
        }
        else {
            // Vertical movement
            if (state[SDL_SCANCODE_UP] || state[SDL_SCANCODE_W]) {
                game->player.y -= PLAYER_SPEED * delta;
                game->player.currentState = STATE_MOVE;
            }
            if (state[SDL_SCANCODE_DOWN] || state[SDL_SCANCODE_S]) {
                game->player.y += PLAYER_SPEED * delta;
                game->player.currentState = STATE_MOVE;
            }
            // Horizontal movement
            if (state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_A]) {
                game->player.x -= PLAYER_SPEED * delta;
                game->player.currentState = STATE_MOVE;
                game->player.facingRight = 0;
            }
            if (state[SDL_SCANCODE_RIGHT] || state[SDL_SCANCODE_D]) {
                game->player.x += PLAYER_SPEED * delta;
                game->player.currentState = STATE_MOVE;
                game->player.facingRight = 1;
            }
        }
    }

    // 3. Movement restrictions (Collision Detection)
    int half_w = 0;
    // Get width from idle sprite (assuming all sprites are similar size)
    if (game->player.spriteIdle) half_w = game->player.spriteIdle->w / 2;

    if (game->player.x < half_w) game->player.x = half_w;
    if (game->player.x > LEVEL_WIDTH - half_w) game->player.x = LEVEL_WIDTH - half_w;
    if (game->player.y < ZONE_TOP) game->player.y = ZONE_TOP;
    if (game->player.y > ZONE_BOTTOM) game->player.y = ZONE_BOTTOM;

    // 4. Update camera
    game->camera_x = (int)game->player.x - SCREEN_WIDTH / 2;
    if (game->camera_x < 0) game->camera_x = 0;
    if (game->camera_x > LEVEL_WIDTH - SCREEN_WIDTH) game->camera_x = LEVEL_WIDTH - SCREEN_WIDTH;
}


//Renders the entire game scene.

void RenderGame(Game* game, double fps) {
    char text[128];

    // Clear background
    SDL_FillRect(game->screen, NULL, game->colorBlack);

    // 1. Draw World (Background)
    if (game->background) {
        int bgW = game->background->w;
        // Background Tiling
        for (int i = 0; i < LEVEL_WIDTH; i += bgW) {
            int screenPos = i - game->camera_x + (bgW / 2);
            // Optimization: Draw only what is visible on screen
            if (screenPos + bgW > 0 && screenPos - bgW < SCREEN_WIDTH) {
                DrawSurface(game->screen, game->background, screenPos, SCREEN_HEIGHT / 2);
            }
        }
    }

    // 2. Draw Player
    SDL_Surface* currentSprite;
    // Choose appropriate sprite based on state and direction
    if (game->player.facingRight) {
        if (game->player.currentState == STATE_ATTACK) currentSprite = game->player.spriteAttack;
        else if (game->player.currentState == STATE_MOVE) currentSprite = game->player.spriteWalk;
        else currentSprite = game->player.spriteIdle;
    }
    else {
        if (game->player.currentState == STATE_ATTACK) currentSprite = game->player.spriteAttackL;
        else if (game->player.currentState == STATE_MOVE) currentSprite = game->player.spriteWalkL;
        else currentSprite = game->player.spriteIdleL;
    }

    DrawSurface(game->screen, currentSprite,
        (int)game->player.x - game->camera_x,
        (int)game->player.y);

    // 3. Draw Interface (HUD)
    DrawRectangle(game->screen, HUD_MARGIN, HUD_MARGIN, SCREEN_WIDTH - 2 * HUD_MARGIN, HUD_HEIGHT, game->colorRed, game->colorBlue);

    // Line 1
    sprintf(text, "Czas: %.1lf s  |  FPS: %.0lf", game->worldTime, fps);
    DrawString(game->screen, SCREEN_WIDTH / 2 - strlen(text) * FONT_SIZE / 2, HUD_TEXT_Y_1, text, game->charset);

    // Line 2
    sprintf(text, "Strzalki/WSAD - ruch | n - nowa gra | esc - wyjscie | Spacja - atak");
    DrawString(game->screen, SCREEN_WIDTH / 2 - strlen(text) * FONT_SIZE / 2, HUD_TEXT_Y_2, text, game->charset);

    // Line 3
    sprintf(text, "Wymagania: 1, 2, 3, 4, A, D");
    DrawString(game->screen, SCREEN_WIDTH / 2 - strlen(text) * FONT_SIZE / 2, HUD_TEXT_Y_3, text, game->charset);

    // Finalize frame
    SDL_UpdateTexture(game->scrtex, NULL, game->screen->pixels, game->screen->pitch);
    SDL_RenderCopy(game->renderer, game->scrtex, NULL, NULL);
    SDL_RenderPresent(game->renderer);
}

// MAIN

#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char** argv) {
    Game game;
    memset(&game, 0, sizeof(Game));

    if (InitGame(&game) != 0) return 1;
    if (LoadResources(&game) != 0) {
        FreeResources(&game); // Always free resources
        return 1;
    }

    ResetGame(&game);

    int t1 = SDL_GetTicks();
    int frames = 0;
    double fpsTimer = 0;
    double fps = 0;

    // Main game loop
    while (!game.quit) {
        int t2 = SDL_GetTicks();
        double delta = (t2 - t1) * 0.001;
        t1 = t2;

        game.worldTime += delta;

        // Logic
        UpdatePhysics(&game, delta);

        // Rendering
        RenderGame(&game, fps);

        // Event handling
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE) game.quit = 1;
                if (event.key.keysym.sym == SDLK_n) ResetGame(&game);
                break;
            case SDL_QUIT:
                game.quit = 1;
                break;
            }
        }

        // FPS Counter
        frames++;
        fpsTimer += delta;
        if (fpsTimer > 0.5) {
            fps = frames * 2.0;
            frames = 0;
            fpsTimer -= 0.5;
        }
    }

    FreeResources(&game);
    return 0;
}