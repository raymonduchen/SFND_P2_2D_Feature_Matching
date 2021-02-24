#include <numeric>
#include "matching2D.hpp"

using namespace std;

void addFrameToBuffer(std::vector<DataFrame> &dataBuffer, cv::Mat &img, int dataBufferSize)
{
    double t = (double)cv::getTickCount();

    DataFrame frame;
    frame.cameraImg = img;

    if (dataBuffer.size() == dataBufferSize){
        dataBuffer.erase(dataBuffer.begin());
    }

    dataBuffer.push_back(frame);

    t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
    cout << "Load frame to buffer in " << 1000 * t / 1.0 << " ms" << endl;
}

// Find best matches for keypoints in two camera images based on several matching methods
void matchDescriptors(std::vector<cv::KeyPoint> &kPtsSource, std::vector<cv::KeyPoint> &kPtsRef, cv::Mat &descSource, cv::Mat &descRef,
                      std::vector<cv::DMatch> &matches, std::string descriptorType, std::string matcherType, std::string selectorType)
{
    double t = (double)cv::getTickCount();

    // configure matcher
    bool crossCheck = false;
    cv::Ptr<cv::DescriptorMatcher> matcher;
    int normType;

    if (descriptorType == "SIFT"){
        // DES_HOG
        normType = cv::NORM_L2;
    }
    else if (descriptorType == "BRISK" || descriptorType == "BRIEF" || descriptorType == "ORB" ||
             descriptorType == "FREAK" || descriptorType == "AKAZE" || descriptorType == "SIFT"){
        // DES_BINARY
        normType = cv::NORM_HAMMING;
    }


    if (matcherType.compare("MAT_BF") == 0)
    {
        matcher = cv::BFMatcher::create(normType, crossCheck);
    }
    else if (matcherType.compare("MAT_FLANN") == 0)
    {
        if (descSource.type() != CV_32F)
        { // Convert binary descriptors to floating point due to a bug in current OpenCV implementation
            descSource.convertTo(descSource, CV_32F);
            descRef.convertTo(descRef, CV_32F);
        }

        matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::FLANNBASED);
    }

    // perform matching task
    if (selectorType.compare("SEL_NN") == 0)
    { // nearest neighbor (best match)

        matcher->match(descSource, descRef, matches); // Finds the best match for each descriptor in desc1
    }
    else if (selectorType.compare("SEL_KNN") == 0)
    { // k nearest neighbors (k=2)
        std::vector<std::vector<cv::DMatch>> knn_matches;
        matcher->knnMatch(descSource, descRef, knn_matches, 2);

        for (int i = 0; i < knn_matches.size(); i++) {
            const float dist1 = knn_matches[i][0].distance;
            const float dist2 = knn_matches[i][1].distance;
            if ((dist1 / dist2) < 0.8) {
              // Push the best match between the two matches if we are lower than the threshold
              matches.push_back(knn_matches[i][0]);
            }
        }
        
    }
    
    t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
    cout << matcherType << " matcher matched n="<< matches.size() << " pairs in " << 1000 * t / 1.0 << " ms" << endl;
}

