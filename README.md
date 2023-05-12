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

### Components
Each component shoule be defined by a single ```.h``` file in the ```includes/tpublic/Components``` directory, implementing the ```tpublic::ComponentBase``` interface. A component simply some data (maybe with some access methods for convenience) and functionality to read defaults from the data definition, and binary serialization. Flags control persistence (in case of player components) and how replication should be done to clients (only *public* components will be visible on clients).

### Systems
This is where all the heavy logic goes. Systems are updated on a per-entity basis and are done in three ways:

* ```Init```: When an entity (with the system) is created.
* ```UpdatePrivate```: Invoked during world tick. The can read and write *private* components of the entity, but only read from *public* ones. The system can also read *public* components of other entities.
* ```UpdatePublic```: Also invoked during world tick, but after ```UpdatePrivate```. Both *public* and *private* components of the entity can be read and written to, but access of other entities isn't possible.

The reason why updates are separated into two stages is that during each stage, all entities can be updated in parallel, with the caveat being that when a system looks at other entities than its own, it will only see the updates from the previous world tick.
