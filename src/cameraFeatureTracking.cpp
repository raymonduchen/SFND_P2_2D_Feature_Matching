/* INCLUDES FOR THIS PROJECT */
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <limits>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/xfeatures2d/nonfree.hpp>

#include "dataStructures.h"
#include "matching2D.hpp"

using namespace std;

/* MAIN PROGRAM */
// argv[0] : program name
// argv[1] : detectorType - SHITOMASI, HARRIS, FAST, BRISK, ORB, AKAZE, SIFT  (default : SHITOMASI)
// argv[2] : descriptorType - BRISK, BRIEF, ORB, FREAK, AKAZE, SIFT  (default : BRISK)
// argv[3] : matcherType - MAT_BF, MAT_FLANN  (default : MAT_BF)
// argv[4] : selectorType - SEL_NN, SEL_KNN  (default : SEL_NN)
// argv[5] : bVis - visualize result  (default : 1)
// argv[6] : bLimitKpts - limit keypoints below 50 points  (default : 0)
// argv[7] : bFocusOnVehicle - only keep results within preceding vehicle region  (default : 1)
int main(int argc, const char *argv[])
{
    /* INIT VARIABLES AND DATA STRUCTURES */
    string detectorType = "SHITOMASI";  // SHITOMASI, HARRIS, FAST, BRISK, ORB, AKAZE, SIFT
    string descriptorType = "BRISK"; // BRISK, BRIEF, ORB, FREAK, AKAZE, SIFT
    string matcherType = "MAT_BF";        // MAT_BF, MAT_FLANN
    string selectorType = "SEL_NN";       // SEL_NN, SEL_KNN
    bool bVis = true;
    bool bLimitKpts = false;
    bool bFocusOnVehicle = true;

    if (argc >= 2){
        if (isDetector(string(argv[1]))){
           detectorType = string(argv[1]);
        }
        else{
           std::cout << "\nInvalid Detector Type! Use SHITOMASI detector";
        }
    }

    if (argc >= 3){
        if (isDescriptor(string(argv[2]))){
           descriptorType = string(argv[2]);
        }
        else{
           std::cout << "\nInvalid Descriptor Type! Use BRISK descriptor";
        }
    }

    if (argc >= 4){
        if (isMatcher(string(argv[3]))){
           matcherType = string(argv[3]);
        }
        else{
           std::cout << "\nInvalid Matcher Type! Use MAT_BF matcher";
        }
    }

    if (argc >= 5){
        if (isSelector(string(argv[4]))){
           selectorType = string(argv[4]);
        }
        else{
           std::cout << "\nInvalid Selector Type! Use SEL_NN selector";
        }
    }
    
    if (argc >= 6){
        if (stoi(string(argv[5])) == stoi(string("1"))){
            bVis = true;
        }
        else{
            bVis = false;
        }    
    }
    
    if (argc >= 7){
        if (stoi(string(argv[6])) == stoi(string("1"))){
            bLimitKpts = true;
        }
        else{
            bLimitKpts = false;
        }
    }
        
    if (argc >= 8){
        if (stoi(string(argv[7])) == stoi(string("1"))){
            bFocusOnVehicle = true;
        }
        else{
            bFocusOnVehicle = false;
        }
    }
    
    if ((detectorType == "AKAZE" || descriptorType =="AKAZE") && (detectorType != descriptorType)){ 
        if (detectorType == "AKAZE")
            cout << endl << "AKAZE detector only support AKAZE descriptor. Use AKAZE as detector and descriptor type" << endl;
        if (descriptorType == "AKAZE")
            cout << endl << "AKAZE descriptor only support AKAZE detector. Use AKAZE as detector and descriptor type" << endl;

        detectorType = "AKAZE";
        descriptorType = "AKAZE";
    }
      
        
    cout << endl << "========== SETTING ==========" << endl
         << "detectorType : "<< detectorType << endl
         << "descriptorType : "<< descriptorType << endl
         << "matcherType : "<< matcherType << endl
         << "selectorType : "<< selectorType << endl
         << "bVis : "<< bVis << endl
         << "bLimitKpts : "<< bLimitKpts << endl
         << "bFocusOnVehicle : "<< bFocusOnVehicle << endl
         << "=============================" << endl;

    // data location
    string dataPath = "../";

    // camera
    string imgBasePath = dataPath + "images/";
    string imgPrefix = "KITTI/2011_09_26/image_00/data/000000"; // left camera, color
    string imgFileType = ".png";
    int imgStartIndex = 0; // first file index to load (assumes Lidar and camera names have identical naming convention)
    int imgEndIndex = 9;   // last file index to load
    int imgFillWidth = 4;  // no. of digits which make up the file index (e.g. img-0001.png)

    // misc
    int dataBufferSize = 2;       // no. of images which are held in memory (ring buffer) at the same time
    vector<DataFrame> dataBuffer; // list of data frames which are held in memory at the same time

    /* MAIN LOOP OVER ALL IMAGES */

    for (size_t imgIndex = 0; imgIndex <= imgEndIndex - imgStartIndex; imgIndex++)
    {
        /* LOAD IMAGE INTO BUFFER */

        // assemble filenames for current index
        ostringstream imgNumber;
        imgNumber << setfill('0') << setw(imgFillWidth) << imgStartIndex + imgIndex;
        string imgFullFilename = imgBasePath + imgPrefix + imgNumber.str() + imgFileType;

        cout << endl << "Processing " << imgFullFilename << endl;

        // load image from file and convert to grayscale
        cv::Mat img, imgGray;
        img = cv::imread(imgFullFilename);
        cv::cvtColor(img, imgGray, cv::COLOR_BGR2GRAY);

        //// STUDENT ASSIGNMENT
        //// TASK MP.1 -> replace the following code with ring buffer of size dataBufferSize
        addFrameToBuffer(dataBuffer, imgGray, dataBufferSize);

        //// EOF STUDENT ASSIGNMENT
        cout << "#1 : LOAD IMAGE INTO BUFFER done" << endl;

        /* DETECT IMAGE KEYPOINTS */

        // extract 2D keypoints from current image
        vector<cv::KeyPoint> keypoints; // create empty feature list for current image

        //// TASK MP.2 -> add the following keypoint detectors in file matching2D.cpp and enable string-based selection based on detectorType
        detKeypointsModern(keypoints, imgGray, detectorType, bVis);

        //// TASK MP.3 -> only keep keypoints on the preceding vehicle
        // only keep keypoints on the preceding vehicle
        if (bFocusOnVehicle){
            cv::Rect roi = cv::Rect(535, 180, 180, 150);
            roiKeypoints(keypoints, roi);
        }
            

        // optional : limit number of keypoints (helpful for debugging and learning)
        if (bLimitKpts){
            int maxKeypoints = 50;
            limitKeypoints(keypoints, detectorType, maxKeypoints);
        }

        // push keypoints and descriptor for current frame to end of data buffer
        (dataBuffer.end() - 1)->keypoints = keypoints;
        cout << "#2 : DETECT KEYPOINTS done" << endl;

        /* EXTRACT KEYPOINT DESCRIPTORS */

        //// STUDENT ASSIGNMENT
        //// TASK MP.4 -> add the following descriptors in file matching2D.cpp and enable string-based selection based on descriptorType
        cv::Mat descriptors;
        descKeypoints((dataBuffer.end() - 1)->keypoints, (dataBuffer.end() - 1)->cameraImg, descriptors, descriptorType);

        // push descriptors for current frame to end of data buffer
        (dataBuffer.end() - 1)->descriptors = descriptors;

        cout << "#3 : EXTRACT DESCRIPTORS done" << endl;

        if (dataBuffer.size() > 1) // wait until at least two images have been processed
        {
            /* MATCH KEYPOINT DESCRIPTORS */
            vector<cv::DMatch> matches;
            //// STUDENT ASSIGNMENT
            //// TASK MP.5 -> add FLANN matching in file matching2D.cpp
            //// TASK MP.6 -> add KNN match selection and perform descriptor distance ratio filtering with t=0.8 in file matching2D.cpp

            matchDescriptors((dataBuffer.end() - 2)->keypoints, (dataBuffer.end() - 1)->keypoints,
                             (dataBuffer.end() - 2)->descriptors, (dataBuffer.end() - 1)->descriptors,
                             matches, descriptorType, matcherType, selectorType);

            // store matches in current data frame
            (dataBuffer.end() - 1)->kptMatches = matches;

            cout << "#4 : MATCH KEYPOINT DESCRIPTORS done" << endl;

            // visualize matches between current and previous image
            if (bVis)
            {
                cv::Mat matchImg = ((dataBuffer.end() - 1)->cameraImg).clone();
                cv::drawMatches((dataBuffer.end() - 2)->cameraImg, (dataBuffer.end() - 2)->keypoints,
                                (dataBuffer.end() - 1)->cameraImg, (dataBuffer.end() - 1)->keypoints,
                                matches, matchImg,
                                cv::Scalar::all(-1), cv::Scalar::all(-1),
                                vector<char>(), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

                string windowName = "Matching keypoints between successive camera images";
                cv::namedWindow(windowName, 7);
                cv::imshow(windowName, matchImg);
                cout << "Press key to continue to next image" << endl;
                cv::waitKey(0); // wait for key to be pressed
            }

            cout << endl << "-----------------------------" << endl;
        }
    } // eof loop over all images

    return 0;
}
