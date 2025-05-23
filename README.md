# Magnum Bootstrap
A School project for the course "network" at UQAC.

A self-contained bootstrap for Magnum GLFW with Bullet that should compile on Windows, Linux and MacOS but only tested on Windows (definitely not work on Linux and MacOS).

Have some fun with it, but don't expect too much.

## How to use it:
1. Clone Both repositories ( this one and the API required here : https://github.com/Darkgiant24/MatchMakingAPI)
2. We build it on CLION, but feel free to use your own IDE or only cmake.
3. Start the API server (in the API repository) and make sure it is running. You might have to change Addresses / ports in the configuration file (globals.cpp for engine launch.json for API).

    a. dotnet build
    b. dotnet run

4. Launch the server ( this repository ),  give it in arguments it's own address and the port you want to use (ex : "server.exe localhost:8080").
This step is important because the server run the game loop and isn't created on runtime
5. Launch the client ( this repository ), log in using credentials from the API (playerx, passwordxxx, x being 1-8)
6. You can start playing the matchmaking : depending on your win, you will be matched with players of similar skill level. 

Important note : The server will wait for 4 players to join before starting game so you might have to launch 4 clients. 
( if you run base API, you might use player1, player3, player5 and player7 to test the matchmaking)

## Critical bugs : 
- The server will crash on its reset 
- The server might crash randomly (not often enough to be tested)
- The server have random action if the same client is launched multiple times ( might not work at all )
- The ball is uncontrollable ( depending on the click position of the mouse, it will go / spawn in random direction / position )
- After one game, the client will crash (but will not if you close the client before the crash taking you to the main menu and crash again after joinning game)
- Depending on your machine, the cubes might be shown on debug ( you see the inside of the cube )
- In the game, because network > precision, if you step out too far from the center, the entity will freeze in place (for balls)

- API-side, if a server is opened then closed, players can still access it for matchmaking, causing them to be stuck in the game (but not in the API) and they will have to restart the client to be able to play again.
## Minor bugs :
- A small precision error in the position of every entity ( network was more important than precision )
- Sync is not used so there might be some errors 
- Isn't functionning on POSIX 
- The game is slow (overusing the CPU on the server)
- no interpolation on the client

## Noticed problem if you want to expand it :
The engine (this application) is not great at all : 
- definitely not modular, 
- the network doesn't integrate well with the engine ( the network interact with the engine over shared pointers, but  )
- the ECS part is only slowing the whole process ( due to bad coding : each frame, the registry is copied, should have based the physics on the registry and not the other way around ) 
- the includes are not well organized ( magnum proposed to use a specific method but I didn't use it ) and it slows the compilation too much ( can have up to 10 minutes of compilation time depending on the target )
- the main horror (having some random main becoming later just another class with header files) on src is something 
- I tried to use hierarchy in magnum but caused me some problem (over the network) so have fun.
- Only 1 channel is used (in the network), I don't know the impact, but it is noticeable


## What's great about it :
### we learned a lot :
- First Big step on the network 
- First big step on the game engine
- First big step on the API
- First step on the ECS 
- improvement on the CMake
- improvement on the C++ and some good practices (mostly not applied here sadly)
- Discovery of many libraries (Magnum, Bullet, ImGui, httplib, etc.)

### There are some nice features like :
- Good Serialisation
- Rendering only on the client and physic only on the server
- The API works well 
