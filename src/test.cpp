#include <opencv2/opencv.hpp>
#include "opencv2/xfeatures2d.hpp"
#include "opencv2/features2d.hpp"

using namespace std;
using namespace cv;
using namespace cv::xfeatures2d;

int MAX_FEATURES = 100;
float 	scaleFactor = 1.2f;
int 	nlevels = 8;
int 	edgeThreshold = 31;
int 	firstLevel = 0;
int 	WTA_K = 2;
int 	scoreType = ORB::HARRIS_SCORE;
int 	patchSize = 31;
int 	fastThreshold = 20;
const float GOOD_MATCH_PERCENT = 0.15f;

void surf_matching(cv::Mat image1, cv::Mat image2, Mat &im1Reg, Mat &h ){
  int minHessian = 300;
  cv::Ptr<cv::xfeatures2d::SURF> detector = cv::xfeatures2d::SURF::create( minHessian, 3, 3, false, false );
  std::vector<cv::KeyPoint> kp1, kp2;
  cv::Mat desc1, desc2;
  detector->detectAndCompute( image1, cv::noArray(), kp1, desc1);
  detector->detectAndCompute( image2, cv::noArray(), kp2, desc2);
  cv::Ptr<cv::DescriptorMatcher> matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::FLANNBASED);
  std::vector<std::vector<cv::DMatch> > knnMatches;
  matcher->knnMatch( desc1, desc2, knnMatches, 3);
  const float ratio_thresh = 0.6f;
  std::vector<cv::DMatch> good_matches;
  std::cout<<"knnMatches size "<<knnMatches.size()<<"\n";
  std::vector<cv::Point2f> points1, points2;
  int cnt=0;
  for(size_t i=0; i<knnMatches.size(); i++)
  {
    if(knnMatches[i][0].distance < ratio_thresh * knnMatches[i][1].distance /*&& fabs(kp2[knnMatches[i][1].trainIdx].pt.y - kp1[knnMatches[i][1].queryIdx].pt.y) <10 &&
    (fabs(kp2[knnMatches[i][1].trainIdx].pt.x - kp1[knnMatches[i][1].queryIdx].pt.x) <500)
    && (fabs(kp2[knnMatches[i][1].trainIdx].pt.x - kp1[knnMatches[i][1].queryIdx].pt.x) >200)*/)
    {
      good_matches.push_back(knnMatches[i][0]);
      points1.push_back(kp2[knnMatches[i][1].trainIdx].pt);
      points2.push_back(kp1[knnMatches[i][1].queryIdx].pt);
                         std::cout<<kp2[knnMatches[i][1].trainIdx].pt<<"  "<<"\n";
                         std::cout<<kp1[knnMatches[i][1].queryIdx].pt<<"  "<<"\n";
                         std::cout<<cnt++<<"  "<<"\n";
                         std::cout<<"---------------------------------------\n";


    }
  }
  cv::Mat img_matches;
  cv::drawMatches(image1, kp1, image2, kp2, good_matches, img_matches, cv::Scalar::all(-1),
                 cv::Scalar::all(-1), std::vector<char>(), cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
  std::cout<<"drawmatches worked"<<"\n";
  cv::imshow("Matches", img_matches);
  cv::waitKey(0);

  // Find homography
  h = findHomography( points1, points2, RANSAC );

  int rows = image1.rows + image2.rows;
  int cols = image1.cols + image2.cols;
  // Use homography to warp image
  warpPerspective(image1, im1Reg, h,cv::Size(cols,rows));
  cv::imshow("im1Reg", im1Reg);
  cv::waitKey(0);
  for(int i=0; i<image2.cols; i++){
    for(int j=0; j<image2.rows; j++){
      im1Reg.at<float>(i,j) = image2.at<float>(i,j);
    }
  }
  cv::imshow("im1Reg", im1Reg);

  std::cout<<"image1 size: "<<image1.size()<<"\n";
  std::cout<<"image2 size: "<<image2.size()<<"\n";
  std::cout<<"im1Reg size: "<<im1Reg.size()<<"\n";
}


