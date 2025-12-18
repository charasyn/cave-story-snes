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
