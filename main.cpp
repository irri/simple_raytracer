/*************************************
** PARALLEL RAYTRACER
**************************************
** ASSIGNMENT 1 FOR
** INTELLIGENT INFORMATION SYSTEMS
** TOKYO INSTITUTE OF TECHNOLOGY
**
** Philip Irri (philip@irri.se)
** 2012-11-10
*************************************/

#include <glm/glm.hpp>	// Math and vector manipulation library
#include <png.h>		// Used to export the color buffer to an image
#include <stdio.h>
#include <stdlib.h>

using namespace std;
using namespace glm;

// OUTPUT DIMENSIONS
const int WIDTH = 600;
const int HEIGHT = 400;

// Exports a RGBA pixel buffer to a PNG image
int exportImage(const char* filename, int width, int height, float* buffer);

// Clamp function
inline float clamp(float x)
{
    return x < 0.0f ? 0.0f : (x > 1.0f ? 1.0f : x);
}

class Camera {
public:
	Camera(float aspect_ratio) {
		pos  = vec3(0.0f, 0.0f, 0.0f);
		dir = vec3(0.0f, 0.0f, -1.0f);
		up = vec3(0.0f, 1.0f, 0.0f);

		fovW = 0.7845f; // pi/4
		fovH = aspect_ratio * fovW;
		tanFovW = glm::tan(fovW);
		tanFovH = glm::tan(fovH);
	}
	
	// Vectors
	vec3 pos;
	vec3 dir;
	vec3 up;
	
	// Field of view
	float fovW;
	float fovH;
	float tanFovW;
	float tanFovH;
};

class Sphere {
public:
	Sphere() {
		pos = vec3(0.0f, 0.0, 0.0f);
		radius = 1.0f;
		diffuse = vec3(1.0f, 1.0, 1.0f);
		specular = vec3(1.0f, 0.0, 0.0f);
		refl = 0.2f;
	}
	
	// Checks intersection between a ray and the sphere.
	// Returns a boolen if an intersection was found.
	// parameter t will be assigned the distance from 
	// the start of the ray to the intersection point.
	bool intersect(vec3 start, vec3 dir, float & t) {
		vec3 dist = pos - start; 
		float B = glm::dot(dir, dist);
		float D = B*B - glm::dot(dist, dist) + radius * radius; 
		if (D < 0.0f) 
		    return false; 
		float t0 = B - glm::sqrt(D); 
		float t1 = B + glm::sqrt(D);
		bool retvalue = false;  
		if ((t0 > 0.1f) && (t0 < t)) 
		{
		    t = (float)t0;
		    retvalue = true; 
		} 
		if ((t1 > 0.1f) && (t1 < t)) 
		{
		    t = (float)t1; 
		    retvalue = true; 
		}
		return retvalue;
	}
	
	// Sphere properties
	vec3 pos;
	float radius;
	
	// Material properties
	vec3 diffuse;
	vec3 specular;
	float refl;		// [0.0 - 1.0] where 0.0 is non reflective and
					// 1.0 is "mirror" reflective material
};

class Light {
public:
	Light() {
		pos = vec3(0.0, 0.0f, 0.0f);
		color = vec3(1.0, 1.0f, 1.0f);
		intensity = 0.7f;
	}
	
	vec3 pos;
	vec3 color;
	float intensity;
};

// Writes RGBA values to the buffer from a color vector
void setColor(float* buffer, int n, const vec3* color) {
	buffer[n +0] = clamp(color->r);
	buffer[n +1] = clamp(color->g);
	buffer[n +2] = clamp(color->b);
	buffer[n +3] = 1.0f;
}

// CONFIGURATION VARIABLES
///////////////////////////////////////////////////
const unsigned int MAX_DEPTH = 3;
vec3 background_color = vec3(0.1f, 0.1f, 0.1f);
const float aspect_ratio = (float)HEIGHT / (float)WIDTH;
Camera my_camera = Camera(aspect_ratio);
const int NR_LIGHTS = 4;
Light lights[NR_LIGHTS];
const int NR_SPHERES = 10;
Sphere spheres[NR_SPHERES];
///////////////////////////////////////////////////

