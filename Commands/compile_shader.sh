#! /bin/sh

/usr/bin/glslc Syntics/res/shader.vert -o Syntics/res/vert.spv
/usr/bin/glslc Syntics/res/shader.frag -o Syntics/res/frag.spv

/usr/bin/glslc Syntics/res/gui.vert -o Syntics/res/gui.vert.spv
/usr/bin/glslc Syntics/res/gui.frag -o Syntics/res/gui.frag.spv

/usr/bin/glslc Syntics/res/terrain.vert -o Syntics/res/terrain.vert.spv
/usr/bin/glslc Syntics/res/terrain.frag -o Syntics/res/terrain.frag.spv

