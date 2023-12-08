#! /bin/sh


#/usr/bin/glslc Syntics/res/shaders/test_bed.vert -o Syntics/res/shaders/spv/test_bed.vert.spv
#/usr/bin/glslc Syntics/res/shaders/test_bed.frag -o Syntics/res/shaders/spv/test_bed.frag.spv

/usr/bin/glslc Syntics/res/shaders/game.vert -o Syntics/res/shaders/spv/game.vert.spv
/usr/bin/glslc Syntics/res/shaders/game.frag -o Syntics/res/shaders/spv/game.frag.spv

/usr/bin/glslc Syntics/res/shaders/notebook.vert -o Syntics/res/shaders/spv/notebook.vert.spv
/usr/bin/glslc Syntics/res/shaders/notebook.frag -o Syntics/res/shaders/spv/notebook.frag.spv

/usr/bin/glslc Syntics/res/shaders/game_grass.vert -o Syntics/res/shaders/spv/game_grass.vert.spv
/usr/bin/glslc Syntics/res/shaders/game_grass.frag -o Syntics/res/shaders/spv/game_grass.frag.spv

/usr/bin/glslc Syntics/res/shaders/gui.vert -o Syntics/res/shaders/spv/gui.vert.spv
/usr/bin/glslc Syntics/res/shaders/gui.frag -o Syntics/res/shaders/spv/gui.frag.spv
#/usr/bin/glslc Syntics/res/shaders/gui_graph.frag -o Syntics/res/shaders/spv/gui_graph.frag.spv
