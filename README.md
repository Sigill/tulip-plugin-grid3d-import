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

## LICENSE

This program is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with this program. If not, see <http://www.gnu.org/licenses/>.

