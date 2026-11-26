#ifndef REVO_VU0_NATIVE_H
#define REVO_VU0_NATIVE_H

#include "PC.h"
#include "runtime.h"

void vu0_sphere_sweep(PcVu0 *v);
void vu0_segment_crosses_triangle(PcVu0 *v);
void vu0_triangle_plane(PcVu0 *v);
void vu0_triangle_normal(PcVu0 *v);

#endif
