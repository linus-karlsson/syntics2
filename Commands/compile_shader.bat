@echo off

C:\VulkanSDK\1.3.236.0\Bin\glslc.exe Syntics/res/shader.vert -o Syntics/res/vert.spv
C:\VulkanSDK\1.3.236.0\Bin\glslc.exe Syntics/res/shader.frag -o Syntics/res/frag.spv

C:\VulkanSDK\1.3.236.0\Bin\glslc.exe Syntics/res/gui.vert -o Syntics/res/gui.vert.spv
C:\VulkanSDK\1.3.236.0\Bin\glslc.exe Syntics/res/gui.frag -o Syntics/res/gui.frag.spv
C:\VulkanSDK\1.3.236.0\Bin\glslc.exe Syntics/res/gui_graph.frag -o Syntics/res/gui_graph.frag.spv

C:\VulkanSDK\1.3.236.0\Bin\glslc.exe Syntics/res/platform_game.vert -o Syntics/res/platform_game.vert.spv
C:\VulkanSDK\1.3.236.0\Bin\glslc.exe Syntics/res/platform_game.frag -o Syntics/res/platform_game.frag.spv

