/**
 * DouglasPeucker.h
 * 
 * Copyright (C) 2015  Joey Andres

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#pragma once

#include <cmath>
#include <list>
#include <utility>
#include <tuple>

template <typename T>
using listIter = typename std::list<T>::iterator;

/*!@class DouglasPeucker
 * @brief Line simplification algorithm.
 * @typename T the data type of the point that will be dealt with.
 */
template<typename T>
class DouglasPeuckerAbstract{
 public:
  /**
   * @param line a list of point with data type T.
   */
  explicit DouglasPeuckerAbstract(const std::list<T>& line) : _line(line) {}

  /**
   * @return line which is a list of points with type T.
   */
  std::list<T>& getLine() { return _line; }

  /**
   * @return line which is a list of points with type T.
   */
  const std::list<T>& getLine() const { return _line; }

  /**
   * @param epsilon The higher, the more points gotten rid of.
   */
  void simplify(double epsilon) {
    listIter<T> endIter = _line.end();
    _simplify(epsilon, _line.begin(), --endIter);
  }

 protected:
  /**
   * @param p1 first point.
   * @param p2 second point.
   * @return distance between p1 and p2.
   */
  virtual double _distance(const T& p1, const T& p2) const = 0;

  /**
   * @param p1 First point in the segment.
   * @param p2 Second point in the segment.
   * @param p A point around the segement.
   * @return distance from segment p1p2 to p.
   */
  virtual double _pointSegmentDistance(const T& p1,
                                       const T& p2,
                                       const T& p) const = 0;

  /**
   * @param i1 index of first point.
   * @param i2 index of second point.
   * @param pMaxDist pointer to the maximum distance of _line[return index].
   * @return the index of the point farthest fromthe segment (t1,t2).
   */
  listIter<T> _getFarthestPointFromSegment(listIter<T> it1,
                                           listIter<T> it2,
                                           double* pMaxDist = nullptr) {
    // Keep track of the point with the maximum distance.
    listIter<T> maxIt = it1;
    maxIt++;
    double maxDist = _pointSegmentDistance(*it1, *it2, *maxIt);

    listIter<T> it = it1;
    it++;
    while (it != it2) {
      double dist = _pointSegmentDistance(*it1, *it2, *it);
      if (dist > maxDist) {
        maxIt = it;
        maxDist = dist;
      }

      it++;
    }

    // Assign pMaxDist a value if it's not null.
    if (pMaxDist) { *pMaxDist = maxDist; }
    return maxIt;
  }

  /**
   * Recursively delete points that are within epsilon.
   * @param epsilon the higher the more aggressive.
   * @param iterator of the first point in a segment.
   * @param iterator of the last point in a segment.
   */
  void _simplify(double epsilon,
                 listIter<T> it1,
                 listIter<T> it2) {
    if (distance(it1, it2) <= 1) return;

    // Acquire the farthest point from the segment it1, it2
    double dist = 0.0f;
    listIter<T> index = _getFarthestPointFromSegment(it1, it2, &dist);

    // If the farthest point exceeds epsilon, recurse, with index as pivot.
    if (dist > epsilon) {
      _simplify(epsilon, it1, index);
      _simplify(epsilon, index, it2);
    } else {
      // Delete everything except it1 and it2.
      auto it = it1;
      it++;
      for (; it != it2; ) {
        _line.erase(it++);
      }
    }
  }

 protected:
  std::list<T> _line;
};

// Default point type 2d, which is a std::tuple<double, double>.
using p2d = std::tuple<double, double>;

// Struct that enclose the accessor for p2d.
struct p2dAccessor{
  inline static double getX(const p2d& p) { return std::get<0>(p); }
  inline static double getY(const p2d& p) { return std::get<1>(p); }
};

/*!@class Distance2D
 * @brief cartesian distance in 2D.
 *
 * @typename T 2D data type.
 * @typename TAccessor2D an accessor class that contains getX and getY.
 *                       defaults to void if T have getX and getY method.
 */