// Use one of several types of state-of-art descriptors to uniquely identify keypoints
void descKeypoints(vector<cv::KeyPoint> &keypoints, cv::Mat &img, cv::Mat &descriptors, string descriptorType)
{
    // select appropriate descriptor
    cv::Ptr<cv::DescriptorExtractor> extractor;
    if (descriptorType.compare("BRISK") == 0)
    {
        /*
        int threshold = 30;        // FAST/AGAST detection threshold score.
        int octaves = 3;           // detection octaves (use 0 to do single scale)
        float patternScale = 1.0f; // apply this scale to the pattern used for sampling the neighbourhood of a keypoint.

        extractor = cv::BRISK::create(threshold, octaves, patternScale);
        */
        extractor = cv::BRISK::create();
    }
    else if (descriptorType.compare("BRIEF") == 0)
    {
        /*
        int bytes = 32;
        bool use_orientation = false;
        
        extractor = cv::xfeatures2d::BriefDescriptorExtractor::create(bytes, use_orientation);
        */
        extractor = cv::xfeatures2d::BriefDescriptorExtractor::create();
    }
    else if (descriptorType.compare("ORB") == 0)
    {
        /*
        int nfeatures = 500;
        float scaleFactor = 1.2f;
        int nlevels = 8;
        int edgeThreshold = 31;
        int firstLevel = 0;
        int WTA_K = 2;
        cv::ORB::ScoreType scoreType = cv::ORB::HARRIS_SCORE;
        int patchSize = 31;
        int fastThreshold = 20;

        extractor = cv::ORB::create(nfeatures, scaleFactor, nlevels, edgeThreshold, firstLevel,
                                  WTA_K, scoreType, patchSize, fastThreshold);
        */
        extractor = cv::ORB::create();
    }
    else if (descriptorType.compare("FREAK") == 0)
    {
        /*
        bool orientationNormalized = true;
        bool scaleNormalized = true;
        float patternScale = 22.0f;
        int nOctaves = 4;
        const std::vector< int > &  selectedPairs = std::vector< int >();
   
        extractor = cv::xfeatures2d::FREAK::create(orientationNormalized, scaleNormalized, patternScale, 
                                                   nOctaves, selectedPairs);
        */
        extractor = cv::xfeatures2d::FREAK::create();
    }
    else if (descriptorType.compare("AKAZE") == 0)
    {
        /*
        cv::AKAZE::DescriptorType descriptor_type = cv::AKAZE::DESCRIPTOR_MLDB;
        int descriptor_size = 0;
        int descriptor_channels = 3;
        float threshold = 0.001f;
        int nOctaves = 4;
        int nOctaveLayers = 4;
        cv::KAZE::DiffusivityType diffusivity = cv::KAZE::DIFF_PM_G2;

        extractor = cv::AKAZE::create(descriptor_type, descriptor_size, descriptor_channels,
                                      threshold, nOctaves, nOctaveLayers, diffusivity);
        */
        extractor = cv::AKAZE::create();                              

    }
    else if (descriptorType.compare("SIFT") == 0)
    {
        /*
        int nfeatures = 0;
        int nOctaveLayers = 3;
        double contrastThreshold = 0.04;
        double edgeThreshold = 10;
        double sigma = 1.6;

        extractor = cv::SIFT::create(nfeatures, nOctaveLayers, contrastThreshold, edgeThreshold, sigma);
        //extractor = cv::xfeatures2d::SIFT::create(nfeatures, nOctaveLayers, contrastThreshold, edgeThreshold, sigma);
        */

        extractor = cv::SIFT::create();
    }

    // perform feature description
    double t = (double)cv::getTickCount();
    extractor->compute(img, keypoints, descriptors);
    t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
    cout << descriptorType << " descriptor extraction in " << 1000 * t / 1.0 << " ms" << endl;

}

void detKeypointsModern(std::vector<cv::KeyPoint> &keypoints, cv::Mat &img, std::string detectorType, bool bVis)
{
    if (detectorType.compare("SHITOMASI") == 0)
    {
        detKeypointsShiTomasi(keypoints, img, bVis);
    }
    else if (detectorType.compare("HARRIS") == 0)
    {
        detKeypointsHarris(keypoints, img, bVis);
    }
    else if (detectorType.compare("FAST") == 0)
    {
        detKeypointsFast(keypoints, img, bVis);
    }
    else if (detectorType.compare("BRISK") == 0)
    {
        detKeypointsBrisk(keypoints, img, bVis);
    }
    else if (detectorType.compare("ORB") == 0)
    {
        detKeypointsOrb(keypoints, img, bVis);
    }
    else if (detectorType.compare("AKAZE") == 0)
    {
        detKeypointsAkaze(keypoints, img, bVis);
    }
    else if (detectorType.compare("SIFT") == 0)
    {
        detKeypointsSift(keypoints, img, bVis);
    }
}

void roiKeypoints(std::vector<cv::KeyPoint> &keypoints, cv::Rect roi)
{
    keypoints.erase(std::remove_if(keypoints.begin(), keypoints.end(), 
              [roi](cv::KeyPoint point) {return !roi.contains(point.pt);}), 
              keypoints.end());

    cout << "Region of interest has n=" << keypoints.size() << " keypoints" << endl;

}