// The shade function
// Calculates and returns the final color of a pixel
vec3 shade(vec3 startpos, vec3 incoming_direction, int recursion_depth) {

	vec3 color = vec3(0.0f, 0.0f, 0.0f);
	
	// Check for closest intersection of all spheres
	int sphere_id = -1;
	float t = 9999999.0f; // Large value
	float closest = 9999999.0f;
	for (int i=0; i<NR_SPHERES; ++i) {
		if ( spheres[i].intersect(startpos, incoming_direction, t) ) {
			if (t < closest) {
				closest = t;
				sphere_id = i;
			}
		}
	}
	
	// If a intersection was found run calculate the pixel color
	if (sphere_id != -1) {

		// Vectors
		vec3 intersection_point = startpos + t*incoming_direction;
		vec3 normal = intersection_point - spheres[sphere_id].pos;
		normal = glm::normalize(normal);

		// For each light in the scene
		for (int i=0; i<NR_LIGHTS; ++i) {
			vec3 light_ray = intersection_point - lights[i].pos;
			light_ray = glm::normalize(light_ray);
		
			// diffuse lightning
			vec3 diffuse = clamp(glm::dot(-light_ray, normal)) 
						   * spheres[sphere_id].diffuse;
			
			// specular lightning
			vec3 h = normalize(-light_ray - incoming_direction);
			vec3 specular = glm::pow(clamp(glm::dot(normal, h)), 5.0f) 
						    * spheres[sphere_id].specular;
			
			// If the material is reflective, recusively run the SHADE function
			// with the reflective vector
			vec3 refl_color = vec3(0.0f, 0.0f, 0.0f);
			if (recursion_depth < MAX_DEPTH && spheres[sphere_id].refl > 0.0f) {
				vec3 refl = glm::reflect(incoming_direction, normal);
				refl = normalize(refl);
				refl_color = 0.9f * shade(intersection_point+normal*0.001f, 
										  refl, recursion_depth+1);
			}
			
			color += (1.0f - spheres[sphere_id].refl) * 
					 (  lights[i].intensity * lights[i].color 
					 	* (diffuse + specular)
					 ) ;
			color += spheres[sphere_id].refl * refl_color;
			
		}


	}
	// If no intersection was found
	else {
		color = background_color;
	}
	
	return color;
}

int main() {

	// Allocate memory for color buffer
	float* color_buffer = new float[WIDTH*HEIGHT*4]; // rgba

	// Set up lights
	lights[0].pos = vec3(-0.5f, -1.0f, 0.0f);
	lights[0].color = vec3(0.5f, 0.5f, 1.0f);
	lights[1].pos = vec3(0.5f, 1.0f, -1.0f);
	lights[1].color = vec3(1.0f, 0.5f, 1.0f);
	lights[2].pos = vec3(0.5f, 1.5f, -1.5f);
	lights[2].color = vec3(0.1f, 1.0f, 0.2f);
	lights[3].pos = vec3(0.5f, -3.0f, -2.5f);
	lights[3].color = vec3(0.1f, 0.0f, 0.5f);	
		
	
	// Set up spheres
	spheres[0].pos = vec3(0.0f, 0.0f, -2.5f);
	spheres[0].refl = 0.7;
	
	spheres[1].pos = vec3(-0.5f, 0.5f, -3.5f);
	spheres[1].radius = 1.4;
	spheres[1].diffuse = vec3(0.1f, 0.8f, 0.8f);
	
	spheres[2].pos = vec3(0.5f, 0.0f, -1.0f);
	spheres[2].radius = 0.4;
	spheres[2].refl = 0.0;
	
	spheres[3].pos = vec3(-0.5f, 0.0f, -1.0f);
	spheres[3].radius = 0.3;
	spheres[3].refl = 0.3;
	
	spheres[4].pos = vec3(0.1f, -0.1f, -0.6f);
	spheres[4].radius = 0.15;
	spheres[4].diffuse = vec3(1.0f, 0.8f, 0.5f);
	spheres[4].specular = vec3(0.0f, 0.8f, 1.0f);
	
	spheres[5].pos = vec3(4.0f, 3.0f, -5.0f);
	spheres[5].radius = 2.0f;
	spheres[5].diffuse = vec3(1.0f, 0.3f, 0.1f);
	spheres[5].specular = vec3(0.0f, 0.8f, 1.0f);
	
	spheres[6].pos = vec3(-10.0f, 0.0f, -10.0f);
	spheres[6].radius = 9.0f;
	spheres[6].diffuse = vec3(0.0f, 0.6f, 0.0f);
	spheres[6].specular = vec3(1.0f, 0.0f, 1.0f);
	spheres[6].refl = 0.5;
	
	spheres[7].pos = vec3(0.0f, 0.0f, 3.0f);
	spheres[7].radius = 1.0f;
	spheres[7].diffuse = vec3(0.2f, 0.0f, 0.7f);
	spheres[7].specular = vec3(0.0f, 0.8f, 1.0f);
	spheres[7].refl = 0.5;
	
	spheres[8].pos = vec3(1.0f, -1.0f, 0.0f);
	spheres[8].radius = 1.0f;
	spheres[8].diffuse = vec3(0.8f, 0.0f, 0.2f);
	spheres[8].specular = vec3(0.7f, 0.5f, 1.0f);
	spheres[8].refl = 0.5;
	
	spheres[9].pos = vec3(-1.0f, 3.0f, 3.0f);
	spheres[9].radius = 1.0f;
	spheres[9].diffuse = vec3(0.5f, 0.5f, 0.0f);
	spheres[9].specular = vec3(0.9f, 0.5f, 0.0f);
	spheres[9].refl = 0.5;
	
	
	#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < (WIDTH*HEIGHT); ++i) {
//	for (int v = 0; v < HEIGHT; ++v) {
//		for (int u = 0; u < WIDTH; ++u) {
			
			vec3 final_color = vec3(0.0f, 0.0f, 0.0f);
			
			int u = i%WIDTH;
			int v = i/WIDTH;
			float x = ((2.0f * (float)u) - (float)WIDTH) 
						/ (float)WIDTH * my_camera.tanFovW;
			float y = ((2.0f * (float)v) - (float)HEIGHT)
						/ (float)HEIGHT * my_camera.tanFovH;

			vec3 target = vec3(x, y, my_camera.dir.z);
			target = glm::normalize(target);
			vec3 camera_ray = target - my_camera.pos;
			
			final_color = shade(my_camera.pos, camera_ray, 0);
			setColor(color_buffer, i*4, &final_color);
	}

	exportImage("out.png", WIDTH, HEIGHT, color_buffer);
	
	delete [] color_buffer;
	return 0;
	
}

