#ifndef SLAM_H
#define SLAM_H

#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/xfeatures2d.hpp"
#include <ros/ros.h>
#include <tf/transform_listener.h>
#include <geometry_msgs/PoseStamped.h>
#include <fstream>
#include <iostream>
#include <string.h>
#include <ctype.h>

class basic_slam{
      ros::NodeHandle nh;
      ros::Publisher pose_pub;
      ros::Publisher pose_pub_f;

      // ros::Subscriber image_sub;
      image_transport::Subscriber image_sub;
      bool first_image;
      //boost::shared_ptr<sensor_msgs::Image const> msg;
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
      cv::Mat imgT1, imgT2;
      cv::Mat rotation ;
      cv::Mat translation ;
      std::vector<cv::Point2f> buffer_pointsT1, pointsT1, pointsT2;
      cv::Mat projecMat;
      std::string calibration_path;
      cv::Mat k;
      int c;
      bool bypass;


public:
  basic_slam(ros::NodeHandle &node, std::string pathOfFile, int method);
  basic_slam(std::string pathOfFile, cv::Mat T1, cv::Mat T2);
  //~basic_slam();
  void load_cam_settings();

  //void load_image(cv::Mat& img, std::string filepath);

  void fastFeatures(cv::Mat image, std::vector<cv::Point2f>& buffer_pointsT1);
  void Orb_matcher(cv::Mat &im1, cv::Mat &im2);
  void SURF_feature_only(cv::Mat image, std::vector<cv::Point2f> &pointsT1);
  void SURF_feature_matching(cv::Mat image1, cv::Mat image2);
  void flow_features(cv::Mat image1, cv::Mat image2);
  void feature_points(cv::Mat image, std::vector<cv::Point2f>& points);
  void calcEfromF(cv::Mat &E, int &correctE);
  void checkE(cv::Mat E);
  void calc_extrinsics();
  cv::Mat ret_rotation();
  cv::Mat ret_translation();
  void computeCameraMatrix(cv::Mat &k, cv::Mat &t, cv::Mat &r, cv::Mat& p);
  void convertToHomogenousMat(std::vector<cv::Point2f> v, cv::Mat &h_m);
  void RtoQuaternion(cv::Mat r, std::vector<double> &q);
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
  // void imageCallback(const sensor_msgs::ImageConstPtr& msg);
  void imageCallback(const sensor_msgs::ImageConstPtr& msg);
  void imageCallback_flow(const sensor_msgs::ImageConstPtr& msg);
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
};

#endif
