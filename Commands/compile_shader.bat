@echo off

C:\VulkanSDK\1.3.236.0\Bin\glslc.exe Syntics/res/shaders/test_bed.vert -o Syntics/res/shaders/spv/test_bed.vert.spv
C:\VulkanSDK\1.3.236.0\Bin\glslc.exe Syntics/res/shaders/test_bed.frag -o Syntics/res/shaders/spv/test_bed.frag.spv

C:\VulkanSDK\1.3.236.0\Bin\glslc.exe Syntics/res/shaders/game.vert -o Syntics/res/shaders/spv/game.vert.spv
C:\VulkanSDK\1.3.236.0\Bin\glslc.exe Syntics/res/shaders/game.frag -o Syntics/res/shaders/spv/game.frag.spv

C:\VulkanSDK\1.3.236.0\Bin\glslc.exe Syntics/res/shaders/game_car.frag -o Syntics/res/shaders/spv/game_car.frag.spv

C:\VulkanSDK\1.3.236.0\Bin\glslc.exe Syntics/res/shaders/gui.vert -o Syntics/res/shaders/spv/gui.vert.spv
C:\VulkanSDK\1.3.236.0\Bin\glslc.exe Syntics/res/shaders/gui.frag -o Syntics/res/shaders/spv/gui.frag.spv
REM C:\VulkanSDK\1.3.236.0\Bin\glslc.exe Syntics/res/shaders/gui_graph.frag -o Syntics/res/shaders/spv/gui_graph.frag.spv

REM C:\VulkanSDK\1.3.236.0\Bin\glslc.exe Syntics/res/shaders/platform_game.vert -o Syntics/res/shaders/spv/platform_game.vert.spv
REM C:\VulkanSDK\1.3.236.0\Bin\glslc.exe Syntics/res/shaders/platform_game.frag -o Syntics/res/shaders/spv/platform_game.frag.spv