template<typename T, typename TAccessor2D = void>
class Distance2D{
 public:
  static double getDistance(const T& p1, const T& p2) {
    double x2 = TAccessor2D::getX(p1) - TAccessor2D::getX(p2);
    double y2 = TAccessor2D::getY(p1) - TAccessor2D::getY(p2);
    return std::sqrt(x2*x2 + y2*y2);
  }
};

/*!@class Distance2D
 * @brief cartesian distance in 2D.
 *
 * @typename T 2D data type.
 */
template<typename T>
class Distance2D<T, void>{
 public:
  static double getDistance(const T& p1, const T& p2) {
    double x2 = p1.getX() - p2.getX();
    double y2 = p1.getY() - p2.getY();
    return std::sqrt(x2*x2 + y2*y2);
  }
};

/*!@class PointSegmentDistance2D
 * @brief Distance between point and a line segment.
 *
 * @typename T 2D data type.
 * @typename TAccessor2D an accessor class that contains getX and getY.
 *                       defaults to void if T have getX and getY method.
 */
template <typename T, typename TAccessor2D = void>
class PointSegmentDistance2D{
 public:
  static double getDistance(const T& p1,
                            const T& p2,
                            const T& p){
    // Ax + By + C = 0. A = slope, B = 1, C = (-y1+slope*x1).
    // (m, n) is in itp.
    double slope = (TAccessor2D::getY(p2) - TAccessor2D::getY(p1))/
        (TAccessor2D::getX(p2) - TAccessor2D::getX(p1));
    double A = slope;
    double Am = A*TAccessor2D::getX(p);
    double B = -1;
    double Bn = B*TAccessor2D::getY(p);
    double C = TAccessor2D::getY(p1) - slope*TAccessor2D::getX(p1);

    return std::abs(Am + Bn + C)/std::sqrt(A*A+B*B);
  }
};

/*!@class PointSegmentDistance2D
 * @brief Distance between point and a line segment.
 *
 * @typename T 2D data type.
 */
template <typename T>
class PointSegmentDistance2D<T, void>{
 public:
  static double getDistance(const T& p1,
                            const T& p2,
                            const T& p){
    // Ax + By + C = 0. A = slope, B = 1, C = (-y1+slope*x1).
    // (m, n) is in itp.
    double slope = (p2.getY() - p1.getY())/(p2.getX() - p1.getX());
    double A = slope;
    double Am = A*p.getX();
    double B = -1;
    double Bn = B*p.getY();
    double C = p1.getY() - slope*p1.getX();

    return std::abs(Am + Bn + C)/std::sqrt(A*A+B*B);
  }
};

/*!@class DouglasPuecker2D
 * @brief Douglas-Peucker implementation for 2D line.
 *
 * @typename T 2D data type.
 * @typename TAccessor2D an accessor class that contains getX and getY.
 *                       defaults to void if T have getX and getY method.
 */
template <typename T, typename TAccessor2D = void>
class DouglasPuecker2D final : public DouglasPeuckerAbstract<T>{
 public:
  using DouglasPeuckerAbstract<T>::DouglasPeuckerAbstract;

  double _distance(const T& p1,
                   const T& p2) const override {
    return Distance2D<T, TAccessor2D>::getDistance(p1, p2);
  }
  double _pointSegmentDistance(const T& p1,
                               const T& p2,
                               const T& p) const override{
    return PointSegmentDistance2D<T, TAccessor2D>::getDistance(p1, p2, p);
  }
};

/*!@class DouglasPuecker2D
 * @brief Douglas-Peucker implementation for 2D line.
 *
 * @typename T 2D data type.
 */
template <typename T>
class DouglasPuecker2D<T, void> final : public DouglasPeuckerAbstract<T>{
 public:
  using DouglasPeuckerAbstract<T>::DouglasPeuckerAbstract;
  double _distance(const T& p1,
                   const T& p2) const override {
    return Distance2D<T>::getDistance(p1, p2);
  }
  double _pointSegmentDistance(const T& p1,
                               const T& p2,
                               const T& p) const override {
    return PointSegmentDistance2D<T>::getDistance(p1, p2, p);
  }
};