void limitKeypoints(std::vector<cv::KeyPoint> &keypoints, std::string detectorType, int maxKeypoints)
{
    // NOTE ONLY SHITOMASI HAS BEEN IMPLEMENTED LIMITKEYPOINTS
    if (detectorType.compare("SHITOMASI") == 0)
    { // there is no response info, so keep the first 50 as they are sorted in descending quality order
        keypoints.erase(keypoints.begin() + maxKeypoints, keypoints.end());
        cv::KeyPointsFilter::retainBest(keypoints, maxKeypoints);
        cout << "Keypoints have been limited to n=" << keypoints.size() << " keypoints" << endl;
    }
    else
    {
        cout << "Not support keypoints limiting for the detector" << endl;
    }

    
}


// Detect keypoints in image using the traditional Shi-Thomasi detector
void detKeypointsShiTomasi(vector<cv::KeyPoint> &keypoints, cv::Mat &img, bool bVis)
{
    double t = (double)cv::getTickCount();
    // compute detector parameters based on image size
    int blockSize = 4;       //  size of an average block for computing a derivative covariation matrix over each pixel neighborhood
    double maxOverlap = 0.0; // max. permissible overlap between two features in %
    double minDistance = (1.0 - maxOverlap) * blockSize;
    int maxCorners = img.rows * img.cols / max(1.0, minDistance); // max. num. of keypoints

    double qualityLevel = 0.01; // minimal accepted quality of image corners
    double k = 0.04;

    // Apply corner detection
    vector<cv::Point2f> corners;
    cv::goodFeaturesToTrack(img, corners, maxCorners, qualityLevel, minDistance, cv::Mat(), blockSize, false, k);

    // add corners to result vector
    for (auto it = corners.begin(); it != corners.end(); ++it)
    {

        cv::KeyPoint newKeyPoint;
        newKeyPoint.pt = cv::Point2f((*it).x, (*it).y);
        newKeyPoint.size = blockSize;
        keypoints.push_back(newKeyPoint);
    }
    t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
    cout << "SHITOMASI detection with n=" << keypoints.size() << " keypoints in " << 1000 * t / 1.0 << " ms" << endl;

    // visualize results
    if (bVis)
    {
        cv::Mat visImage = img.clone();
        cv::drawKeypoints(img, keypoints, visImage, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        string windowName = "SHITOMASI Corner Detector Results";
        cv::namedWindow(windowName, 6);
        imshow(windowName, visImage);
        //cout << "Press key to continue" << endl;
        //cv::waitKey(0);
    }
}


// Detect keypoints in image using the Harris detector
void detKeypointsHarris(std::vector<cv::KeyPoint> &keypoints, cv::Mat &img, bool bVis)
{
    double t = (double)cv::getTickCount();
    int blockSize = 2;     // for every pixel, a blockSize × blockSize neighborhood is considered
    int apertureSize = 3;  // aperture parameter for Sobel operator (must be odd)
    int minResponse = 100; // minimum value for a corner in the 8bit scaled response matrix
    double k = 0.04;       // Harris parameter (see equation for details)

    // Apply corner detection    
    // Detect Harris corners and normalize output
    cv::Mat dst, dst_norm, dst_norm_scaled;
    dst = cv::Mat::zeros(img.size(), CV_32FC1);
    cv::cornerHarris(img, dst, blockSize, apertureSize, k, cv::BORDER_DEFAULT);
    cv::normalize(dst, dst_norm, 0, 255, cv::NORM_MINMAX, CV_32FC1); //, cv::Mat());
    cv::convertScaleAbs(dst_norm, dst_norm_scaled);


    // Locate local maxima in the Harris response matrix and perform non-maximum suppression (NMS) 
    // in a local neighborhood around each maximum. The resulting coordinates shall be stored in a 
    // list of keypoints of the type `vector<cv::KeyPoint>`.
    const int nms_window_size = 2 * apertureSize + 1; // 7 x 7
    const int rows = dst_norm.rows;
    const int cols = dst_norm.cols;

    // Store the resulting points in a vector of cv::KeyPoints
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int centre_r = -1;
            int centre_c = -1;

            // The max value is set to the minimum response
            // We should have keypoints that exceed this first
            unsigned char max_val = static_cast<unsigned char>(minResponse);
            for (int x = -nms_window_size; x <= nms_window_size; x++) {
                for (int y = -nms_window_size; y <= nms_window_size; y++) {
                    if ((i + x) < 0 || (i + x) >= rows) {
                        continue;
                    }
                    if ((j + y) < 0 || (j + y) >= cols) {
                        continue;
                    }
                    const unsigned char val = dst_norm_scaled.at<unsigned char>(i + x, j + y);
                    if (val > max_val) {
                        max_val = val;
                        centre_r = i + x;
                        centre_c = j + y;
                    }
                }
            }

            // If the largest value was at the centre, remember this keypoint
            if (centre_r == i && centre_c == j) {
                keypoints.emplace_back(j, i, 2 * apertureSize, -1, max_val);
            }
        }
    }

    t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
    cout << "HARRIS detection with n=" << keypoints.size() << " keypoints in " << 1000 * t / 1.0 << " ms" << endl;

    // visualize results
    if (bVis)
    {
        cv::Mat visImage = img.clone();
        cv::drawKeypoints(img, keypoints, visImage, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        string windowName = "HARRIS Detector Results";
        cv::namedWindow(windowName, 6);
        imshow(windowName, visImage);
        //cout << "Press key to continue" << endl;
        //cv::waitKey(0);
    }
}

