#include "SLAM.h"
//constructor runs the surf feature matching with a one initial reference frame
basic_slam::basic_slam(ros::NodeHandle &node, std::string pathOfFile, int method){
  nh = node;
  first_image = false;
  cv::Mat rotation = cv::Mat::eye(3, 3, CV_32FC1);
  cv::Mat translation = cv::Mat::zeros(3, 1, CV_32FC1);
  calibration_path = pathOfFile;
  c=0;
  load_cam_settings();
  pose_pub = nh.advertise<geometry_msgs::PoseStamped>("/estimated_pose", 1);
  pose_pub_f = nh.advertise<geometry_msgs::PoseStamped>("/estimated_pose_f", 1);
  std::cout<<"pub \n";
  image_transport::ImageTransport it(nh);
  std::cout<<"it done \n";
  bypass = true;
  if(method ==1)
  image_sub = it.subscribe("/camera/rgb/image_raw", 1, &basic_slam::imageCallback_flow, this);
  else
  image_sub = it.subscribe("/camera/rgb/image_raw", 1, &basic_slam::imageCallback, this);
}

basic_slam::basic_slam(std::string pathOfFile, cv::Mat T1, cv::Mat T2){
  cv::Mat rotation = cv::Mat::eye(3, 3, CV_64FC1);
  cv::Mat translation = cv::Mat::zeros(3, 1, CV_64FC1);
  calibration_path = pathOfFile;
  imgT1 = T1;
  imgT2 = T2;
  load_cam_settings();
  SURF_feature_matching(imgT1, imgT2);

  //feature_points(imgT2, pointsT2);
  //feature_points(imgT1, pointsT1);
  // cout<<"points T1: "<< '\n' << pointsT1<<"\n";
  // cout<<"points T2: "<< '\n' << pointsT2<<"\n";
  std::cout<<"constructed \n";
}


void basic_slam::Orb_matcher(cv::Mat &im1, cv::Mat &im2)
{

  // Convert images to grayscale
  cv::Mat im1Gray, im2Gray;
  cv::cvtColor(im1, im1Gray, CV_BGR2GRAY);
  cv::cvtColor(im2, im2Gray, CV_BGR2GRAY);

  // Variables to store keypoints and descriptors
  std::vector<cv::KeyPoint> keypoints1, keypoints2;
  cv::Mat descriptors1, descriptors2;

  // Detect ORB features and compute descriptors.
  cv::Ptr<cv::Feature2D> orb = cv::ORB::create(1000);
  orb->detectAndCompute(im1Gray, cv::Mat(), keypoints1, descriptors1);
  orb->detectAndCompute(im2Gray, cv::Mat(), keypoints2, descriptors2);

  // Match features.
  std::vector<cv::DMatch> matches;
  cv::Ptr<cv::DescriptorMatcher> matcher = cv::DescriptorMatcher::create("BruteForce-Hamming");
  matcher->match(descriptors1, descriptors2, matches, cv::Mat());

  // Sort matches by score
  std::sort(matches.begin(), matches.end());

  // Remove not so good matches
  const int numGoodMatches = matches.size() * 0.2f;
  matches.erase(matches.begin()+numGoodMatches, matches.end());
  std::cout<<"matches size"<<matches.size()<<"\n";

  // Draw top matches
  cv::Mat imMatches;
  cv::drawMatches(im1, keypoints1, im2, keypoints2, matches, imMatches);


  // Extract location of good matches
  // std::vector<Point2f> points1, points2;
  pointsT1.clear();
  pointsT2.clear();
  int cnt=0;
  for( size_t i = 0; i < matches.size(); i++ )
  {
    if(fabs(keypoints1[ matches[i].queryIdx ].pt.y - keypoints2[ matches[i].queryIdx ].pt.y) < 2){
    pointsT1.push_back( keypoints1[ matches[i].queryIdx ].pt );
    pointsT2.push_back( keypoints2[ matches[i].trainIdx ].pt );

      std::cout<<keypoints1[ matches[i].queryIdx ].pt.x<<"  "<<keypoints1[ matches[i].queryIdx ].pt.y<<"\n";
      std::cout<<keypoints2[ matches[i].queryIdx ].pt.x<<"  "<<keypoints2[ matches[i].queryIdx ].pt.y<<"\n";
      std::cout<<++cnt<<"\n";
      std::cout<<">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"<<"\n";
    }
  }

  cv::imshow("match_frame", imMatches);
  cv::waitKey(0);
}


