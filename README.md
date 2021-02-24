# SFND 2D Feature Tracking

<img src="images/keypoints.png" width="820" height="248" />

The idea of the camera course is to build a collision detection system - that's the overall goal for the Final Project. As a preparation for this, you will now build the feature tracking part and test various detector / descriptor combinations to see which ones perform best. This mid-term project consists of four parts:

* First, you will focus on loading images, setting up data structures and putting everything into a ring buffer to optimize memory load. 
* Then, you will integrate several keypoint detectors such as HARRIS, FAST, BRISK and SIFT and compare them with regard to number of keypoints and speed. 
* In the next part, you will then focus on descriptor extraction and matching using brute force and also the FLANN approach we discussed in the previous lesson. 
* In the last part, once the code framework is complete, you will test the various algorithms in different combinations and compare them with regard to some performance measures. 

See the classroom instruction and code comments for more details on each of these parts. Once you are finished with this project, the keypoint matching part will be set up and you can proceed to the next lesson, where the focus is on integrating Lidar points and on object detection using deep-learning. 







```
SFND_P2_2D_Feature_Matching
|- src
|  |- dataStructures.h            # define data structure for image/keypoint/descriptor/descriptor match
|  |- matching2D.cpp              # implement funcs for keypoint/descriptor/descriptor match
|  |- matching2D.hpp              
|  |- cameraFeatureTracking.cpp   # Main file for this project
|- images
|- LICENSE
|- README.md
|- CMakeLists.txt
```



### Performance Evaluation 1

In this section, we'll count the number of keypoints on the preceding vehicle for all 10 images and discuss the distribution of their neighborhood size

- Number of keypoints counted over all 10 images : 

|  Detector  | Img 1 | Img 2 | Img 3 | Img 4 | Img 5 | Img 6 | Img 7 | Img 8 | Img 9 | Img 10 |
| :--------: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :----: |
| Shi-Tomasi |  125  |  118  |  123  |  120  |  120  |  113  |  114  |  123  |  111  |  112   |
|   Harris   |  15   |  13   |  16   |  14   |  21   |  32   |  14   |  27   |  21   |   23   |
|    FAST    |  149  |  152  |  150  |  155  |  149  |  149  |  156  |  150  |  138  |  143   |
|   BRISK    |  264  |  282  |  282  |  277  |  297  |  279  |  289  |  272  |  266  |  254   |
|    ORB     |  92   |  102  |  106  |  113  |  109  |  125  |  130  |  129  |  127  |  128   |
|   AKAZE    |  166  |  157  |  161  |  155  |  163  |  164  |  173  |  175  |  177  |  179   |
|    SIFT    |  138  |  132  |  124  |  137  |  134  |  140  |  137  |  148  |  159  |  137   |



**Shi-Tomasi**

- A lot of sparse keypoints spread are detected not only in vehicle region but tree region and bridge region

![SHITOMASI](./images/SHITOMASI.png)

**Harris**

- Compared to Shi-Tomasi, few sparse keypoints are detected and it seems that only very strong keypoints are detected
- The sparsity may result from non-maximum suppresion (NMS). It could be further tweaked in NMS setting

![HARRIS](./images/HARRIS.png)

**FAST**

- Compared to Shi-Tomasi and Harris, much more keypoints are detected in tree and bridge region
- The parameters can be further tweaked instead of default FAST setting

![FAST](./images/FAST.png)

**BRISK**

- BRISK has a good spread of distributed keypoints with different sizes 
- However, there're too many noisy keypoints with large sizes and lots of them spread on bridge region and tree region

![BRISK](./images/BRISK.png)

**ORB**

- ORB generates keypoints mostly on vehicle region and road region
- But the keypoint neighborhood size are quite large and similar

![ORB](./images/ORB.png)

**AKAZE**

- AKAZE generates a good spread of distributed keypoints with different sizes like BRISK
- However, it has smaller neighborhood size compared to BRISK

![AKAZE](./images/AKAZE.png)

**SIFT**

- SIFT also has a good spread of distributed keypoints with different sizes like BRISK and AKAZE
- Besides, it has a good spread of neighborhood sizes 

![SIFT](./images/SIFT.png)

### Performance Evaluation 2

- In this section, we'll count the number of matched keypoints for all 10 images using all possible combinations of detectors and descriptors. In the matching step, the BF approach is used with the descriptor distance ratio set to 0.8.



| Detector\Descriptor | **BRISK** | **BRIEF** | **ORB** | **FREAK** | **AKAZE** | **SIFT** |
| ------------------- | --------- | --------- | ------- | --------- | --------- | -------- |
| **Shi-Tomasi**      |           |           |         |           | n.a.      |          |
| **Harris**          |           |           |         |           | n.a.      |          |
| **FAST**            |           |           |         |           | n.a.      |          |
| **BRISK**           |           |           |         |           | n.a.      |          |
| **ORB**             |           |           |         |           | n.a.      |          |
| **AKAZE**           | n.a.      | n.a.      | n.a.    | n.a.      |           | n.a.     |
| **SIFT**            |           |           | n.a.    |           | n.a.      |          |



### Performance Evaluation 3

- In this section, we'll log the time it takes for keypoint detection and descriptor extraction and propose the TOP3 detector / descriptor combinations must be recommended as the best choice for our purpose of detecting keypoints on vehicles.











## Dependencies for Running Locally

* cmake >= 2.8
  * All OSes: [click here for installation instructions](https://cmake.org/install/)
* make >= 4.1 (Linux, Mac), 3.81 (Windows)
  * Linux: make is installed by default on most Linux distros
  * Mac: [install Xcode command line tools to get make](https://developer.apple.com/xcode/features/)
  * Windows: [Click here for installation instructions](http://gnuwin32.sourceforge.net/packages/make.htm)
* OpenCV >= 4.1
  * This must be compiled from source using the `-D OPENCV_ENABLE_NONFREE=ON` cmake flag for testing the SIFT and SURF detectors.
  * The OpenCV 4.1.0 source code can be found [here](https://github.com/opencv/opencv/tree/4.1.0)
* gcc/g++ >= 5.4
  * Linux: gcc / g++ is installed by default on most Linux distros
  * Mac: same deal as make - [install Xcode command line tools](https://developer.apple.com/xcode/features/)
  * Windows: recommend using [MinGW](http://www.mingw.org/)

## Basic Build Instructions

1. Clone this repo.
2. Make a build directory in the top level directory: `mkdir build && cd build`
3. Compile: `cmake .. && make`
4. Run it: `./2D_feature_tracking`.