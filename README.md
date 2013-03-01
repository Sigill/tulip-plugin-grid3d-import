# Tulip Grid3D import plugin

## Description

This plugin allow to generate graphs whose structure is a 3D grid

## Build

Launch one of the CMake project configuration tool and select your build directory. Set the CMAKE_MODULE_PATH variable to the location of the FindTULIP3.cmake file (should be &lt;tulip_install_dir&gt;/share/tulip).

More informations [here](http://tulip.labri.fr/TulipDrupal/?q=node/1481).

## Use

The plugin needs the following parameters:

 * _Width, Height, Depth_: Integers indicating the dimensions of the graph.
 * _Connectivity_: StringCollection indicating the connectivity of the graph ("0", "4" or "8").
 * _Positionning_: Boolean indicating if the nodes should be positionned in space.
 * _Spacing_: Double indicating the space between each nodes.