void basic_slam::fastFeatures(cv::Mat image, std::vector<cv::Point2f>& buffer_pointsT1)	{
  std::vector<cv::KeyPoint> keypoints_1;
  int fast_threshold = 20;
  bool nonmaxSuppression = false;
  cv::FAST(image, keypoints_1, fast_threshold, nonmaxSuppression);
  cv::KeyPoint::convert(keypoints_1, buffer_pointsT1, std::vector<int>());
  std::cout<<" fast corners detected"<<keypoints_1.size()<<"\n";
}

void basic_slam::SURF_feature_only(cv::Mat image, std::vector<cv::Point2f> &pointsT1){
  int minHessian = 3000;
  cv::Ptr<cv::xfeatures2d::SURF> detector = cv::xfeatures2d::SURF::create( minHessian );
  std::vector<cv::KeyPoint> kp1;
  cv::Mat desc;
  detector->detectAndCompute( image, cv::noArray(), kp1, desc);
  cv::KeyPoint::convert(kp1, pointsT1, std::vector<int>());
}

void basic_slam::SURF_feature_matching(cv::Mat image1, cv::Mat image2){
  int minHessian = 400;
  cv::Ptr<cv::xfeatures2d::SURF> detector = cv::xfeatures2d::SURF::create( minHessian, 4, 2, true, false );
  std::vector<cv::KeyPoint> kp1, kp2, matched_kp1, matched_kp2;
  std::vector<cv::Point2f> m_pt1, m_pt2;
  cv::Mat desc1, desc2;
  detector->detectAndCompute( image1, cv::noArray(), kp1, desc1);
  detector->detectAndCompute( image2, cv::noArray(), kp2, desc2);
  try{
    if(kp1.size()==0 || kp2.size()==0)
      throw kp2;

  cv::Ptr<cv::DescriptorMatcher> matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::FLANNBASED);
  std::vector<std::vector<cv::DMatch> > knnMatches;
  matcher->knnMatch( desc1, desc2, knnMatches, 3);

  const float ratio_thresh = 0.6f;
  std::vector<cv::DMatch> good_matches, good_matches_ransac;
  std::cout<<"knnMatches size "<<knnMatches.size()<<"\n";
  good_matches.clear();
  int cnt=0;
  for(size_t i=0; i<knnMatches.size(); i++)
  {
    if(knnMatches[i][0].distance < ratio_thresh * knnMatches[i][1].distance /*&& fabs(kp2[knnMatches[i][1].trainIdx].pt.y - kp1[knnMatches[i][1].queryIdx].pt.y) <3 &&
    (fabs(kp2[knnMatches[i][1].trainIdx].pt.x - kp1[knnMatches[i][1].queryIdx].pt.x) <450)
    && (fabs(kp2[knnMatches[i][1].trainIdx].pt.x - kp1[knnMatches[i][1].queryIdx].pt.x) >340)*/)
    {
      good_matches.push_back(knnMatches[i][0]);
      matched_kp2.push_back(kp2[knnMatches[i][1].trainIdx]);
      matched_kp1.push_back(kp1[knnMatches[i][1].queryIdx]);
      m_pt2.push_back(kp2[knnMatches[i][1].trainIdx].pt);
      m_pt1.push_back(kp1[knnMatches[i][1].queryIdx].pt);
                         std::cout<<kp2[knnMatches[i][1].trainIdx].pt<<"  "<<"\n";
                         std::cout<<kp1[knnMatches[i][1].queryIdx].pt<<"  "<<"\n";

                         std::cout<<cnt++<<"  "<<"\n";
                         std::cout<<"---------------------------------------\n";


    }
  }

  pointsT1 = m_pt1;
  pointsT2 = m_pt2;

  std::cout<<"after ratio thresh m_pt1 : "<<m_pt1.size()<<"\n";
  std::cout<<"after ratio thresh m_pt2 : "<<m_pt2.size()<<"\n";
  cv::Mat mask_ransac;
  cv::findHomography(m_pt1, m_pt2, cv::RANSAC, 3, mask_ransac, 1000, 0.995);
  std::cout<<"mask_ransac:  "<<mask_ransac.size()<<"\n";

  // pointsT1.clear();
  // pointsT2.clear();
  // good_matches_ransac.clear();
  // // using the ransac filtered points
  // int cnt = 0;
  // for(int row = 0; row < mask_ransac.rows; ++row) {
  //     uchar* p = mask_ransac.ptr(row);
  //     for(int col = 0; col < mask_ransac.cols; ++col) {
  //       cnt++;
  //       	if(*p == 1){
  //       	    good_matches_ransac.push_back(good_matches[cnt]);
  //                   std::cout<<m_pt1[cnt].x<<"  "<<m_pt2[cnt].x<<"\n";
  //                   std::cout<<m_pt1[cnt].y<<"  "<<m_pt2[cnt].y<<"\n";
  //                   std::cout<<"---------------------------------------\n";
  //                   pointsT1.push_back(m_pt1[cnt]);
  //                   pointsT2.push_back(m_pt2[cnt]);
  //         }
  //     }
  // }

  // std::cout<<"points1 size after ransac: "<<pointsT1.size()<<"\n";
  // std::cout<<"points2 size after ransac: "<<pointsT2.size()<<"\n";
  // cv::Mat img_matches;
  // cv::drawMatches(image1, kp1, image2, kp2, good_matches, img_matches, cv::Scalar::all(-1),
  //                cv::Scalar::all(-1), std::vector<char>(), cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
  // std::cout<<"drawmatches worked"<<"\n";
  // cv::imshow("Matches", img_matches);
  // cv::waitKey(0);
  }
  catch(int kp2){
    std::cout<<"exception caught \n";
  }
}

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void basic_slam::flow_features(cv::Mat image1, cv::Mat image2){

  // pointsT1 = buffer_pointsT1;
  std::cout<<"into flow features \n";
  std::vector<uchar> status;
  std::vector<float> err;
  cv::Mat image_gray1, image_gray2;
  cv::cvtColor(image1, image_gray1, cv::COLOR_BGR2GRAY);
  cv::cvtColor(image2, image_gray2, cv::COLOR_BGR2GRAY);
  cv::TermCriteria criteria = cv::TermCriteria((cv::TermCriteria::COUNT) + (cv::TermCriteria::EPS), 30, 0.3);
  std::cout<<"pointsT1 before flow "<<pointsT1.size()<<"\n"<<"pointsT2 before flow "<<pointsT2.size()<<"\n";
  cv::calcOpticalFlowPyrLK(image1, image2, pointsT1, pointsT2, status, err, cv::Size(21,21), 3, criteria, cv::OPTFLOW_USE_INITIAL_FLOW);
  std::cout<<"pointsT1 after flow "<<pointsT1.size()<<"\n"<<"pointsT2 after flow "<<pointsT2.size()<<"\n";

  std::vector<cv::Point2f> good_new;
  std::vector<cv::Scalar> colors;
  cv::RNG rng;
  cv::Mat mask = cv::Mat::zeros(image1.size(), image1.type());
  for(int i = 0; i < 100; i++)
  {
    int r = rng.uniform(0, 256);
    int g = rng.uniform(0, 256);
    int b = rng.uniform(0, 256);
    colors.push_back(cv::Scalar(r,g,b));
  }
  std::vector<cv::Point2f> m_pt1 , m_pt2;
  for(uint i = 0; i < pointsT1.size(); i++)
  {
    if(status[i] == 1) {
        good_new.push_back(pointsT2[i]);
        m_pt1.push_back(pointsT1[i]);
        m_pt2.push_back(pointsT2[i]);
        // draw the tracks
        cv::line(mask, pointsT2[i], pointsT1[i], colors[100], 2);
        cv::circle(image2, m_pt2[i], 5, colors[i]);
      }
  }
  pointsT1.clear();
  pointsT2.clear();
  pointsT1 = m_pt1;
  pointsT2 = m_pt2;
  std::cout<<"pointsT1 after track "<<pointsT1.size()<<"\n"<<"pointsT2 after track "<<pointsT2.size()<<"\n";

  cv::Mat mask_ransac;

  // cv::findHomography(m_pt1, m_pt2, cv::RANSAC, 3, mask_ransac, 2000, 0.999);
  // std::cout<<"mask_ransac:  "<<mask_ransac.size()<<"\n";
  // pointsT1.clear();
  // pointsT2.clear();
  // int cnt = 0;
  // for(int row = 0; row < mask_ransac.rows; ++row) {
  //       uchar* p = mask_ransac.ptr(row);
  //       for(int col = 0; col < mask_ransac.cols; ++col) {
  //         cnt++;
  //         if(*p == 1){
  //             std::cout<<m_pt1[cnt].x<<"  "<<m_pt2[cnt].x<<"\n";
  //             std::cout<<m_pt1[cnt].y<<"  "<<m_pt2[cnt].y<<"\n";
  //
  //             std::cout<<"---------------------------------------\n";
  //             pointsT1.push_back(m_pt1[cnt]);
  //             pointsT2.push_back(m_pt2[cnt]);
  //             cv::circle(imgT1, m_pt1[cnt], 5, colors[cnt]);
  //             cv::circle(imgT2, m_pt2[cnt], 5, colors[cnt]);
  //           }
  //         }
  //       }
  //       std::cout<<"number of points after tracking pointsT1: "<<pointsT1.size();
  //       std::cout<<"number of points after tracking pointsT2: "<<pointsT2.size();
  cv::Mat img;
  cv::add(image2, mask, img);
  cv::imshow("frame1", imgT1);
  cv::imshow("Frame2", imgT2);
  cv::waitKey(0);
  // imgT1 = imgT2.clone();
  // buffer_pointsT1 = pointsT2;
  std::cout<<"out of flow features \n";
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void basic_slam::feature_points(cv::Mat image, std::vector<cv::Point2f>& points){
  std::vector<cv::KeyPoint> keypoints;
  int thresh = 20;
  cv::FAST(image, keypoints, thresh, true);
  cv::KeyPoint::convert(keypoints, points, std::vector<int>());
  std::cout<<"generated feature points \n";
}

void basic_slam::load_cam_settings(){
    std::cout<<"calib path "<<calibration_path<<"\n";
    // cv::FileStorage fSettings(calibration_path, cv::FileStorage::READ);
    // float fx = fSettings["Camera.fx"];
    // float fy = fSettings["Camera.fy"];
    // float cx = fSettings["Camera.cx"];
    // float cy = fSettings["Camera.cy"];
    // float bf = fSettings["Camera.bf"];
    // cout<<fx<<" "<<fy<<" "<<cx<<" "<<cy<<"\n";
    //projecMat = (cv::Mat_<float>(3,4) << fx, 0., cx, 0., 0., fy, cy, 0., 0,  0., 1., 0.);
    projecMat = (cv::Mat_<double>(3,3) << 530.4, 0.0, 320.5, 0.0, 530.4, 240.5, 0.0, 0.0, 1.0);
     projecMat = (cv::Mat_<double>(3,3) << 718.8560, 0.0, 607.1928, 0.0, 718.8560, 185.2157, 0.0, 0.0, 1.0);

    k = projecMat;
}


// void basic_slam::calc_extrins_2(){
//
//        cv::Mat F, E, mask;
//        F = cv::findFundamentalMat(pointsT1,pointsT2,cv::FM_LMEDS);
//        E = (k.t().dot(F)).dot(k);
// // function to be completed
//      }

void basic_slam::calcEfromF(cv::Mat &E, int &correctE){
  cv::Mat F = cv::findFundamentalMat(pointsT1, pointsT2, cv::FM_RANSAC, 1000, 0.99);
  checkE(F);
  E = k.t() * F * k;
  if(fabsf(determinant(E)) > 1e-07) {
      correctE = 0;
			std::cout << "det(E) != 0 : " << determinant(E) << "\n";
      }
    }

void basic_slam::checkE(cv::Mat E){
  cv::Mat h_pt1, h_pt2;
  convertToHomogenousMat(pointsT1, h_pt1);
  convertToHomogenousMat(pointsT2, h_pt2);


  cv::Mat E_plot = h_pt1*E*h_pt2.t();
  std::vector<double> eplot;
  for (int i = 0; i < E_plot.rows; ++i) {
    eplot.push_back(E_plot.at<double>(i,i));
  }
  // eplot = E_plot.data;
  std::ofstream output_file("../catkin_ws/src/slam_test/xExval.txt");
  for(std::vector<double>::iterator i = eplot.begin(); i != eplot.end(); ++i) {
    output_file << *i << '\n';
  }
  // std::ostream_iterator<double> output_iterator(output_file, "\n");
  // std::copy(eplot.begin(), eplot.end(), output_iterator);

}

void basic_slam::calc_extrinsics(){

     cv::Mat F, E, mask, m1 = cv::Mat::zeros(3, 4, CV_64FC1), m2=cv::Mat::zeros(3, 4, CV_64FC1), zeros = cv::Mat::zeros(3,1, CV_64FC1), I = cv::Mat::eye(3,3, CV_64FC1) ;
     //cv::Mat translation_mono = cv::Mat::zeros(3, 1, CV_32FC1);
     cv::Mat points_4d, points_3d;
     cv::Mat rotation_f = cv::Mat::eye(3, 3, CV_64FC1);
     cv::Mat translation_f = cv::Mat::zeros(3, 1, CV_64FC1), translation_intial = cv::Mat::zeros(3, 1, CV_64FC1);
     // //triangulation to find 3d point in space
     // cv::triangulatePoints( pointsT1, pointsT2,projecMat, projecMat, points_4d);
     // cv::convertPointsFromHomogeneous(points_4d.t(), points_3d);
     //rotation estimation using Nister's 5 point algorithm
     double focal = projecMat.at<double>(0,0);
     std::cout<<"found focal  \n";
     cv::Point2d principle_point(projecMat.at<double>(0,2), projecMat.at<double>(1,2));
     std::cout<<"focal: "<<focal<<"\n"<<principle_point<<"\n";
     // CALCULATING the fundamental Matrix
     std::cout<<"k: "<<projecMat<<"\n"<<projecMat.size()<<"\n";
     // F, mask = cv::findFundamentalMat(pointsT1,pointsT2,cv::FM_LMEDS);
     // std::cout<<"F: "<<F<<std::endl;
     //CALCULATING the essential matrix
     std::cout<<"pointsT1 size "<<pointsT1.size()<<"\n"<<"pointsT2 size: "<<pointsT2.size()<<"\n";
     E = cv::findEssentialMat(pointsT1, pointsT2, focal, principle_point, cv::RANSAC, 0.999,2.0);
     // checkE(E);
     cv::Mat Ef;
     int correctE = 1;
     calcEfromF(Ef, correctE);
     std::cout<<"E: "<<E<<"\n"<<"Ef: "<<Ef<<"\n";
     //std::cout<<"E:"<<E<<"\n";
     cv::recoverPose(Ef, pointsT1, pointsT2, k, rotation_f, translation_f, focal, mask, points_4d);
     cv::recoverPose(E, pointsT1, pointsT2, k, rotation, translation, focal, mask, points_4d);
     translation = translation_intial + rotation*translation;
     std::cout<<"R: "<<rotation<<"\n"<<"T: "<<translation<<"\n";
     std::cout<<rotation.at<double>(0,0)<<"\n";
     //calculate camera matrix for both the camera positions
     // computeCameraMatrix(k, translation, rotation, m2);
     // std::cout<<"m2"<<m2<<"\n";
     //computeCameraMatrix(k, zeros, eye, m1);
     // hconcat(k*I, k*zeros, m1);
     // std::cout<<"m1"<<m1<<"\n";

     // cv::Mat pt1 = cv::Mat(pointsT1);
     // cv::Mat pt2 = cv::Mat(pointsT2);
     // //std::cout<<"m2: "<<m1<<endl;
     //triangulatePoints to obtain coressponding 3D point
     // std::cout<<"pt1: "<<pointsT1<<"\n";
     //convertToHomogenous(pt1.t(), h_pt1);
     // std::cout<<"h_pt1: "<<h_pt1.t()<<"\n";
     // convertToHomogenous(m2, h_m2);
     // std::cout<<"h_m2: "<<h_m2<<"\n";
     std::vector<double> q, qf;
     geometry_msgs::PoseStamped p, pf;
     // translation = k*translation;
     // rotation = k*rotation;

     RtoQuaternion(rotation, q);
     p.header.frame_id = "/odom";
     p.header.stamp = ros::Time::now();
     p.pose.position.x = translation.at<double>(0,0);
     p.pose.position.y = translation.at<double>(0,1);
     p.pose.position.z = translation.at<double>(0,2);
     p.pose.orientation.x = q[0];
     p.pose.orientation.y = q[1];
     p.pose.orientation.z = q[2];
     p.pose.orientation.w = q[3];
     // p.header.frame_id = "/odom";
     // p.header.stamp = ros::Time::now();
     // p.pose.position.x = 0;
     // p.pose.position.y = 0;int
     // p.pose.position.z = 0;
     // p.pose.orientation.x = 0;
     // p.pose.orientation.y = 0;
     // p.pose.orientation.z = 0;
     // p.pose.orientation.w = 0;



     if(correctE = 1){
       RtoQuaternion(rotation_f, qf);
       pf.header.frame_id = "/odom";
       pf.header.stamp = ros::Time::now();
       pf.pose.position.x = translation_f.at<double>(0,0);
       pf.pose.position.y = translation_f.at<double>(0,1);
       pf.pose.position.z = translation_f.at<double>(0,2);
       pf.pose.orientation.x = qf[0];
       pf.pose.orientation.y = qf[1];
       pf.pose.orientation.z = qf[2];
       pf.pose.orientation.w = qf[3];
     }
     else{
       pf.header.frame_id = "/odom";
       pf.header.stamp = ros::Time::now();
       pf.pose.position.x = 0 ;
       pf.pose.position.y = 0;
       pf.pose.position.z = 0;
       pf.pose.orientation.x = 0;
       pf.pose.orientation.y = 0;
       pf.pose.orientation.z = 0;
       pf.pose.orientation.w = 0;
     }

     pose_pub.publish(p);
     pose_pub_f.publish(pf);



     // cv::sfm::euclideanToHomogeneous(pointsT1, pt1);
     // cv::sfm::euclideanToHomogeneous(pointsT2, pt2);
     //
     // cv::triangulatePoints( pt1, pt2, projecMat, projecMat, points_4d);
     // cv::convertPointsFromHomogeneous(points_4d.t(), points_3d);

    //  //translation estimation using solvePnRansac
    //  cv::Mat distCoeff = cv::Mat::zeros(4,1,CV_32FC1C1);
    //  cv::Mat rvec = cv::Mat::zeros(3,1,CV_32FC1C1);
    //  cv::Mat intrinsic_matrix = (cv::Mat_<float>(3,3) << projecMat.at<float>(0,0), projecMat.at<float>(0,1), projecMat.at<float>(0,2),
    //                                                      projecMat.at<float>(1,0), projecMat.at<float>(1,1), projecMat.at<float>(1,2),
    //                                                      projecMat.at<float>(2,0), projecMat.at<float>(2,1), projecMat.at<float>(2,2));
    //
    // int iterations = 500;            //number of iterations for RANSAC
    // float reprojectionError = .5;    // maximum allowed distance to consider it an inlier.
    // float confidence = 0.999;          // RANSAC successful confidence.
    // bool useExtrinsicGuess = true;
    // int flags =cv::SOLVEPNP_ITERATIVE;
    //
    // cv::Mat inliers;
    // cv::solvePnPRansac( points_3d, pointsT1, intrinsic_matrix, distCoeff, rvec, translation, useExtrinsicGuess,
    //                    iterations, reprojectionError, confidence, inliers, flags );

    //cv::triangulatePoints()
   }

   void basic_slam::computeCameraMatrix(cv::Mat &k, cv::Mat &t, cv::Mat &r, cv::Mat& p){
     // Compute second camera matrix from K,R,T
     //cout<<"p_before"<<p<<"\n"<<p.size()<<"\n";
      cv::Mat kt = k*t;
      cv::Mat kr = k*r;
      std::cout<<"mul right"<<"\n";
      cv::hconcat(kr, kt, p);
      //std::cout<<"p_______: "<<p<<"\n";
    }
//
// void computePFromEssential(cv::Mat E){
// }

  void basic_slam::convertToHomogenousMat(std::vector<cv::Point2f> v, cv::Mat &h_m){
      // std::cout<<"input vec size  "<<v<<std::endl;
      // std::cout<<"input vec size internal  "<<v[0]<<std::endl;
      //
      // cv::Mat m(v);
      // std::cout<<"input mat created size  "<<m.size()<<std::endl;
      // cv::Mat last_col = cv::Mat::ones(1, .cols, m.type());
      // cv::vconcat(m,last_col, h_m);
      // // //h_m = h_m.colRange(0,h_m.cols);
      // std::cout<<"h-m size  "<<h_m<<std::endl;
      try{
        if(v.size()!=0){
        cv::Mat h(v.size(),3, CV_64FC1);
        for(int i=0; i<h.rows; i++){
          h.at<double>(i,0) = v[i].x;
          h.at<double>(i,1) = v[i].y;
          h.at<double>(i,2) = 1;
          // std::cout<<" vx, vy :"<<v[i].x<<"  "<<v[i].y<<"\n";
        }
        h_m = h;

        // std::cout<<" homo_size  "<<h.size()<<"\n";
        // std::cout<<" homo  "<<h<<"\n";
      }
    }
      catch(...){
        std::cout<<"homogenous function caused an exception"<<"\n";
      }
}

  void basic_slam::RtoQuaternion(cv::Mat r,std::vector<double> &q){
    //.....equations to convert to Quaternion
    double m00 = r.at<double>(0,0);
    double m01 = r.at<double>(0,1);
    double m02 = r.at<double>(0,2);
    double m10 = r.at<double>(1,0);
    double m11 = r.at<double>(1,1);
    double m12 = r.at<double>(1,2);
    double m20 = r.at<double>(2,0);
    double m21 = r.at<double>(2,1);
    double m22 = r.at<double>(2,2);

    double qx, qy, qz, qw;
    double tr = m00 + m11 + m22;
    if(tr > 0){
  double S = sqrt(tr+1.0) * 2; // S=4*qw
  qw = 0.25 * S;
  qx = (m21 - m12) / S;
  qy = (m02 - m20) / S;
  qz = (m10 - m01) / S;
} else if ((m00 > m11)&(m00 > m22)) {
  double S = sqrt(1.0 + m00 - m11 - m22) * 2;
  qw = (m21 - m12) / S;
  qx = 0.25 * S;
  qy = (m01 + m10) / S;
  qz = (m02 + m20) / S;
} else if (m11 > m22) {
  double S = sqrt(1.0 + m11 - m00 - m22) * 2;
  qw = (m02 - m20) / S;
  qx = (m01 + m10) / S;
  qy = 0.25 * S;
  qz = (m12 + m21) / S;
} else {
  double S = sqrt(1.0 + m22 - m00 - m11) * 2;
  qw = (m10 - m01) / S;
  qx = (m02 + m20) / S;
  qy = (m12 + m21) / S;
  qz = 0.25 * S;
}
q.push_back(qx);
q.push_back(qy);
q.push_back(qz);
q.push_back(qw);

}

cv::Mat basic_slam::ret_rotation(){
  return rotation;
}

cv::Mat basic_slam::ret_translation(){
  return translation;
}


void basic_slam::imageCallback(const sensor_msgs::ImageConstPtr& msg)
{
  //msg = ros::topic::waitForMessage<sensor_msgs::Image>("/camera/rgb/image_raw");
  cv_bridge::CvImagePtr cv_ptr;
 try
 {
   cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
   if(first_image == false){
   imgT1 = cv_ptr->image;
    imgT1 = cv::imread( "/home/tanmay/Desktop/DATASET/01/image_0/000000.png", cv::IMREAD_COLOR);
   if(!imgT1.empty())
     first_image = true;
 }

   else{
     //we have two subsequent images
     imgT2 = cv_ptr->image;
      imgT2 = cv::imread( "/home/tanmay/Desktop/DATASET/01/image_1/000000.png", cv::IMREAD_COLOR);
     //cv::cvtColor(T1, imgT1, cv::COLOR_RGBA2GRAY);
     //cv::cvtColor(T2, imgT2)
     std::cout<<"img1: "<<imgT1.size()<<std::endl;
     std::cout<<"img2: "<<imgT2.size()<<std::endl;
     // cv::imshow("im1", imgT1);
     // cv::imshow("im2", imgT2);
     //
     // cv::waitKey(10000);

     SURF_feature_matching(imgT1, imgT2);
      // cv::waitKey(0);
     calc_extrinsics();
   }
 }
 catch (cv_bridge::Exception& e)
 {
   ROS_ERROR("Could not convert from '%s' to 'bgr8'.", msg->encoding.c_str());
 }
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// imageCallback for feature tracking task
void basic_slam::imageCallback_flow(const sensor_msgs::ImageConstPtr& msg)
{
  //msg = ros::topic::waitForMessage<sensor_msgs::Image>("/camera/rgb/image_raw");
  cv_bridge::CvImagePtr cv_ptr;
 try
 {
   cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
   if(first_image == false){
   imgT1 = cv_ptr->image;
   imgT1 = cv::imread( "/home/tanmay/Desktop/DATASET/01/image_0/000100.png", cv::IMREAD_COLOR);

     if(!imgT1.empty()){
       first_image = true;
       // imgT2 = imgT1;
       // SURF_feature_only(imgT1,buffer_pointsT1); // calculate features using SURF initially
       // fastFeatures(imgT1, buffer_pointsT1);
       // cv::goodFeaturesToTrack(imgT1, buffer_pointsT1, 100, 0.3, 7, cv::Mat(), 5, false, 0.04);
       // std::cout<<"pointsT1: "<<pointsT1<<"\n";
       // cv::imshow("initial_frame", imgT1);
       // cv::imshow("initial_frame", imgT2);
       // cv::waitKey(0);
     }
   }

     else{
       //we have two subsequent images
       imgT2 = cv_ptr->image;
       imgT2 = cv::imread( "/home/tanmay/Desktop/DATASET/01/image_1/000100.png", cv::IMREAD_COLOR);
       std::cout<<"img1: "<<imgT1.size()<<std::endl;
       std::cout<<"img2: "<<imgT2.size()<<std::endl;
       // cv::imshow("im1", imgT1);
       // cv::imshow("im2", imgT2);
       // cv::waitKey(10000);
       // cv::goodFeaturesToTrack(imgT1, buffer_pointsT1, 100 , 0.3, 7, cv::Mat(), 5, false, 0.04);
       fastFeatures(imgT1, buffer_pointsT1);
       // SURF_feature_matching(imgT1, imgT2);

       flow_features(imgT1, imgT2);
       // Orb_matcher(imgT1, imgT2);
       std::cout<<"iteration:  "<< ++c <<"\n";
       // cv::waitKey(0);
       calc_extrinsics();
     }
 }



 catch (cv_bridge::Exception& e)
 {
   ROS_ERROR("Could not convert from '%s' to 'bgr8'.", msg->encoding.c_str());
 }
}
