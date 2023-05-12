# trolddom-public
Gameplay code and content for the Trolddom game. These parts are released as Open Source under the MIT license. Note that this doesn't include all code, it just defines and implements all skills, maps, classes, NPCs, etc.

* ```include/```, ```lib/```: Here you'll find the C++ source code for various gameplay elements and for parsing and serializing data.
* ```data/```: All the data.
* ```util/```: Utility for parsing and verifying data.
* ```extra/```: Third-party source code.

## Huh, what?
You're more than welcome to come up with new stuff for the game. New classes, monsters, anything, or maybe just balance tweaks. Just submit a pull request and I'll try to integrate it into the game. 

## Data
The game is very data-driven and I've put effort into being able to author many different kinds of content, without having to write any code. The ```.txt``` files you can find in the directory structure in ```data/``` defines most of the stuff you can find in the game. These files should be largely self-documenting, but I'll try to make some more comprehensive documentation at some later point.

During the data build step, these files are compiled into *the manifest*, which is a single binary file with all the data shared between clients and servers. 

Maps (dungeons, open world, etc) are made using image files, with each pixel color mapped to a type of tile sprite. To make it easier and faster to edit maps, the data compiler will automatically insert border tiles, as specified in data. The main goal of this approach is to avoid having to make an editor for the game.

## Code
The basic gameplay architecture uses a (fairly) strict [Entity-Component-System (ECS)](https://en.m.wikipedia.org/wiki/Entity_component_system) approach.

All gameplay objects (players, NPCs, etc) are represented by an entity. An entity type, as defined in data, is simply a list of components and systems. Components and systems themselves are defined and implemented in code. Note that the player system isn't included in this public repository, as its tentacles dig too deep into other parts of the server code.
