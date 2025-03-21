
# Factory Game

By using this repo you are agreeing to the Epic Games EULA - You may not use this code outside of Unreal Engine.

This repo includes a snapshot of the FactoryGame depot. It has had many assets stripped due to incompatible licences.

Redacted Components Include:
- RedpointEOS
- Most audio effects
- Many UI elements
- Non-functional custom shaders
- PS5 & Switch Controller Support
- Texture Pack
- EOS Configs
- Some Custom Shaders
- A few marketplace packs

You are free to do with this repo whatever you like, make it into your own game, take it parts and repurpose them, make parts into open source systems, all I ask is that you credit Daft Software if you do so!

I hope that this repository helps people and that someone can learn a thing or two from it

## Forewarning

While some of the code in this project is of high quality, I was also heavily learning during the process of making it and iterating heavily. Some stuff here is not going to be of quality and there will be bugs!

I recommend taking almost everything in this project with a HEAVY grain of salt - eg do not look to it as an example of how GAS should be used, more how it can be used.

Many many things in this project are extremely opinionated - to the point where many of the opinions I had while writing this code, I no longer hold or recommend. Some of it doesn't adhere to Epic's standards.

Some of the code here is quite fast due to having been aggressively optimized at the cost of readability. On the other hand, some of it runs underwhelmingly slow due to bad optimization techniques or algorithmic inferiority over better methods.

Phyronnaz offers VoxelCore, which really is an amazing library, going back, if I were to do this again I would be heavily leveraging that library and it's optimizations. If you do decide to use the voxel component of this project, I recommend to upgrade lots of parts like the palette compression to use this instead of DIY solutions!

This project uses Mover. There has been a lot of API upgrades and some methods may be incompatible and it does not work properly with Iris, you need to disable Iris in order for the movement to work over the network.

The mesher is unoptimized and a very naive culled mesher, it does not implement any fanciness like greedy meshing.

There is a few undiagnosed / unfixed problems with the voxel code resulting in unexpected issues.

## RedpointEOS

Due to not being able to redistribute june's plugin, the online component of FactoryGame will likely not work. However it is possible to restore it by buying RedpointEOS and then placing it in /Plugins with the correct EOS SDK version.

https://www.fab.com/listings/b900b244-0ff6-49e3-8562-5fc630ba9515

The version of EOS used was Redpoint 2024.06.20 and SDK 1.15.4

## Useful Tips

Voxel Commands:
`FG.ChunkGenBudget`
`FG.FreezeChunkLoading`
`FG.VoxelImmediateMode`
`FG.EnableVoxelMeshing`
`FG.VoxelRenderDistance`
`FG.Mesher.WireframeMode`
`FG.FlushRendering`

Inventory Commands:
`FG.ListItems`
`FG.Give {Item} {Num}`

A lot of the core settings are in Project > FGVoxel Project Settings:
Here you will find the settings to change generator from superflat to natural or mesher from simple to instanced mesh.

Many important assets are setup in Content/CoreGameData, this data asset must be present always.

## Special Thanks to Vori

In particular Vori contributed heavily to this project, helping a lot with networking support and seamless Steam <-> EOS support, contributing on many systems, working on a soundtrack, and spending many many hours helping me debug, flesh out ideas, helping me with algorithms and generally just giving moral support - This project wouldn't have gotten nearly as far as it did without your help.

## Also thank you to other mentors and friends for their help

- MarkJGx
- Phyronnaz
- Mr4Goosey
- vblanco
- GlassBeaver
- Zeblote
- Deathrey
- Ajiiisai
- Eren
- Coombszy
