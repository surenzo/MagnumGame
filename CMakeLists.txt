cmake_minimum_required(VERSION 3.30)
project(MagnumBootstrap)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/modules/" ${CMAKE_MODULE_PATH})
set(CMAKE_CXX_FLAGS "-fpermissive -std=c++0x")
add_compile_options(-Wa,-mbig-obj)
# Tous les fichiers de ta "core logic"
file(GLOB_RECURSE SRC_FILES
        src/*/*.cpp
        src/*/*.hpp)

add_subdirectory(externals)
find_package(Magnum REQUIRED
        GL
        GlfwApplication
        Primitives
        Shaders
        Trade
        MeshTools
        Primitives
        SceneGraph)

find_package(MagnumIntegration REQUIRED ImGui Bullet)
find_package(Bullet REQUIRED Dynamics)

set_directory_properties(PROPERTIES CORRADE_USE_PEDANTIC_FLAGS ON)

# Crée une lib partagée entre les exécutables
add_library(GameCore STATIC ${SRC_FILES}
        src/NetWork/Shared_Input.cpp
        src/NetWork/Shared_Input.h
        src/NetWork/Shared_Objects.cpp
        src/NetWork/Shared_Objects.h
        src/NetWork/Shared_Input.cpp
        src/ECS/Deserialization.cpp
        src/main_server.h)

target_include_directories(GameCore PUBLIC
        inc
        externals/entt/single_include
        externals/enet/include
)

include_directories(${CMAKE_SOURCE_DIR}/externals)

target_link_libraries(GameCore PUBLIC
        Magnum::Application
        Magnum::GL
        Magnum::Magnum
        Magnum::MeshTools
        Magnum::Primitives
        Magnum::SceneGraph
        Magnum::Shaders
        Magnum::Trade
        MagnumIntegration::Bullet
        MagnumIntegration::ImGui
        Bullet::Dynamics
        BulletDynamics
        BulletCollision
        LinearMath
        Bullet3Common
        enet
)

target_sources(GameCore PRIVATE
        src/main.cpp
        src/main_Client.cpp
)
set_property(TARGET GameCore PROPERTY CXX_STANDARD 20)

## Définition de l'exécutable principal
#add_executable(MagnumBootstrap src/main.cpp)
#target_link_libraries(MagnumBootstrap PRIVATE GameCore)

add_executable(true_client src/trueMainClient.cpp src/main.h src/main_Client.h)
target_link_libraries(true_client PRIVATE GameCore)

add_executable(Server src/main_server.cpp)
target_link_libraries(Server PRIVATE GameCore)
#
#add_executable(Client src/main_Client.cpp)
#target_link_libraries(Client PRIVATE GameCore)
