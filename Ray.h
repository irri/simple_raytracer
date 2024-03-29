/*
 * Ray.h
 *
 *  Created on: Jan 20, 2012
 *      Author: david
 */

#ifndef RAY_H_
#define RAY_H_

#include <glm/glm.hpp>

using namespace glm;

class Ray {
public:
  Ray();
  Ray(vec3 position, vec3 direction);
  virtual ~Ray();

  static Ray generateRay(vec3 start_position, vec3 end_position);
  static Ray reflection(Ray ray, vec3 normal, vec3 intersection_point);
  static Ray refraction(Ray ray, vec3 normal, vec3 intersection_point, float refr_index);

  const vec3& getPosition() const;
  const vec3& getDirection() const;

private:
  vec3 position;
  vec3 direction;
};


#endif /* RAY_H_ */