int exportImage(const char* filename, int width, int height, float* buffer) {
  FILE * fp;
  png_structp png_ptr = NULL;
  png_infop info_ptr = NULL;
  int x, y;
  png_byte ** row_pointers = NULL;
  /* "status" contains the return value of this function. At first
     it is set to a value which means 'failure'. When the routine
     has finished its work, it is set to a value which means
     'success'. */
  int status = -1;
  /* The following number is set by trial and error only. I cannot
     see where it it is documented in the libpng manual.
  */
  int pixel_size = 4;  //RGBA
  int depth = 8;

  fp = fopen (filename, "wb");
  if (! fp) {
    goto fopen_failed;
  }

  png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png_ptr == NULL) {
    goto png_create_write_struct_failed;
  }

  info_ptr = png_create_info_struct (png_ptr);
  if (info_ptr == NULL) {
    goto png_create_info_struct_failed;
  }

  /* Set up error handling. */

/*  if (setjmp (png_jmpbuf (png_ptr))) {
    goto png_failure;
  }
*/
  /* Set image attributes. */

  png_set_IHDR (png_ptr,
          info_ptr,
          width,
          height,
          depth,
          PNG_COLOR_TYPE_RGB_ALPHA,
          PNG_INTERLACE_NONE,
          PNG_COMPRESSION_TYPE_DEFAULT,
          PNG_FILTER_TYPE_DEFAULT);

  /* Initialize rows of PNG. */

  row_pointers = (png_byte **) png_malloc (png_ptr, height * sizeof (png_byte *));
  //for (y = 0; y < height; ++y) {
  for (y = 0; y < height; ++y) {
    png_byte *row =
      (png_byte *) png_malloc (png_ptr, sizeof (unsigned char) * width * pixel_size);
    row_pointers[height-1-y] = row;
    for (x = 0; x < width; ++x) {
      *row++ = 255*buffer[(y*width+x)*pixel_size];
      *row++ = 255*buffer[(y*width+x)*pixel_size+1];
      *row++ = 255*buffer[(y*width+x)*pixel_size+2];
      *row++ = 255*buffer[(y*width+x)*pixel_size+3];
    }
  }

  /* Write the image data to "fp". */

  png_init_io (png_ptr, fp);
  png_set_rows (png_ptr, info_ptr, row_pointers);
  png_write_png (png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

  /* The routine has successfully written the file, so we set
     "status" to a value which indicates success. */

  status = 0;

  for (y = 0; y < height; y++) {
    png_free (png_ptr, row_pointers[y]);
  }
  png_free (png_ptr, row_pointers);

  //png_failure:
  png_create_info_struct_failed:
     png_destroy_write_struct (&png_ptr, &info_ptr);
  png_create_write_struct_failed:
     fclose (fp);
  fopen_failed:
     return status;
     
}


