# ForestFireAutomata
    
## Overview
This implementation of the Forest Cellular Automata is a variation of forest fire propagation as described in [Drossel&Schwabl92].
Instead of a single kind of tree (calling Combustible), there was used three different kind of coumbustibles:

 * **Light green tree:** burn very quickly.
 * **Green tree:** burn normally.
 * **Dark green tree:** burn very slow.

Moreover, that was introduced the concept of the **wind** for determine the neighbors of a specific cell.
Initially it takes one of the values of the **wind rose** (NW, N, NE, E, SE, S, SW, W) and later, during the whole execution, every 50 loop iteration it takes randomly one of the two neighbors of the current direction wind. 
For instance, if the current direction of the wind is E, the next direction can be either NE or SE.

The neighbors of a cell are exactly three, and are determined according to the wind direction. 
For instance, how it is showed in the following image, if the wind direction is SE, the neighbords of a cell [i][j] are the cell:

<p align="center">
    <img src="https://i.imgur.com/k4D1Zpa.png" width="500" >
</p>
                                                       
Trees catch fire if at least one of their neighbors are on fire or if it is strucked by lightning with probability ***f*** (by default 0.00000002) each generation. According to the kind of combustible described above, burning trees become empty cells in the next generation. Empty cells have a probability ***p*** (default 0.002) to grow a coumbustible each generation.


