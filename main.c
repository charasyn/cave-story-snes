/*---------------------------------------------------------------------------------


    Simple game with map and sprite engine demo
    -- alekmaul


---------------------------------------------------------------------------------*/
#include <snes.h>
#include "common.h"

#include "./res/soundbank.h"

#include "bank_data.h"

//---------------------------------------------------------------------------------
extern char SOUNDBANK__;
extern char jumpsnd, jumpsndend;

//---------------------------------------------------------------------------------
// Physics constants adapted from player.txt
#define PLAYER_MAX_SPEED 675 / 2   // Max horizontal speed
#define PLAYER_ACCEL 71        // Ground acceleration
#define PLAYER_FRICTION 42 / 2     // Ground friction
#define PLAYER_AIR_ACCEL 27    // Air acceleration
#define PLAYER_JUMP_SPEED 1066 // Initial vertical speed of a jump (0x42A)
#define PLAYER_GRAVITY 66 / 2     // Gravity applied when falling (0x42)
#define PLAYER_GRAVITY_JUMP 26 / 2 // Lighter gravity when holding jump button (0x1A)
#define PLAYER_MAX_FALL_SPEED 1279 / 2 // Terminal velocity (0x4FF)

enum
{
    MARIODOWN = 0,
    MARIOJUMPING = 1,
    MARIOWALK = 2,
    MARIOSTAND = 6
}; // Mario state

//---------------------------------------------------------------------------------
extern char tileset, tilesetend, tilepal;
extern char tilesetdef, tilesetatt; // for map & tileset of map

extern char mapmario, objmario;

extern char mariogfx, mariogfx_end;
extern char mariopal;

extern char snesfont, snespal;
//---------------------------------------------------------------------------------
brrsamples Jump; // The sound for jumping

u16 pad0, old_pad0; // pad variables

t_objs *marioobj;             // pointer to mario object
s16 *marioox, *mariooy;       // basics x/y coordinates pointers with fixed point
s16 *marioxv, *marioyv;       // basics x/y velocity pointers with fixed point
u16 mariox, marioy;           // x & y coordinates of mario with map depth (not only screen)
u8 mariofidx, marioflp, flip; // to manage sprite display

//---------------------------------------------------------------------------------
// Init function for mario object
void marioinit(u16 xp, u16 yp, u16 type, u16 minx, u16 maxx)
{
    // to have a little message regarding init (DO NOT USE FOR REAL SNES GAME, JUST DEBUGGING PURPOSE)
    consoleNocashMessage("marioinit %d %d\n", (u16)xp, (u16)yp);

    // prepare new object
    if (objNew(type, xp, yp) == 0)
        // no more space, get out
        return;

    // Init some vars for snes sprite (objgetid is the current object id)
    objGetPointer(objgetid);
    marioobj = &objbuffers[objptr - 1];
    marioobj->width = 16;
    marioobj->height = 16;

    // grab the coordinates & velocity pointers
    marioox = (u16 *)&(marioobj->xpos + 1);
    mariooy = (u16 *)&(marioobj->ypos + 1);
    marioxv = (short *)&(marioobj->xvel);
    marioyv = (short *)&(marioobj->yvel);

    // update some variables for mario
    mariofidx = 0;
    marioflp = 2; // Start with walk animation frames
    flip = 0;

    // prepare dynamic sprite object
    oambuffer[0].oamframeid = 6;
    oambuffer[0].oamrefresh = 1;
    oambuffer[0].oamattribute = 0x60 | (0 << 1); // palette 0 of sprite and sprite 16x16 and priority 2
    oambuffer[0].oamgraphics = &mariogfx;

    // Init Sprites palette
    setPalette(&mariopal, 128 + 0 * 16, 16 * 2);
}

//---------------------------------------------------------------------------------
// Horizontal movement physics from player.txt
void player_update_walk()
{
    s16 acc, fric;
    u8 is_grounded = (marioobj->tilestand != 0);

    if (is_grounded)
    {
        acc = PLAYER_ACCEL;
        fric = PLAYER_FRICTION;
    }
    else
    {
        acc = PLAYER_AIR_ACCEL;
        fric = 0;
    }

    // Apply acceleration based on input
    if (pad0 & KEY_LEFT)
    {
        if (*marioxv > -PLAYER_MAX_SPEED)
            *marioxv -= acc;
    }
    if (pad0 & KEY_RIGHT)
    {
        if (*marioxv < PLAYER_MAX_SPEED)
            *marioxv += acc;
    }

    // Apply friction only on the ground
    if (is_grounded)
    {
        if (abs(*marioxv) <= fric)
            *marioxv = 0;
        else if (*marioxv < 0)
            *marioxv += fric;
        else if (*marioxv > 0)
            *marioxv -= fric;
    }
}

