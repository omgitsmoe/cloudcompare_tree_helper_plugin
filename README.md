# CloudCompare-Plugin: qTreeIso 

Helper plugin to separate point clouds based on scalar or color field values.
Also provides a way to extract a 10&nbsp;cm BHD (1.3&nbsp;m height) slice by selecting
the base point of the trunk.

## Compilation

Place the files in this repo into `CloudCompare\plugins\3rdParty\qTreeIso`
and then compile CloudCompare. The DLL for the plugin will be in the `CloudCompare\plugins` folder.

## Usage

The plugin provides six different actions:

- Split by colour: Extracts all points of the same RGB colour into separate cloud objects
- Split by scalar: Extracts all points with the same value for the active scalar field into separate cloud objects
- Remove lowest points: Tries to isolate the "ground points" in a point cloud (based on a 2d grid)
- BHD slice: Select the base point of a trunk upon activating the plugin action and then a
  10&nbsp;cm slice is extracted from the (tree) point cloud at 1.3&nbsp;m height
- Extract points of same color: Separates all the points of the same color as the picked point
  into a separate cloud object
- Extract points of same color: Extracts all the points of the same color as the picked point
  **into the selected** cloud object

Especially the last two actions were used to quickly isolate trees from the plot-level cloud
in my masters thesis. This way individual parts of the point cloud that were coloured
according to the results of [treeseg's](https://github.com/apburt/treeseg)
(using [PCL's](https://pointclouds.org/)
[region growing](https://pointclouds.org/documentation/classpcl_1_1_region_growing.html) internally)
could be extracted in a faster and less error prone way.
Further code that was used in the master thesis can be found [here](https://github.com/omgitsmoe/boxdim).
