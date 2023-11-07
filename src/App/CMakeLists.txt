set(SRCS
    main.cpp
    Window.cpp
    Window.h

    Shaders/diffuse.fs
    Shaders/diffuse.vs
    Shaders/cube.fs
    Shaders/cube.vs
    Textures/voronoi.png
#    Models/chess.glb
    Models/Cube.gltf
    Models/Cube.bin

    resources.qrc
)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(demo-app ${SRCS})

target_link_libraries(demo-app
    PRIVATE
        Qt5::Widgets
        FGL::Base
        thirdparty::tinygltf
        thirdparty::glm
)