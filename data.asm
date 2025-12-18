.include "hdr.asm"

.section ".rodata1" superfree

snesfont:
.incbin "pvsneslibfont.pic"

snespal:
.incbin "pvsneslibfont.pal"

dancermap:
.incbin "dancer.pic"

dancerpal:
.incbin "dancer.pal"


tileset:
.incbin "tiles.pic"
tilesetend:

tilepal:
.incbin "tiles.pal"


mariogfx: .incbin "mario_sprite.pic"
mariogfx_end:

mariopal: .incbin "mario_sprite.pal"

jumpsnd: .incbin "mariojump.brr"
jumpsndend:


PXM_Cave:
.incbin "Cave.cpxm"
PXM_Cave_end:

PAL_Cave:
.incbin "PrtCave_vert.pal"
PAL_Cave_end:

PXA_Cave:
.incbin "Cave.pxa"
PXA_Cave_end:

PXE_Cave:
.incbin "Cave.pxe"
PXE_Cave_end:

UFTC_Cave:
.incbin "PrtCave_vert.pic"
UFTC_Cave_end:


.ends

.section ".rodata2" superfree

mapmario:
.incbin "BG1.m16"

objmario:
.incbin "map_1_1.o16"

tilesetatt:
.incbin "map_1_1.b16"

tilesetdef:
.incbin "map_1_1.t16"

.ends