//---------------------------------------------------------------------------------
// Vertical movement (jump and gravity) physics from player.txt
void player_update_jump()
{
    u8 is_grounded = (marioobj->tilestand != 0);

    // Initiate jump if on the ground and jump button is just pressed
    if (is_grounded && (pad0 & KEY_A) && !(old_pad0 & KEY_A))
    {
        *marioyv = -PLAYER_JUMP_SPEED;
        spcPlaySound(0);
    }
    // Apply gravity
    else if (!is_grounded)
    {
        // Lighter gravity if holding jump button (variable jump height)
        if ((pad0 & KEY_A) && (*marioyv < 0))
        {
            *marioyv += PLAYER_GRAVITY_JUMP;
        }
        else
        {
            *marioyv += PLAYER_GRAVITY;
        }

        // Clamp to terminal velocity
        if (*marioyv > PLAYER_MAX_FALL_SPEED)
            *marioyv = PLAYER_MAX_FALL_SPEED;
    }
}

//---------------------------------------------------------------------------------
// Update function for mario object
void marioupdate(u8 idx)
{
    // Get pad value
    pad0 = padsCurrent(0);

    // Update physics based on player.txt logic
    player_update_walk();
    player_update_jump();

    // 1st, check collision with map. This will update marioobj->tilestand.
    objCollidMap(idx);

    // Update animation and sprite direction
    if (marioobj->tilestand != 0) // On the ground
    {
        if (*marioxv == 0) // Standing still
        {
            oambuffer[0].oamframeid = MARIOSTAND;
            oambuffer[0].oamrefresh = 1;
        }
        else // Walking
        {
            // Set sprite direction
            if (*marioxv > 0)
                oambuffer[0].oamattribute |= 0x40; // flip sprite to the right
            else
                oambuffer[0].oamattribute &= ~0x40; // do not flip (left)

            // Update walking animation frame
            flip++;
            if ((flip & 3) == 3)
            {
                mariofidx++;
                mariofidx = mariofidx & 1;
                oambuffer[0].oamframeid = marioflp + mariofidx;
                oambuffer[0].oamrefresh = 1;
            }
        }
    }
    else // In the air
    {
        oambuffer[0].oamframeid = MARIOJUMPING;
        oambuffer[0].oamrefresh = 1;
    }

    // Update position based on velocity
    objUpdateXY(idx);

    // check boundaries
    if (*marioox <= 0)
        *marioox = 0;
    if (*mariooy <= 0)
        *mariooy = 0;

    // change sprite coordinates regarding map location
    mariox = (*marioox);
    marioy = (*mariooy);
    oambuffer[0].oamx = mariox - x_pos;
    oambuffer[0].oamy = marioy - y_pos;
    oamDynamic16Draw(0);

    // update camera regarding mario object
    mapUpdateCamera(mariox, marioy);

    // Store current pad state for next frame
    old_pad0 = pad0;
}


#include "stage.h"
#include "tables.h"