void alignImages(Mat &im1, Mat &im2, Mat &im1Reg, Mat &h)

{

  // Convert images to grayscale
  Mat im1Gray, im2Gray;
  cvtColor(im1, im1Gray, CV_BGR2GRAY);
  cvtColor(im2, im2Gray, CV_BGR2GRAY);
  cout<<"grayed \n";
  // Variables to store keypoints and descriptors
  std::vector<KeyPoint> keypoints1, keypoints2;
  Mat descriptors1, descriptors2;

  // Detect ORB features and compute descriptors.
  Ptr<Feature2D> orb = ORB::create(MAX_FEATURES = 1000,
      scaleFactor = 1.2f,
      nlevels = 8,
      edgeThreshold = 16,
      firstLevel = 0,
      WTA_K = 2,
      scoreType = cv::ORB::HARRIS_SCORE,
      patchSize = 31,
      fastThreshold = 20);

  orb->detectAndCompute(im1Gray, Mat(), keypoints1, descriptors1);
  orb->detectAndCompute(im2Gray, Mat(), keypoints2, descriptors2);
  cout<<"orbed \n";
  // Match features.
  std::vector<DMatch> matches;
  Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("BruteForce-Hamming");
  matcher->match(descriptors1, descriptors2, matches, Mat());

  // Sort matches by score
  std::sort(matches.begin(), matches.end());

  // Remove not so good matches
  const int numGoodMatches = matches.size() * GOOD_MATCH_PERCENT;
  matches.erase(matches.begin()+numGoodMatches, matches.end());
  cout<<"matches size : "<<matches.size()<<"\n";
  // Draw top matches
  Mat imMatches;
  // drawMatches(im1, keypoints1, im2, keypoints2, matches, imMatches);
  // Extract location of good matches
  std::vector<Point2f> points1, points2;
  std::vector<cv::KeyPoint> plotpt1, plotpt2;
  int cnt=0;
  for( size_t i = 0; i < matches.size(); i++ )
  {
    points1.push_back( keypoints1[ matches[i].queryIdx ].pt );
    points2.push_back( keypoints2[ matches[i].trainIdx ].pt );
    plotpt1.clear();
    plotpt2.clear();
    plotpt1.push_back( keypoints1[ matches[i].queryIdx ]);
    plotpt2.push_back( keypoints2[ matches[i].trainIdx ] );
      cout<<keypoints1[ matches[i].queryIdx ].pt.x<<"  "<<keypoints1[ matches[i].queryIdx ].pt.y<<"\n";
      cout<<keypoints2[ matches[i].queryIdx ].pt.x<<"  "<<keypoints2[ matches[i].queryIdx ].pt.y<<"\n";
      cout<<++cnt<<"\n";
      cout<<">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"<<"\n";
      std::cout<<"plotpt1.size-->"<<plotpt1.size();
      cv::drawMatches(im1, plotpt1, im2, plotpt2, matches, imMatches);
      cv::waitKey(0);
      std::cout<<"cnt match drawn";
  }

  // Find homography
  h = findHomography( points1, points2, RANSAC );

  // Use homography to warp image
  warpPerspective(im1, im1Reg, h, im2.size());
}

int main(int argc, char **argv)
{
  // Read reference image
  string refFilename("/home/tanmay/Desktop/DATASET/01/image_0/000001.png");
  cout << "Reading reference image : " << refFilename << endl;
  Mat imReference = imread(refFilename);


  // Read image to be aligned
  string imFilename("/home/tanmay/Desktop/DATASET/01/image_1/000001.png");
  cout << "Reading image to align : " << imFilename << endl;
  Mat im = imread(imFilename);


  // Registered image will be resotred in imReg.
  // The estimated homography will be stored in h.
  Mat imReg, h;

  // Align images
  // cout << "Aligning images ..." << endl;
  alignImages(im, imReference, imReg, h);
  // surf_matching(imReference, im, imReg, h);
  // imwrite("alighned_epipolar_new.jpg", imReg);

  // Print estimated homography
  cout << "Estimated homography : \n" << h << endl;

}