// Detect keypoints in image using the Fast detector
void detKeypointsFast(std::vector<cv::KeyPoint> &keypoints, cv::Mat &img, bool bVis)
{
    double t = (double)cv::getTickCount();
    
    int threshold = 30;
    bool nms = true;
    cv::FastFeatureDetector::DetectorType type = cv::FastFeatureDetector::TYPE_9_16;  //TYPE_7_12, TYPE_5_8

    // Apply corner detection
    cv::Ptr<cv::FeatureDetector> detector = cv::FastFeatureDetector::create(threshold, nms, type);
    //cv::Ptr<cv::FeatureDetector> detector = cv::FastFeatureDetector::create();

    detector->detect(img, keypoints);
    t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
    cout << "FAST detection with n=" << keypoints.size() << " keypoints in " << 1000 * t / 1.0 << " ms" << endl;

    // visualize results
    if (bVis)
    {
        cv::Mat visImage = img.clone();
        cv::drawKeypoints(img, keypoints, visImage, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        string windowName = "FAST Detector Results";
        cv::namedWindow(windowName, 6);
        imshow(windowName, visImage);
        //cout << "Press key to continue" << endl;
        //cv::waitKey(0);
    }
}

// Detect keypoints in image using the Brisk detector
void detKeypointsBrisk(std::vector<cv::KeyPoint> &keypoints, cv::Mat &img, bool bVis)
{
    double t = (double)cv::getTickCount();
    /*
    int threshold = 30;
    int octaves = 3;
    float patternScale = 1.0f;

    //cv::Ptr<cv::FeatureDetector> detector = cv::BRISK::create(threshold, octaves, patternScale);
    */

    // Apply corner detection
    cv::Ptr<cv::FeatureDetector> detector = cv::BRISK::create();

    detector->detect(img, keypoints);
    t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
    cout << "BRISK detection with n=" << keypoints.size() << " keypoints in " << 1000 * t / 1.0 << " ms" << endl;

    // visualize results
    if (bVis)
    {
        cv::Mat visImage = img.clone();
        cv::drawKeypoints(img, keypoints, visImage, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        string windowName = "BRISK Detector Results";
        cv::namedWindow(windowName, 6);
        imshow(windowName, visImage);
        //cout << "Press key to continue" << endl;
        //cv::waitKey(0);
    }
}


// Detect keypoints in image using the Orb detector
void detKeypointsOrb(std::vector<cv::KeyPoint> &keypoints, cv::Mat &img, bool bVis)
{
    double t = (double)cv::getTickCount();
    /*
    int nfeatures = 500;
    float scaleFactor = 1.2f;
    int nlevels = 8;
    int edgeThreshold = 31;
    int firstLevel = 0;
    int WTA_K = 2;
    cv::ORB::ScoreType scoreType = cv::ORB::HARRIS_SCORE;
    int patchSize = 31;
    int fastThreshold = 20;
    
    cv::Ptr<cv::FeatureDetector> detector = cv::ORB::create(nfeatures, scaleFactor, nlevels, edgeThreshold, 
                                                            firstLevel, WTA_K, scoreType, patchSize, fastThreshold);
    */

    // Apply corner detection
    cv::Ptr<cv::FeatureDetector> detector = cv::ORB::create();
    detector->detect(img, keypoints);
    t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
    cout << "ORB detection with n=" << keypoints.size() << " keypoints in " << 1000 * t / 1.0 << " ms" << endl;

    // visualize results
    if (bVis)
    {
        cv::Mat visImage = img.clone();
        cv::drawKeypoints(img, keypoints, visImage, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        string windowName = "ORB Detector Results";
        cv::namedWindow(windowName, 6);
        imshow(windowName, visImage);
        //cout << "Press key to continue" << endl;
        //cv::waitKey(0);
    }
}


// Detect keypoints in image using the Akaze detector
void detKeypointsAkaze(std::vector<cv::KeyPoint> &keypoints, cv::Mat &img, bool bVis)
{
    double t = (double)cv::getTickCount();
    /*
    cv::AKAZE::DescriptorType descriptor_type = cv::AKAZE::DESCRIPTOR_MLDB;
    int descriptor_size = 0;
    int descriptor_channels = 3;
    float threshold = 0.001f;
    int nOctaves = 4;
    int nOctaveLayers = 4;
    cv::KAZE::DiffusivityType diffusivity = cv::KAZE::DIFF_PM_G2;

    cv::Ptr<cv::FeatureDetector> detector = cv::AKAZE::create(descriptor_type, descriptor_size, descriptor_channels,
                                                            threshold, nOctaves, nOctaveLayers, diffusivity);
    */                       

    // Apply corner detection
    cv::Ptr<cv::FeatureDetector> detector = cv::AKAZE::create();
    detector->detect(img, keypoints);
    t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
    cout << "AKAZE detection with n=" << keypoints.size() << " keypoints in " << 1000 * t / 1.0 << " ms" << endl;

    // visualize results
    if (bVis)
    {
        cv::Mat visImage = img.clone();
        cv::drawKeypoints(img, keypoints, visImage, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        string windowName = "AKAZE Detector Results";
        cv::namedWindow(windowName, 6);
        imshow(windowName, visImage);
        //cout << "Press key to continue" << endl;
        //cv::waitKey(0);
    }
}


// Detect keypoints in image using the Sift detector
void detKeypointsSift(std::vector<cv::KeyPoint> &keypoints, cv::Mat &img, bool bVis)
{
    double t = (double)cv::getTickCount();
    /*
    int nfeatures = 0;
    int nOctaveLayers = 3;
    double contrastThreshold = 0.04;
    double edgeThreshold = 10;
    double sigma = 1.6;

    cv::Ptr<cv::FeatureDetector> detector = cv::SIFT::create(nfeatures, nOctaveLayers, contrastThreshold, edgeThreshold, sigma);
    //cv::Ptr<cv::FeatureDetector> detector = cv::xfeatures2d::SIFT::create(nfeatures, nOctaveLayers, contrastThreshold, edgeThreshold, sigma);
    */

    // Apply corner detection
    cv::Ptr<cv::FeatureDetector> detector = cv::SIFT::create();
    detector->detect(img, keypoints);
    t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
    cout << "SIFT detection with n=" << keypoints.size() << " keypoints in " << 1000 * t / 1.0 << " ms" << endl;

    // visualize results
    if (bVis)
    {
        cv::Mat visImage = img.clone();
        cv::drawKeypoints(img, keypoints, visImage, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        string windowName = "SIFT Detector Results";
        cv::namedWindow(windowName, 6);
        imshow(windowName, visImage);
        //cout << "Press key to continue" << endl;
        //cv::waitKey(0);
    }
}


bool isDetector(std::string detectorType)
{
  if (detectorType == "SHITOMASI" || detectorType == "HARRIS" || detectorType == "FAST" ||
      detectorType == "BRISK" || detectorType == "ORB" || detectorType == "AKAZE" || 
      detectorType == "SIFT") 
      return true;
  else
      return false;
}

bool isDescriptor(std::string descriptorType)
{
  if (descriptorType == "BRISK" || descriptorType == "BRIEF" || descriptorType == "ORB" ||
      descriptorType == "FREAK" || descriptorType == "AKAZE" || descriptorType == "SIFT") 
      return true;
  else
      return false;
}

bool isMatcher(std::string matcherType)
{
  if (matcherType == "MAT_BF" || matcherType == "MAT_FLANN") 
      return true;
  else
      return false;
}

bool isSelector(std::string selectorType)
{
  if (selectorType == "SEL_NN" || selectorType == "SEL_KNN") 
      return true;
  else
      return false;
}



