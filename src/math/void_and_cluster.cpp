#include "dither.h"

#include "../constants.h"
#include "../macros.h"
#include "math.h"
#include "Rand.h"

using namespace L;

union Cell {
  struct {
    uint32_t i : 1, b : 1, rank : (sizeof(float)*8-2);
  };
  uint32_t full;
};
static const int32_t k_width(9), k_height(9);
static const int32_t k_width_h(k_width/2), k_height_h(k_height/2);
static float k_pixel_dist(1.5f);
float* gaussian_kernel() {
  static float kernel[k_width][k_height];
  const float o2(2.f*k_pixel_dist*k_pixel_dist);
  const float lin(1.f/(PI<float>()*o2));
  for(int32_t i(0); i<k_width; i++)
    for(int32_t j(0); j<k_height; j++)
      kernel[i][j] = lin*pow(E<float>(), -(sqr(i-k_width_h)+sqr(j-k_height_h))/o2);
  return &kernel[0][0];
}
void void_or_cluster(Cell* matrix, uint32_t width, uint32_t height, uint32_t target, uint32_t& rx, uint32_t& ry) {
  static float* kernel(gaussian_kernel());
  float max_value(0.f);
  int32_t xo(Rand::next(0, width-1)), yo(Rand::next(0, height-1)); // Offset start of search
  for(int32_t xod(0); xod<width; xod++)
    for(int32_t yod(0); yod<height; yod++) {
      const int32_t x(pmod(xod+xo, int32_t(width)));
      const int32_t y(pmod(yod+yo, int32_t(height)));
      if((matrix+x*height+y)->b==target) {
        float value(0.f);
        for(int32_t i(0); i<k_width; i++)
          for(int32_t j(0); j<k_height; j++) {
            const float k_value(*(kernel+i*k_height+j));
            const int32_t mx(pmod(x+i-k_width_h, int32_t(width)));
            const int32_t my(pmod(y+j-k_height_h, int32_t(height)));
            value += (((matrix+mx*height+my)->b==target) ? 1.f : 0.f) * k_value;
          }
        if(value>=max_value) {
          max_value = value;
          rx = x;
          ry = y;
        }
      }
    }
}
void save_binary_pattern(Cell* matrix, uint32_t width, uint32_t height) {
  for(uint32_t y(0); y<height; y++)
    for(uint32_t x(0); x<width; x++)
      (matrix+x*height+y)->i = (matrix+x*height+y)->b;
}
void load_binary_pattern(Cell* matrix, uint32_t width, uint32_t height) {
  for(uint32_t y(0); y<height; y++)
    for(uint32_t x(0); x<width; x++)
      (matrix+x*height+y)->b = (matrix+x*height+y)->i;
}
void reverse_binary_pattern(Cell* matrix, uint32_t width, uint32_t height) {
  for(uint32_t y(0); y<height; y++)
    for(uint32_t x(0); x<width; x++)
      (matrix+x*height+y)->b = 1-(matrix+x*height+y)->b;
}
float* L::void_and_cluster(uint32_t width, uint32_t height, float* out) {
  const uint32_t size(width*height);
  Cell* matrix((Cell*)out);
  for(uint32_t i(0); i<size; i++)
    matrix[i].full = 0;

  // Initial binary pattern generation
  const uint32_t n(size/10);
  for(uint32_t i(0); i<n;) {
    const uint32_t x(Rand::next(0, width-1)), y(Rand::next(0, height-1));
    Cell* cell(matrix+x*height+y);
    if(!cell->b) {
      cell->b = 1;
      i++;
    }
  }

  // Void and cluster
  uint32_t cx, cy, vx, vy;
  do {
    // Find clusterest cluster
    void_or_cluster(matrix, width, height, 1, cx, cy);
    (matrix+cx*height+cy)->b = 0;
    // Find voidest void
    void_or_cluster(matrix, width, height, 0, vx, vy);
    (matrix+vx*height+vy)->b = 1;
  } while(cx!=vx || cy!=vy);
  save_binary_pattern(matrix, width, height);

  // Dither matrix generation
  for(uint32_t rank(n-1); ; rank--) {
    void_or_cluster(matrix, width, height, 1, cx, cy);
    (matrix+cx*height+cy)->b = 0;
    (matrix+cx*height+cy)->rank = rank;
    if(!rank) break;
  }
  load_binary_pattern(matrix, width, height);
  for(uint32_t rank(n); rank<size; rank++) {
    void_or_cluster(matrix, width, height, 0, cx, cy);
    (matrix+cx*height+cy)->b = 1;
    (matrix+cx*height+cy)->rank = rank;
  }

  // Normalize
  for(uint32_t i(0); i<size; i++)
    out[i] = float(matrix[i].rank+1)/(size+1);
  return out;
}