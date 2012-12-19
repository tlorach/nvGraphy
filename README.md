# nvGraphy

nvGraphy is a little tool I developped internally to perform quick (and dirty ;-) overview of coma-separated files (csv).

I never got official time to write this tool. So please don't be surprised if it doesn't look like any regular windowed application.

## What can it do

nvGraphy can help to quickly visualize csv files from following sources:

* Some Graphic API benchmark log. D3D and OpenGL can be dumped in such a way that you could later look at the reports.
* nvPerfHUD NVPMAPI csv log
* Fraps
* generic data (as long as lines start with a 'frame' number...)

The ideas is very simple : the first field will be used to put some values along X-axis : an axis of integers reflecting an array of data

When visualizing a *csv* file, each column will be considered as a specific kind of data to be displayed in its separated graph. Meaning that one csv file could lead to *many* graphs. On the left, there is a way to turn them ON/OFF

When visualizing many csv files, the viewer will attempt to put them in parallel : respective columns of each files will share the same graph. The intend is to help to compare one report with another.

## How to use it

Simply drop the files onto the exe...

Or invoke it from a cmd line :
````
nvGraphy.exe file1.csv files2.csv file3.csv file4.csv ...
````

Some cmd line options are available :

````
-a <annotation path> : where annotations will be saved
-n <common filename> : when sending a list of folders, this name will be appended to each path. I not used while still sending folder list, incremental_call_time_log.csv will be default
````

__Example__:
````
nvGraphy C:\FolderTest\GPU1 C:\FolderTest\GPU2 -a C:\FolderTest -n incremental_call_time_log.csv
````

Implicit load of “incremental_call_time_log.csv” (when you drag n drop some folders to nvGraphy, for example):
````
nvGraphy C:\FolderTest\GPU1 C:\FolderTest\GPU2
````

## Keyboard and Mouse controls
“Help” menu tells a bit about the key shortcuts, but :

* __Left button + mouse move__ : select a range
* __Right button + mouse move__ : scroll on X axis
* __Wheel__ : zoom in/out in X axis. If you zoom a lot, you’ll see points
* __‘z’__ : zoom in the region. If no region, it will zoom out to show all
* __‘Ctrl’ + wheel__ : zoom in Y axis ONLY the the graph on which we are
* __‘Ctrl’ + Right button + drag__ : Scroll on X and Y
* __‘Ctrl’ + ‘Shift’ + right button + drag__ : Scroll on Y ONLY the graph on which we move the mouse
* __Left click on Graph dots__ : select the graph item
* ‘del’ on a selected box : delete the box
* ‘del’ on a selected item of a graph : average the value. Good to remove errors in the graph
* ‘s’ : save these annotations
* ‘r’ : highlight Render targets and related textures
* __‘tight graphs’__ : if unchecked, each graph is independent
* __check boxes for graphs__ : show/hide graphs and graph displays

## Display data

When many benchmark files got generated, you can now take them all and drop them to nvGraphy.

Here we have 2 files : one for a GPU1 and one for a GPU2. We can see that some parts show divergences . This would be the place where we want to start our investigations...

Note that if you place your cursor over the curve, you will get a 'tooltip' showing you values.

![nvGraphy](https://github.com/tlorach/nvGraphy/raw/master/README_pics/NvGraphy_competitiveAPICGraphs.png)


## Infos on selection
If you want to get more details on a specific section of these competitive curves, you can select this range:

![nvGraphy](https://github.com/tlorach/nvGraphy/raw/master/README_pics/NvGraphy_competitiveRangeCompare.png)
This will show:
* the average value for each curve
* the growth of each curve
* the average gap between them
* the gradient of the gap : how fast they diverge/converge. Good to get an idea on how important is the section, on a competitive standpoint.


## FRAPS saved timing

Just drop Fraps log files and you will see them...

![nvGraphy](https://github.com/tlorach/nvGraphy/raw/master/README_pics/NvGraphy_FRAPS_FPS.png)
![nvGraphy](https://github.com/tlorach/nvGraphy/raw/master/README_pics/NvGraphy_FRAPS_Ms.png)

## display generic data
nvGraphy can display generic csv files.
The only restriction is to have a ''frame#'' at the beginning of each line.
````
 Frame, Value1, Value2, value3
 1, 15%, 0.5, 3.1
 2, 25%, 1.5, 3.1
 ...
 40, 1%, 0.0, 0.1
````

(Note that you can use '%'...)

Each column will be displayed on a separate display. 

![nvGraphy](https://github.com/tlorach/nvGraphy/raw/master/README_pics/NvGraphy_generic1.png)

If you want to compare values in the same display, you can provide many files. For example 

 nvGraphy Generic.txt Generic2.txt

will display Generic and Generic2 values in the same displays :

![nvGraphy](https://github.com/tlorach/nvGraphy/raw/master/README_pics/NvGraphy_generic2.png)


## Search
Here is a simple example of searching :
* type the text (*memcpy* in our example) and press *enter*
* items where memcpy exists will be highlighted in yellow

![nvGraphy](https://github.com/tlorach/nvGraphy/raw/master/README_pics/NvGraphy_Search.png)

There is also an option for *Regular expressions* search.

Note that nvGraphy will search in *all the text information available*.


## Annotations
nvGraphy allows you to create simple annotations : on a specific point or on a range of data.

This is rather useful when for example you want to comment a specific drawcall or highlight a specific range of operations that would for example correspond to a specific pass in the scene.

### Annotation on a point
* Select the point
* write the comment in the dedicated text control

![nvGraphy](https://github.com/tlorach/nvGraphy/raw/master/README_pics/NvGraphy_annotPT.png)

### Annotation on a range
* Select a range of data
![nvGraphy](https://github.com/tlorach/nvGraphy/raw/master/README_pics/nvGraphy_rangeannot1.png)
* Write the comment in the dedicated text control
![nvGraphy](https://github.com/tlorach/nvGraphy/raw/master/README_pics/nvGraphy_rangeannot2.png)

You can remove the comment by selecting it and pressing **delete** key...

There is a button to save these annotation : one file for each graph will be create with the related annotations. Later, nvGraphy will look for this file at load time.

__Note__: nvGraphy doesn't save automatically. You will have to do it by yourself before quitting.

## Remove artifacts in the values
Some data may still contain invalid values : Sampling of time could lead to bad spikes which have no meaning at all on the curve. You may want to remove them from the graph.
Select the point
press 'delete'
![nvGraphy](https://github.com/tlorach/nvGraphy/raw/master/README_pics/NvGraphy_correctPT1.png)
![nvGraphy](https://github.com/tlorach/nvGraphy/raw/master/README_pics/NvGraphy_correctPT2.png)

## nvPerfHUD NVPMAPI csv

nvPerfHUD 5 can output its analysis from the NVPMAPI benchmark report.
*F8 menu : this will perform the analysis.
*under the ''state buckets'' you can choose to ''export'' data

Note that nvPerfHUD don't export State buckets information : it only exports timing for each Drawcall.

Simply drop the csv file into nvGraphy:

![nvGraphy](https://github.com/tlorach/nvGraphy/raw/master/README_pics/NvGraphy_nvPerfHUD.png)