//---------------------------------------------------------------------------------
int main(void)
{
    // Initialize sound engine (take some time)
    spcBoot();

    // Initialize text console with our font
    //consoleSetTextMapPtr(0x6000);
    //consoleSetTextGfxPtr(0x3000);
    //consoleInitText(1, 16 * 2, &snesfont, &snespal);
    // Set give soundbank
    spcSetBank(&SOUNDBANK__);
    // allocate around 10K of sound ram (39 256-byte blocks)
    spcAllocateSoundRegion(39);
    // Load music
    spcLoad(MOD_WHATISLOVE);
    // Load sample
    spcSetSoundEntry(15, 8, 6, &jumpsndend - &jumpsnd, &jumpsnd, &Jump);

    // Init background
    bgSetGfxPtr(1, 0x2000);
    bgSetMapPtr(0, 0x6000, SC_32x32);
    //bgInitTileSet(0, &UFTC_Cave, tileset_info[3].palette, 0, (tileset_info[stageTileset].size*32), 16 * 2, BG_16COLORS, 0x2000);
    bgSetMapPtr(1, 0x6800, SC_32x32);

    bgSetDisable(2);

    // Now Put in 16 color mode and disable Bgs except current
    setMode(BG_MODE1, 0);

    // Draw a wonderful text :P
    // Put some text
    //consoleDrawText(6, 16, "MARIOx00  WORLD TIME");
    //consoleDrawText(6, 17, " 00000 ox00 1-1  000");

    // Wait for nothing :P
    setScreenOn();
    spcPlay(0);
    spcSetModuleVolume(100);

    consoleMesenBreakpoint();

    //stage_load(13);
    // Init sprite engine (0x0000 for large, 0x1000 for small)
    oamInitDynamicSprite(0x0000, 0x1000, 0, 0, OBJ_SIZE8_L16);

    // Object engine activate
    objInitEngine();

    // Init function for state machine
    objInitFunctions(0, &marioinit, &marioupdate, NULL);

    // Load all objects into memory
    objLoadObjects((char *)&objmario);

    uint16_t frame = 0;

    // Initialize camera
    u16 x = 0; 
    u16 y = 0;

    // Force initialization of the graphics on the first frame
    stage_update_screen(x, y); 

    setScreenOn();

    u8 stage_no = 13;

    joy_init();

    while(1) {
        pad0 = padsCurrent(0);

        // Only redraw if inputs actually changed something
        bool moved = false; 

        if (pad0 & KEY_LEFT)  { x -= 1; moved = true; }
        if (pad0 & KEY_RIGHT) { x += 1; moved = true; }
        if (pad0 & KEY_UP)    { y -= 1; moved = true; }
        if (pad0 & KEY_DOWN)  { y += 1; moved = true; }

        if (moved) {
            // Only calculate the new row/col
            // (This function handles the WaitForVBlank + DMA internally)
        //    stage_update_screen(x, y);
        }

        //if(pad0 & KEY_L) {stage_no--; stage_load(stage_no); stage_draw_screen(x, y);}
        //if(pad0 & KEY_R) {stage_no++; stage_load(stage_no); stage_draw_screen(x, y);}

        game_main(0);
        // Update the map regarding the camera
        //mapUpdate();

        // Update all the available objects
        //objUpdateAll();

        // prepare next frame and wait vblank
        //oamInitDynamicSpriteEndFrame();
        spcProcess();
        WaitForVBlank();
        joy_update();
        dmaCopyVram(map_buffer_bg1, 0x6000, map_buffer_bg1);
        dmaCopyVram(map_buffer_bg2, 0x6000, map_buffer_bg2);
        //mapVblank();
        //oamVramQueueUpdate();
    }
    return 0;
    // Init layer with tiles and init also map length 0x6800 is mandatory for map engine
    /*bgSetGfxPtr(1, 0x3000);
    bgSetMapPtr(1, 0x6000, SC_32x32);
    bgInitTileSet(0, &tileset, &tilepal, 0, (&tilesetend - &tileset), 16 * 2, BG_16COLORS, 0x2000);
    bgSetMapPtr(0, 0x6800, SC_64x32);

    // Init sprite engine (0x0000 for large, 0x1000 for small)
    oamInitDynamicSprite(0x0000, 0x1000, 0, 0, OBJ_SIZE8_L16);

    // Object engine activate
    objInitEngine();

    // Init function for state machine
    objInitFunctions(0, &marioinit, &marioupdate, NULL);

    // Load all objects into memory
    objLoadObjects((char *)&objmario);

    // Load map in memory and update it regarding current location of the sprite
    mapLoad((u8 *)&mapmario, (u8 *)&tilesetdef, (u8 *)&tilesetatt);

    // Now Put in 16 color mode and disable BG3
    setMode(BG_MODE1, 0);
    bgSetDisable(2);

    // Put some text
    consoleDrawText(6, 16, "MARIOx00  WORLD TIME");
    consoleDrawText(6, 17, " 00000 ox00 1-1  000");

    // Put screen on
    setScreenOn();

    // Play file from the beginning
    spcPlay(0);
    spcSetModuleVolume(100);

    // Wait VBL 'and update sprites too ;-) )
    WaitForVBlank();

    // Wait for nothing :P
    while (1)
    {
        // Update the map regarding the camera
        mapUpdate();

        // Update all the available objects
        objUpdateAll();

        // prepare next frame and wait vblank
        oamInitDynamicSpriteEndFrame();
        spcProcess();
        WaitForVBlank();
        mapVblank();
        oamVramQueueUpdate();
    }*/
    return 0;
}