// Default point type for 3d, which is a std::tuple<double, double>.
using p3d = std::tuple<double, double, double>;

// Struct that enclose the accessor for p3d.
struct p3dAccessor{
  inline static double getX(const p3d& p) {
    return std::get<0>(p);
  }

  inline static double getY(const p3d& p) {
    return std::get<1>(p);
  }

  inline static double getZ(const p3d& p) {
    return std::get<2>(p);
  }
};

/*!@class Distance3D
 * @brief cartesian distance in 3D.
 *
 * @typename T 3D data type.
 * @typename TAccessor3D an accessor class that contains getX, getY and getZ.
 *                       defaults to void if T have getX, getY, getZ.
 */
template<typename T, typename TAccessor3D = void>
class Distance3D{
 public:
  static double getDistance(const T& p1, const T& p2) {
    double x2 = TAccessor3D::getX(p1) - TAccessor3D::getX(p2);
    double y2 = TAccessor3D::getY(p1) - TAccessor3D::getY(p2);
    double z2 = TAccessor3D::getZ(p1) - TAccessor3D::getZ(p2);
    return std::sqrt(x2*x2 + y2*y2 + z2*z2);
  }
};

/*!@class Distance3D
 * @brief cartesian distance in 3D.
 *
 * @typename T 3D data type.
 */
template<typename T>
class Distance3D<T, void>{
 public:
  static double getDistance(const T& p1, const T& p2) {
    double x2 = p1.getX() - p2.getX();
    double y2 = p1.getY() - p2.getY();
    double z2 = p1.getZ() - p2.getZ();
    return std::sqrt(x2*x2 + y2*y2 + z2*z2);
  }
};

/*!@class PointSegmentDistance3D
 * @brief Distance between point and a line segment.
 *
 * @typename T 3D data type.
 * @typename TAccessor3D an accessor class that contains getX, getY and getZ.
 *                       defaults to void if T have getX, getY, getZ.
 */
template <typename T, typename TAccessor3D = void>
class PointSegmentDistance3D{
 public:
  static double getDistance(const T& x1,
                            const T& x2,
                            const T& x0){
    /**
     * d = |(x0-x1)x(x0-x2)|/|x2-x1|
     * 
     * Decompose:
     * 
     * Let x0Mx1 = x0-x1
     *     x0Mx2 = x0-x2
     *     x2Mx1 = x2-x1
     */
    double x0Mx1_x = TAccessor3D::getX(x0) - TAccessor3D::getX(x1);
    double x0Mx1_y = TAccessor3D::getY(x0) - TAccessor3D::getY(x1);
    double x0Mx1_z = TAccessor3D::getZ(x0) - TAccessor3D::getZ(x1);

    double x0Mx2_x = TAccessor3D::getX(x0) - TAccessor3D::getX(x2);
    double x0Mx2_y = TAccessor3D::getY(x0) - TAccessor3D::getY(x2);
    double x0Mx2_z = TAccessor3D::getZ(x0) - TAccessor3D::getZ(x2);

    double x2Mx1_x = TAccessor3D::getX(x2) - TAccessor3D::getX(x1);
    double x2Mx1_y = TAccessor3D::getY(x2) - TAccessor3D::getY(x1);
    double x2Mx1_z = TAccessor3D::getZ(x2) - TAccessor3D::getZ(x1);

    // Acquire cross product.
    double cx = x0Mx1_y*x0Mx2_z - x0Mx1_z*x0Mx2_y;
    double cy = x0Mx1_z*x0Mx2_x - x0Mx1_x*x0Mx2_z;
    double cz = x0Mx1_x*x0Mx2_y - x0Mx1_y*x0Mx2_x;

    // Acquire magnitudes.
    double mag01 = std::sqrt(cx*cx + cy*cy + cz*cz);
    double mag02 = std::sqrt(x2Mx1_x*x2Mx1_x +
                             x2Mx1_y*x2Mx1_y +
                             x2Mx1_z*x2Mx1_z);
    return mag01/mag02;
  }
};

