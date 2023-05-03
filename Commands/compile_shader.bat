@echo off

C:\VulkanSDK\1.3.236.0\Bin\glslc.exe Syntics/res/game.vert -o Syntics/res/game.vert.spv
C:\VulkanSDK\1.3.236.0\Bin\glslc.exe Syntics/res/game.frag -o Syntics/res/game.frag.spv

C:\VulkanSDK\1.3.236.0\Bin\glslc.exe Syntics/res/gui.vert -o Syntics/res/gui.vert.spv
C:\VulkanSDK\1.3.236.0\Bin\glslc.exe Syntics/res/gui.frag -o Syntics/res/gui.frag.spv
C:\VulkanSDK\1.3.236.0\Bin\glslc.exe Syntics/res/gui_graph.frag -o Syntics/res/gui_graph.frag.spv

REM C:\VulkanSDK\1.3.236.0\Bin\glslc.exe Syntics/res/platform_game.vert -o Syntics/res/platform_game.vert.spv
REM C:\VulkanSDK\1.3.236.0\Bin\glslc.exe Syntics/res/platform_game.frag -o Syntics/res/platform_game.frag.spv

