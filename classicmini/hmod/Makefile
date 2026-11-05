MOD_ID       := emulationstation_classicmini
MOD_NAME     := EmulationStation
MOD_CREATOR  := Alec Lofquist, RetroPie, DefKorns
MOD_CATEGORY := Frontend
MOD_DEPS     := mod/usr/share/emulationstation/emulationstation

all: hmod

SOURCE_DATE_EPOCH ?= $(shell date +%s)

HMOD_BUILD_DIR ?= ../../vendor/hmod-build
include $(HMOD_BUILD_DIR)/hmod.mk

$(MOD_DEPS):
	@echo "Run stage.sh first." >&2; false

.PHONY: all
