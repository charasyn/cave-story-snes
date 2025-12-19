#.PRECIOUS: %.asm

ifeq ($(strip $(PVSNESLIB_HOME)),)
$(error "Please create an environment variable PVSNESLIB_HOME by following this guide: https://github.com/alekmaul/pvsneslib/wiki/Installation")
endif

HIROM := 1
FASTROM := 1

# BEFORE including snes_rules :
# list in AUDIOFILES all your .it files in the right order. It will build to generate soundbank file
AUDIOFILES := res/whatislove.it
# then define the path to generate soundbank data. The name can be different but do not forget to update your include in .c file !
export SOUNDBANK := res/soundbank

include ${PVSNESLIB_HOME}/devkitsnes/snes_rules

.PHONY: bitmaps all

#---------------------------------------------------------------------------------
# ROMNAME is used in snes_rules file
export ROMNAME := cave-story-snes

# to build musics, define SMCONVFLAGS with parameters you want, for HiROM use -i
SMCONVFLAGS	:= -s -o $(SOUNDBANK) -i -V -b 3
musics: $(SOUNDBANK).obj

all: musics mariojump.brr bitmaps $(ROMNAME).sfc

clean: cleanBuildRes cleanRom cleanGfx cleanAudio
	@rm -f *.clm mariojump.brr $(STAGE_PICS) $(STAGE_PICS_EGGS)

#---------------------------------------------------------------------------------
pvsneslibfont.pic: pvsneslibfont.bmp
	@echo convert font with no tile reduction ... $(notdir $@)
	$(GFXCONV) -s 8 -o 2 -u 16 -p -e 1 -t bmp -i $<

dancer.pic: dancer.png
	@echo convert font with no tile reduction ... $(notdir $@)
	$(GFXCONV) -s 32 -o 4 -u 16 -p -t png -i $<

mario_sprite.pic: mario_sprite.bmp
	@echo convert sprites ... $(notdir $@)
	$(GFXCONV) -s 16 -o 16 -u 16 -p -t bmp -i $<

tiles.pic: tiles.png
	@echo convert map tileset... $(notdir $@)
	$(GFXCONV) -s 8 -o 16 -u 16 -p -m -i $<

map_1_1.m16: map_1_1.tmj tiles.pic
	@echo convert map tiled ... $(notdir $@)
	$(TMXCONV) $< tiles.map

mariofont.pic: mariofont.bmp
	@echo convert font with no tile reduction ... $(notdir $@)
	$(GFXCONV) -s 8 -o 2 -u 16 -e 1 -p -t bmp -m -R -i $<


# 4. Your catch-all rule (now much simpler)
%.pic: %.png
	@echo converting stage gfx $< ...
	$(GFXCONV) -s 8 -o 16 -u 16 -p -m -R -i $<

# 1. Find ALL png files in res/Stage and its subfolders
# This uses the Linux/Unix 'find' command to look recursively
ALL_STAGE_PNGS := $(shell find res/ -name "*.png")

# 2. Extract just the filenames and change extension to .pic
# This removes the directory path so make looks for "PrtEggs2_vert.pic" in the root
STAGE_PICS := $(patsubst %.png,%.pic,$(notdir $(ALL_STAGE_PNGS)))

# 3. Tell 'make' where to look for source files if they aren't in the root
# We find every subdirectory inside res/Stage/ and add it to the search path
VPATH := $(shell find res/ -type d | tr '\n' ':')

bitmaps : pvsneslibfont.pic dancer.pic PrtCave_vert.pic tiles.pic mariofont.pic map_1_1.m16 mario_sprite.pic $(STAGE_PICS)
