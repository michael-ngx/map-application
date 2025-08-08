# Geographic Information System (GIS)

[Demo](https://www.youtube.com/watch?v=ntBSIfVKuzo&t=2s&ab_channel=FrankHan)

[Final Presentation](https://docs.google.com/presentation/d/1JwBw1Ba6oGBfa2be1LEtnXdPn5XN4cSdRIS5KA9OF58/edit#slide=id.g23b65494bde_137_263)

![image](https://user-images.githubusercontent.com/108838237/235533905-75917550-a16a-4b74-bf5c-5350cc3a994e.png)

The project development was split into 4 main milestones:

## Milestone 1: 
Loaded pre-processed Open Street Map (OSM) data containing cities' geographic features into appropriate data structures. Wrote helper functions to set up the StreetMapLibrary

## Milestone 2:
Created a Graphical User Interface using GTK. Displayed all geographic elements of the map and incorporate additional features with OSM data (Suwbay lines, Point of Interest Filters, etc.).  
Implemented R-Tree structure for map data to maximize responsiveness.

## Milestone 3:
Implemented A* algorithm to support path-finding feature to the map, along with corresponding UI elements. Guaranteed less than 0.5 seconds response time.

## Milestone 4:
Attempted to solve the NP-Hard Travelling Courier Problem: find a sub-optimal path for picking-up and dropping off multiple delivery packages throughout the city. Derived a 90\% perfect solution.
