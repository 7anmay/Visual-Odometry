#include"SLAM.h"
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/xfeatures2d.hpp"
#include <ros/ros.h>
#include <iostream>
#include <string.h>
#include <ctype.h>

int main(int argc, char** argv){
    ros::init(argc, argv, "basic_slam");
    //calibration path not being used as of now
    std::string calibration_path  = " ";
    // cout<<"found calib path \n";
    ros::NodeHandle nh_;
    basic_slam obj(nh_, calibration_path, 1);
    // while(ros::ok){
    //   obj.imageCallback();
    // }
    ros::spin();
    return 0;
}