/*!@class PointSegmentDistance3D
 * @brief Distance between point and a line segment.
 *
 * @typename T 3D data type.
 */
template <typename T>
class PointSegmentDistance3D<T, void>{
 public:
  static double getDistance(const T& x1,
                            const T& x2,
                            const T& x0){
    /**
     * d = |(x0-x1)x(x0-x2)|/|x2-x1|
     * 
     * Decompose:
     * 
     * Let x0Mx1 = x0-x1
     *     x0Mx2 = x0-x2
     *     x2Mx1 = x2-x1
     */

    double x0Mx1_x = x0.getX() - x1.getX();
    double x0Mx1_y = x0.getY() - x1.getY();
    double x0Mx1_z = x0.getZ() - x1.getZ();

    double x0Mx2_x = x0.getX() - x2.getX();
    double x0Mx2_y = x0.getY() - x2.getY();
    double x0Mx2_z = x0.getZ() - x2.getZ();

    double x2Mx1_x = x2.getX() - x1.getX();
    double x2Mx1_y = x2.getY() - x1.getY();
    double x2Mx1_z = x2.getZ() - x1.getZ();

    // Acquire cross product.
    double cx = x0Mx1_y*x0Mx2_z - x0Mx1_z*x0Mx2_y;
    double cy = x0Mx1_z*x0Mx2_x - x0Mx1_x*x0Mx2_z;
    double cz = x0Mx1_x*x0Mx2_y - x0Mx1_y*x0Mx2_x;

    // Acquire magnitudes.
    double mag01 = std::sqrt(cx*cx + cy*cy + cz*cz);
    double mag02 = std::sqrt(x2Mx1_x*x2Mx1_x +
                             x2Mx1_y*x2Mx1_y +
                             x2Mx1_z*x2Mx1_z);
    return mag01/mag02;
  }
};

/*!@class DouglasPuecker3D
 * @brief Douglas-Peucker implementation for 3d line.
 *
 * @typename T 3D data type.
 * @typename TAccessor3D an accessor class that contains getX, getY and getZ.
 *                       defaults to void if T have getX, getY, getZ.
 */
template <typename T, typename TAccessor3D = void>
class DouglasPuecker3D final : public DouglasPeuckerAbstract<T>{
 public:
  using DouglasPeuckerAbstract<T>::DouglasPeuckerAbstract;
  double _distance(const T& p1,
                   const T& p2) const override {
    double x2 = TAccessor3D::getX(p1) - TAccessor3D::getX(p2);
    double y2 = TAccessor3D::getY(p1) - TAccessor3D::getY(p2);
    double z2 = TAccessor3D::getZ(p1) - TAccessor3D::getZ(p2);
    return std::sqrt(x2*x2 + y2*y2 + z2*z2);
  }
  double _pointSegmentDistance(const T& x1,
                               const T& x2,
                               const T& x0) const override {
    return PointSegmentDistance3D<T, TAccessor3D>::getDistance(x1, x2, x0);
  }
};

/*!@class DouglasPuecker3D
 * @brief Douglas-Peucker implementation for 3d line.
 *
 * @typename T 3D data type.
 */
template <typename T>
class DouglasPuecker3D<T, void> final : public DouglasPeuckerAbstract<T>{
 public:
  using DouglasPeuckerAbstract<T>::DouglasPeuckerAbstract;
  double _distance(const T& p1,
                   const T& p2) const override {
    double x2 = p1.getX() - p2.getX();
    double y2 = p1.getY() - p2.getY();
    double z2 = p1.getZ() - p2.getZ();
    return std::sqrt(x2*x2 + y2*y2 + z2*z2);
  }
  double _pointSegmentDistance(const T& x1,
                               const T& x2,
                               const T& x0) const override {
    return PointSegmentDistance3D<T>::getDistance(x1, x2, x0);
  }
};
