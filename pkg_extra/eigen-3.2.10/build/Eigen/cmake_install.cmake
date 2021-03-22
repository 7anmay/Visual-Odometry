# Install script for directory: /home/tanmay/catkin_ws/src/slam_test/pkg_extra/eigen-3.2.10/Eigen

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Devel")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/eigen3/Eigen" TYPE FILE FILES
    "/home/tanmay/catkin_ws/src/slam_test/pkg_extra/eigen-3.2.10/Eigen/SparseCholesky"
    "/home/tanmay/catkin_ws/src/slam_test/pkg_extra/eigen-3.2.10/Eigen/SparseLU"
    "/home/tanmay/catkin_ws/src/slam_test/pkg_extra/eigen-3.2.10/Eigen/Householder"
    "/home/tanmay/catkin_ws/src/slam_test/pkg_extra/eigen-3.2.10/Eigen/SuperLUSupport"
    "/home/tanmay/catkin_ws/src/slam_test/pkg_extra/eigen-3.2.10/Eigen/LU"
    "/home/tanmay/catkin_ws/src/slam_test/pkg_extra/eigen-3.2.10/Eigen/MetisSupport"
    "/home/tanmay/catkin_ws/src/slam_test/pkg_extra/eigen-3.2.10/Eigen/Dense"
    "/home/tanmay/catkin_ws/src/slam_test/pkg_extra/eigen-3.2.10/Eigen/StdList"
    "/home/tanmay/catkin_ws/src/slam_test/pkg_extra/eigen-3.2.10/Eigen/LeastSquares"
    "/home/tanmay/catkin_ws/src/slam_test/pkg_extra/eigen-3.2.10/Eigen/SPQRSupport"
    "/home/tanmay/catkin_ws/src/slam_test/pkg_extra/eigen-3.2.10/Eigen/SparseQR"
    "/home/tanmay/catkin_ws/src/slam_test/pkg_extra/eigen-3.2.10/Eigen/SparseCore"
    "/home/tanmay/catkin_ws/src/slam_test/pkg_extra/eigen-3.2.10/Eigen/Sparse"
    "/home/tanmay/catkin_ws/src/slam_test/pkg_extra/eigen-3.2.10/Eigen/Eigenvalues"
    "/home/tanmay/catkin_ws/src/slam_test/pkg_extra/eigen-3.2.10/Eigen/StdVector"
    "/home/tanmay/catkin_ws/src/slam_test/pkg_extra/eigen-3.2.10/Eigen/Core"
    "/home/tanmay/catkin_ws/src/slam_test/pkg_extra/eigen-3.2.10/Eigen/IterativeLinearSolvers"
    "/home/tanmay/catkin_ws/src/slam_test/pkg_extra/eigen-3.2.10/Eigen/QR"
    "/home/tanmay/catkin_ws/src/slam_test/pkg_extra/eigen-3.2.10/Eigen/SVD"
    "/home/tanmay/catkin_ws/src/slam_test/pkg_extra/eigen-3.2.10/Eigen/Eigen2Support"
    "/home/tanmay/catkin_ws/src/slam_test/pkg_extra/eigen-3.2.10/Eigen/Array"
    "/home/tanmay/catkin_ws/src/slam_test/pkg_extra/eigen-3.2.10/Eigen/PaStiXSupport"
    "/home/tanmay/catkin_ws/src/slam_test/pkg_extra/eigen-3.2.10/Eigen/Eigen"
    "/home/tanmay/catkin_ws/src/slam_test/pkg_extra/eigen-3.2.10/Eigen/Cholesky"
    "/home/tanmay/catkin_ws/src/slam_test/pkg_extra/eigen-3.2.10/Eigen/Geometry"
    "/home/tanmay/catkin_ws/src/slam_test/pkg_extra/eigen-3.2.10/Eigen/Jacobi"
    "/home/tanmay/catkin_ws/src/slam_test/pkg_extra/eigen-3.2.10/Eigen/StdDeque"
    "/home/tanmay/catkin_ws/src/slam_test/pkg_extra/eigen-3.2.10/Eigen/UmfPackSupport"
    "/home/tanmay/catkin_ws/src/slam_test/pkg_extra/eigen-3.2.10/Eigen/QtAlignedMalloc"
    "/home/tanmay/catkin_ws/src/slam_test/pkg_extra/eigen-3.2.10/Eigen/PardisoSupport"
    "/home/tanmay/catkin_ws/src/slam_test/pkg_extra/eigen-3.2.10/Eigen/OrderingMethods"
    "/home/tanmay/catkin_ws/src/slam_test/pkg_extra/eigen-3.2.10/Eigen/CholmodSupport"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/tanmay/catkin_ws/src/slam_test/pkg_extra/eigen-3.2.10/build/Eigen/src/cmake_install.cmake")

endif()

