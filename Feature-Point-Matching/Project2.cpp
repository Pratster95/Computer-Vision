
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/features2d.hpp"
#include <algorithm>

using namespace std;
using namespace cv;

#define MAX_FRAMES 521


// Global state variables
int mouse_x;   // Where is the mouse ?
int mouse_y;
int count_selection=0;
bool is_mouse_down, is_mouse_up;
Point point1, point2;
Mat frame, roi;
Ptr<ORB> p_orb;
std::vector<cv::KeyPoint> keypoints1, keypoints2;
Mat descriptor1, descriptor2;

bool selectObject = false;
Rect selection;
Point origin;
Mat image;
vector<Rect> selections;



// Mouse callback event
int orb()
{

    // Read the images
    Mat img1 = roi;
    Mat img2 = frame;
    //printf("img1 rows %d cols %d data %p\n", img1.rows, img1.cols, img1.data);
    //printf("img2 rows %d cols %d data %p\n", img2.rows, img2.cols, img2.data);

    // Create the windows
    //namedWindow("ROI");
    //namedWindow("Frame");

    // Make a visualization image
    Mat vis1 = img1.clone();
    Mat vis2 = img2.clone();

    // Calculate the ORB feature points of both images
    p_orb = ORB::create(30000);
    p_orb->detect(img1, keypoints1);
    p_orb->compute(img1, keypoints1, descriptor1);
    p_orb->detect(img2, keypoints2);
    p_orb->compute(img2, keypoints2, descriptor2);

    // Plot the feature points
    for (int i = 0; i < keypoints1.size(); i++)
        circle(vis1, Point(keypoints1[i].pt), 1, Scalar(0, 255, 255), 3);

    for (int i = 0; i < keypoints2.size(); i++)
        circle(vis2, Point(keypoints2[i].pt), 1, Scalar(0, 255, 255), 3);

    // Match the keypoints
    std::vector< DMatch > matches;
    Ptr<BFMatcher> bf = BFMatcher::create(NORM_HAMMING);
    bf->match(descriptor1, descriptor2, matches);

    // Draw the matches
    //Mat vis3;
    //drawMatches(img1, keypoints1, img2, keypoints2, matches, vis3);
    std::vector<Point2f> img1_vec, img2_vec;
    for (size_t i = 0; i < matches.size(); i++)
    {
        //-- Get the keypoints from the matches
        img1_vec.push_back(keypoints1[matches[i].queryIdx].pt);
        img2_vec.push_back(keypoints2[matches[i].trainIdx].pt);
    }
    Mat H;
    if ((!img1_vec.empty()) && (!img2_vec.empty())) {
        H = findHomography(img1_vec, img2_vec, RANSAC);
    }

    //-- Get the corners from the roi ( the object to be "detected" )
    std::vector<Point2f> roi_corners(4);
    roi_corners[0] = Point2f(0, 0);
    roi_corners[1] = Point2f((float)img1.cols, 0);
    roi_corners[2] = Point2f((float)img1.cols, (float)img1.rows);
    roi_corners[3] = Point2f(0, (float)img1.rows);

    std::vector<Point2f> image_corners(4);
    if (!H.empty())
    {
        perspectiveTransform(roi_corners, image_corners, H);
    }
    // draw the lines around the features 
    line(vis2, image_corners[0], image_corners[1], Scalar(0, 255, 0), 4);

    line(vis2, image_corners[1], image_corners[2], Scalar(0, 255, 0), 4);

    line(vis2, image_corners[2], image_corners[3], Scalar(0, 255, 0), 4);

    line(vis2, image_corners[3], image_corners[0], Scalar(0, 255, 0), 4);


    // Show the images
    imshow("Template", vis1);
    waitKey(10);

    imshow("Output", vis2);
    waitKey(10);

    

    return 0;
}

void  showSelections()
{
    for (size_t i = 0; i < selections.size(); i++)
    {
        rectangle(frame, selections[i], Scalar(0, 0, 255), 2);
    }
    imshow("Output", frame);
}

void MouseCallbackEvent(int event, int x, int y, int flags, void* userdata)
{
    mouse_x = x;
    mouse_y = y;
    printf("x %d y %d  event %d  flags %d\n", x, y, event, flags);
    

    if (event == EVENT_LBUTTONDOWN)
    {
        is_mouse_down = true;
        printf("mouse down!\n");
        //point1 = Point(x, y);
        //point2 = Point(x + 20, y + 20);
        //rectangle(frame, point1, point2, CV_RGB(0, 255, 0), 1);
        std::cout << "(" << x << ", " << y << ")" << "\n";
        origin = Point(x, y);
        selectObject = false;
        //showSelections();

    }
    if (event == EVENT_LBUTTONUP)
    {
        selectObject = true;
        selections.push_back(selection);
        is_mouse_down = false;
        is_mouse_up = true;
        printf("mouse up!\n");
        
    }
    if (is_mouse_down == true && is_mouse_up == false) {

        Point xy_point;
        xy_point = Point(x, y);

        rectangle(frame, origin, xy_point, Scalar(0, 0, 255), 2); //drawing a rectangle continuously
        imshow("Output", frame);

    }
    if (selectObject)
    {
        
        
        //destroyWindow("Template");
        //namedWindow("Template", 1);
        selections.erase(selections.end() - 1);

        selection.x = MIN(x, origin.x);
        selection.y = MIN(y, origin.y);
        selection.width = std::abs(x - origin.x) + 1;
        selection.height = std::abs(y - origin.y) + 1;
        count_selection = count_selection + 1;
        
        //selection &= Rect(0, 0, frame.cols, frame.rows);
        //rectangle(frame, origin, point2, CV_RGB(0, 255, 0), 1);

        if (selection.width > 0 && selection.height > 0)
        {
            
              roi = frame(selection);
              
              imshow("Template", roi);
              is_mouse_down = false;
              is_mouse_up = false;
              //waitKey(30);
            //bitwise_not(roi, roi);
           // imshow("Template", roi);
        }
        selectObject = false;
        
    }

    

}


char path[4096];






int main()
{

    // Create a template window
    namedWindow("Template", 1);
    namedWindow("Output", 1);
    
    // Set the mouse callback
    
    setMouseCallback("Output", MouseCallbackEvent, NULL);

    //----
    // Read the video as a series of frames
    //----
    int frameno = 0;
    
    while (1)
    {
        sprintf(path, "pictureframes/frame%05d.jpg", frameno);
        frame = imread(path);

        if (frame.data == 0) {
            printf("ERROR: could not read frame %s\n", path);
            break;
        }
        if (count_selection > 0)
        {
            
            orb();
        }
        else
        {
           
            imshow("Output", frame);
            waitKey(10);
        }
        //
        

       
        waitKey(30);



        
        // Next frame, and loop around
        frameno = (frameno + 1) % MAX_FRAMES;
        

    }
    

    return 0;
}