#!/bin/bash
# 
# Generates new empty terrain of given size
#
source "$(dirname $0)"/shelllib.sh

cd "$(dirname $0)"/..
base=$(pwd)

name="$1"
x="$2"
y="$3"

if [ -z "$name" -o -z "$x" -o -z "$y" ] ; then
	echo  "$0: <name> <x> <y>" >&2
	exit 1
fi

# Clear scratch
rm -fr scratch
mkdir -p scratch
pushd scratch >/dev/null

# Create main template folder
mkdir Terrain-${name}

# Create main config
cat << __HERE__ > Terrain-${name}/Terrain-${name}.cfg 

#############################################################################
#
# This is a basic setup template for terrain.
#
# It provides a base 30x30 config file that can be copied and edited to
# form as the basis for a new chunk of terrain.
#
# To set up a new terrain:
#   1. Create a new directory called Media/Terrain/Terrain-Foo, where
#      "Foo" is the name of your new terrain.
#   2. Copy Media/Terrain/Blank/Terrain-Blank.cfg into that
#      directory, renaming it Terrain-Foo.cfg
#   3. Open Terrain-Foo.cfg in Notepad (or the editor of your choice)
#      and replace all instances of "Blank" with "Terrain-Foo".
#   4. You\'ll probably want to delete this header comment too. ;)
#
#############################################################################


CustomMaterialName=Terrain/Splatting

# Allow per-page options to be set.
PerPageConfig=Blank_x%dy%d.cfg

Texture.Base=Blank_Base_x%dy%d.jpg
Texture.Coverage=Blank_Coverage_x%dy%d.png
Texture.Splatting0=Grass_01.png
Texture.Splatting1=Mud_01.png
Texture.Splatting2=Cobbles_01.png
Texture.Splatting3=Grass_02.png

# The detail texture (if you wish the terrain manager to create a material for you)
DetailTexture=Detail3.jpg

# The number of tiles that should be loaded (and initialized) per frame.
# Setting this too high will cause an FPS hit when new pages are loaded.
# Default: 1
AsyncLoadRate=3

# The radius of pages around the current "primary" page that must be loaded
LivePageMargin=1

#number of times the detail texture will tile in a terrain tile
DetailTile=12

# Heightmap source
PageSource=Heightmap

# Heightmap-source specific settings
Heightmap.image=Blank_Height_x%dy%d.png

PageMaxX=30

PageMaxZ=30

# If you use RAW, fill in the below too
# RAW-specific setting - size (horizontal/vertical)
#Heightmap.raw.size=129
# RAW-specific setting - bytes per pixel (1 = 8bit, 2=16bit)
#Heightmap.raw.bpp=2

# How large is a page of tiles (in vertices)? Must be (2^n)+1
PageSize=129

# How large is each tile? Must be (2^n)+1 and be smaller than PageSize
TileSize=33

# The maximum error allowed when determining which LOD to use
MaxPixelError=8

# The size of a terrain page, in world units
#PageWorldX=2560
#PageWorldZ=2560
#MaxHeight=5120

PageWorldX=1920
PageWorldZ=1920
MaxHeight=1200

#PageWorldX=7680
#PageWorldZ=7680
#MaxHeight=15360

#PageWorldX=10240
#PageWorldZ=10240
#MaxHeight=20480
# Maximum height of the terrain

# Upper LOD limit
MaxMipMapLevel=4

VertexNormals=yes
#VertexColors=yes
#UseTriStrips=yes

# Use vertex program to morph LODs, if available
VertexProgramMorph=yes

# The proportional distance range at which the LOD morph starts to take effect
# This is as a proportion of the distance between the current LODs effective range,
# and the effective range of the next lower LOD
LODMorphStart=0.5

# This following section is for if you want to provide your own terrain shading routine
# Note that since you define your textures within the material this makes the 
# WorldTexture and DetailTexture settings redundant

# The name of the vertex program parameter you wish to bind the morph LOD factor to
# this is 0 when there is no adjustment (highest) to 1 when the morph takes it completely
# to the same position as the next lower LOD
# USE THIS IF YOU USE HIGH-LEVEL VERTEX PROGRAMS WITH LOD MORPHING
MorphLODFactorParamName=MorphFactor

# The index of the vertex program parameter you wish to bind the morph LOD factor to
# this is 0 when there is no adjustment (highest) to 1 when the morph takes it completely
# to the same position as the next lower LOD
# USE THIS IF YOU USE ASSEMBLER VERTEX PROGRAMS WITH LOD MORPHING
#MorphLODFactorParamIndex=4

__HERE__

# Create tiles
sy=0

while [ $sy != $y ]
do
	sx=0
	while [ $sx != $x ]
	do
		mkdir Terrain-${name}_x${sx}y${sy}
		cp ../../sparkplayer-eartheternal/src/assets/Terrain-Blank_x0y0/Blank_Coverage_x0y0.png Terrain-${name}_x${sx}y${sy}/Blank_Coverage_x${sx}y${sy}.png 
		cp ../../sparkplayer-eartheternal/src/assets/Terrain-Blank_x0y0/Blank_Height_x0y0.png Terrain-${name}_x${sx}y${sy}/Blank_Height_x${sx}y${sy}.png
		sx=$(expr $sx + 1)
	done
	sy=$(expr $sy + 1)
done

echo
echo "Snippet for TerrainPages.nut"
echo "---------------------------------------------"
ls -d Terrain-${name}_*|awk '{ print "\t[\"" $0 "\"] = true," '}

echo
echo "Snippet for GroveTemplate.txt"
echo "---------------------------------------------"
echo "${name}	Terrain-${name}	Terrain-${name}#Terrain-${name}.cfg	Fields			0	0	$(expr $x - 1)	$(expr $y - 1)	100	436	100"
popd >/dev/null
