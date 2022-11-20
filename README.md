Linux dep:
    sudo apt-get install libxkbcommon-x11-dev libx11-xcb-dev
    xfixes
    sudo apt install libxcb-cursor-dev

GLSLANG dep:
  -target_link_libraries(glslang SPIRV MachineIndependent OSDependent GenericCodeGen OGLCompiler)