## Implementation
There are provided both sequential and parallel implementation of the Cellular Automata, and both of them was developed by using the **C++** programming language and the library **Allegro** (https://liballeg.org/) for the visual rendering. In addition, for the parallel implementation it was used the **MPI** library (Message Passing Interface) (https://www.open-mpi.org/) for the communication between process.

There was developed three different versions, in each of which the data was decomposed in a row-fashion way, in order to assign at each process an equal portion of the data.

<p align="center">
    <img src="https://i.imgur.com/HoaX8Jt.png" width="450" >
</p>

The three versions of the parallel implementation differs each other in the way them deals (send/receive) the data. In particular:
 * **Send & Receive All Data ([S_R_All_Data](forest_Fire_S_R_AllData.cpp)):** send and receive all the data, either if it was changed or if isn't.
 * **Send & Receive All Data Fixed Size ([S_R_All_Data_Fixed_Struct_Size](forest_Fire_S_R_AllChangedData_FixedSizeStruct.cpp)):** send/receive all the data to/from neighbords processes (first row to the process on the left and last row to the process on the right). So, the at each iteration it was sended/received an amount of data equals to BLOCKCOLS.
 * **Send & Receive Only Changed Data  ([S_R_All_Changed_Data](forest_Fire_S_R_AllChangedData.cpp)):** send/receive to/from neighbords processes only the data that was effectively changed. So, the size of the data sended/receive is dinamically determined.
 
In each of the above version the processes works according to a Master/Slave architecture, by using two different communicator.
In particular:
* **Master process**: It has to communicate with each workers in order to retrieve the data (thanks to the messagge passing paradigm) and update the current generation and render it (Is the only process enabled to render).
* **Slave Process**: They are distributed according to a cartesian topology (Linear) of dimensions **Nproc (-1) * 1** toroidal along the first dimension. In this topology the master is not included. This choice was done beacuse according to the Task Dependency Graph we can see that each process, in order to compute his portion of data (Row Division described above) need some data from both left and right process.

For instance if one of the implementation is executed with 5 process we have the following Task Dependency Graphs:

**First One:**
<p align="center">
    <img src="https://i.imgur.com/1Yen2Ke.png" width="500" >
</p>

**Second One:**
<p align="center">
    <img src="https://i.imgur.com/HKSbSIE.png" width="500" >
</p>


## Results

### Problem Size: 160x160
Sequential Time  | Parallel Time | Speed Up  | Efficency | Number of Processors | Parallel Version | 
------------- | ------------- | ------------- | ------------- | ------------- | -------------
4,09 s  | 2,57129 s | 1,5906 | 0,5302 | 3 | S_R_All_Data  
4,09 s  | 2,81024 s | 1,4554 | 0,4851 | 3 | S_R_All_Data_Fixed_Struct_Size  
4,09 s  | 2,3867 s | 1,7136 | 0,5712 | 3 | S_R_All_Changed_Data     
4,09 s  | 1,63184 s | 2,5063 | 0,5012 | 5 | S_R_All_Data
4,09 s  | 2,1818 s | 1,8746 | 0,3749 | 5 | S_R_All_Data_Fixed_Struct_Size  
4,09 s  | 1,8528 s | 2,2075 | 0,4414 | 5 | S_R_All_Changed_Data     
4,09 s  | 1,19141 s |  3,4329 | 0,4904 | 7 | S_R_All_Data
4,09 s  | 1,5257 s |  2,6807 | 0,383 | 7 | S_R_All_Data_Fixed_Struct_Size  
4,09 s  | 1,2814 s |  3,1918 | 0,456 | 7 | S_R_All_Changed_Data     

### Problem Size: 320x320
Sequential Time  | Parallel Time | Speed Up  | Efficency | Number of Processors | Parallel Version | 
------------- | ------------- | ------------- | ------------- | ------------- | -------------
16,22 s  | 9,2583 s |  1,7519 | 0,584 | 3 | S_R_All_Data  
16,22 s  | 9,8861 s |  1,6407 | 0,5469 | 3 | S_R_All_Data_Fixed_Struct_Size    
16,22 s  | 9,2356 s |  1,7562 | 0,5854 | 3 | S_R_All_Changed_Data   
16,22 s  | 6,9545 s |  2,3323 | 0,4665 | 5 | S_R_All_Data
16,22 s  | 7,4869 s |  2,1665| 0,4333 | 5 | S_R_All_Data_Fixed_Struct_Size    
16,22 s  | 7,1278 s |  2,2756 | 0,4551 | 5 | S_R_All_Changed_Data   
16,22 s  | 4,92432 s |  3,2939 | 0,4706 | 7 | S_R_All_Data
16,22 s  | 5,8233 s |  2,7854 | 0,3979 | 7 | S_R_All_Data_Fixed_Struct_Size    
16,22 s  | 4,9675 s |  3,2652 | 0,4665 | 7 | S_R_All_Changed_Data   

### Problem Size: 640x640
Sequential Time  | Parallel Time | Speed Up  | Efficency | Number of Processors | Parallel Version | 
------------- | ------------- | ------------- | ------------- | ------------- | -------------
62,705 s  | 34,7373 s |  1,8051 | 0,6017 | 3 | S_R_All_Data  
62,705 s  | Not Executable | -  | - | 3 | S_R_All_Data_Fixed_Struct_Size   
62,705 s  | 35,2331 s |  1,7797 | 0,5932 | 3 | S_R_All_Changed_Data   
62,705 s  | 28,2646 s |  2,2185 | 0,4437 | 5 | S_R_All_Data
62,705 s  | Not Executable |  - | - | 5 | S_R_All_Data_Fixed_Struct_Size    
62,705 s  | 29,2556 s |  2,1434 | 0,4287 | 5 | S_R_All_Changed_Data   
62,705 s  | 19,3945 s |  3,2331 | 0,4619 | 7 | S_R_All_Data
62,705 s  | Not Executable |  - | - | 7 | S_R_All_Data_Fixed_Struct_Size    
62,705 s  | 19,5446 s |  3,20 | 0,4583 | 7 | S_R_All_Changed_Data   

### Problem Size: 1280x1280
Sequential Time  | Parallel Time | Speed Up  | Efficency | Number of Processors | Parallel Version | 
------------- | ------------- | ------------- | ------------- | ------------- | -------------
242,061 s  | Not Executable |  - | - | 3 | S_R_All_Data  
242,061 s  | Not Executable |  - | - | 3 | S_R_All_Data_Fixed_Struct_Size    
242,061 s  | 137,557 s |  1,76 | 0,59 | 3 | S_R_All_Changed_Data  
242,061 s  | Not Executable |  - | - | 5 | S_R_All_Data
242,061 s  | Not Executable |  - | - | 5 | S_R_All_Data_Fixed_Struct_Size    
242,061 s  | 110,22 s |  2,22 | s | 0,44 | S_R_All_Changed_Data    
242,061 s  | Not Executable |  - | - | 7 | S_R_All_Data
242,061 s  | Not Executable |  - | - | 7 | S_R_All_Data_Fixed_Struct_Size    
242,061 s  | 76,77 s |  3,15 | s | 0,45 | S_R_All_Changed_Data  


## Conclusions
We note that none of the obtained results give us a slowdown, in any case we obtain a parallelization of the problem.
In particular we can conclude that for a little problem size (160x160) or (320x320) there are no substantial differences on use one of the parallel method w.r.t another one. We note that only the second method (Send and Receive of Array of struct equal to BLOCKCOLS or BLOCKROWS*BLOCKCOLS) is always behind the other two.

For problem whose size increase (640x640) we note that the second method is not executable, this beacuse the amount of data to Send and Receive at each iteration for each process is very huge (For instance, suppose a matrix (640x640), and the program run with 3 process, the master process receive at each iteration (640*320)*sizeof(struct) data for each process, and each process send the same amount of data to the master, and send (640)*sizeof(struct) data to each of its neighbourd.

While for problem whose size increase (1280x1280) we note that our idea of send and receive only the data that at each iteration dinamically change is useful, giving us the opportunity of parallelize the programme. In fact the other two method are not executable, always for the problem of the amount of data Send and Received.

We can conclude that in general for little problem size the first or the third method must to be used. (The second one is never the best), while for bigger forest size the third method is the only one usable for the parallelization.


## Bibliography
[Drossel&Schwabl92] B. Drossel and F. Schwabl, “Self-organized critical forest-fire model”, Physical Review Letters, Vol. 69, No. 11, September 1992, pp. 1629–1632.
