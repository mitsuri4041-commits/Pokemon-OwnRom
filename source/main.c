
#include <3ds.h>
#include <citro3d.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#define CLEAR_COLOR 0x87CEEBFF

#define DISPLAY_TRANSFER_FLAGS \
    (GX_TRANSFER_FLIP_VERT(0) | \
     GX_TRANSFER_OUT_TILED(0) | \
     GX_TRANSFER_RAW_COPY(0) | \
     GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | \
     GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | \
     GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

typedef struct
{
    float x;
    float y;
} Player;

typedef struct
{
    int region;
    int hour;
    int weather;
    Player player;
    bool menuOpen;
} Game;

static C3D_RenderTarget *topLeft = NULL;
static C3D_RenderTarget *topRight = NULL;

static Game game =
{
    .region = 0,
    .hour = 12,
    .weather = 0,
    .player = { 0.0f, 0.0f },
    .menuOpen = false
};

static const char *regions[] =
{
    "KANTO",
    "JOHTO",
    "HOENN",
    "SINNOH",
    "UNOVA",
    "KALOS",
    "ALOLA"
};

static void initGraphics(void)
{
    gfxInitDefault();
    gfxSet3D(true);

    consoleInit(GFX_BOTTOM, NULL);

    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);

    topLeft = C3D_RenderTargetCreate(
        240, 400,
        GPU_RB_RGBA8,
        GPU_RB_DEPTH24_STENCIL8
    );

    topRight = C3D_RenderTargetCreate(
        240, 400,
        GPU_RB_RGBA8,
        GPU_RB_DEPTH24_STENCIL8
    );

    if (topLeft)
    {
        C3D_RenderTargetSetOutput(
            topLeft,
            GFX_TOP,
            GFX_LEFT,
            DISPLAY_TRANSFER_FLAGS
        );
    }

    if (topRight)
    {
        C3D_RenderTargetSetOutput(
            topRight,
            GFX_TOP,
            GFX_RIGHT,
            DISPLAY_TRANSFER_FLAGS
        );
    }
}

static void updateGame(void)
{
    hidScanInput();

    u32 down = hidKeysDown();
    u32 held = hidKeysHeld();

    circlePosition circle;
    hidCircleRead(&circle);

    if (down & KEY_START)
        game.menuOpen = !game.menuOpen;

    if (down & KEY_SELECT)
        game.region = (game.region + 1) % 7;

    if (!game.menuOpen)
    {
        if (held & KEY_UP)
            game.player.y += 1.0f;

        if (held & KEY_DOWN)
            game.player.y -= 1.0f;

        if (held & KEY_LEFT)
            game.player.x -= 1.0f;

        if (held & KEY_RIGHT)
            game.player.x += 1.0f;

        game.player.x += circle.dx / 128.0f;
        game.player.y -= circle.dy / 128.0f;
    }
}

static void renderWorld(float eyeOffset)
{
    (void)eyeOffset;

    /*
       3D World Renderer foundation.
       Models, terrain, camera and lighting
       will be added in the next development stages.
    */
}

static void renderGame(void)
{
    if (!topLeft || !topRight)
        return;

    float slider = osGet3DSliderState();
    float iod = slider / 3.0f;

    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

    C3D_RenderTargetClear(
        topLeft,
        C3D_CLEAR_ALL,
        CLEAR_COLOR,
        0
    );

    C3D_FrameDrawOn(topLeft);
    renderWorld(-iod);

    if (iod > 0.0f)
    {
        C3D_RenderTargetClear(
            topRight,
            C3D_CLEAR_ALL,
            CLEAR_COLOR,
            0
        );

        C3D_FrameDrawOn(topRight);
        renderWorld(iod);
    }

    C3D_FrameEnd(0);
}

static void renderMenu(void)
{
    consoleClear();

    printf("================================\n");
    printf("          POKEMON WORLD\n");
    printf("================================\n\n");

    printf("  REGION: %s\n\n", regions[game.region]);

    printf("  PLAYER POSITION\n");
    printf("  X: %.2f\n", game.player.x);
    printf("  Y: %.2f\n\n", game.player.y);

    printf("  [D-PAD] Move\n");
    printf("  [CIRCLE PAD] Move\n");
    printf("  [START] Main Menu\n");
    printf("  [SELECT] Change Region\n\n");

    if (game.menuOpen)
    {
        printf("  ===== MAIN MENU =====\n");
        printf("  POKEMON\n");
        printf("  BAG\n");
        printf("  POKEDEX\n");
        printf("  QUESTS\n");
        printf("  SAVE GAME\n");
    }

    printf("\n================================\n");
    printf("       DEVELOPMENT BUILD\n");
    printf("================================\n");
}

int main(void)
{
    initGraphics();

    while (aptMainLoop())
    {
        updateGame();

        if (hidKeysDown() & KEY_B)
            break;

        renderGame();
        renderMenu();
    }

    if (topLeft)
        C3D_RenderTargetDelete(topLeft);

    if (topRight)
        C3D_RenderTargetDelete(topRight);

    C3D_Fini();
    gfxExit();

    return 0;
}
