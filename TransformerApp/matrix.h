#ifndef MATRIXH
#define MATRIXH

#include <stdarg.h>
#include <stdlib.h>

#define Matrix_expr(mat, expr)\
for(unsigned i = 0; i < mat.r; i++)\
for(unsigned j = 0; j < mat.c; j++)\
mat.data[i * mat.c + j] = (expr)

struct Mat{
  unsigned r, c;
  float *data;
};
typedef struct Mat Mat;

Mat newMat(unsigned r, unsigned c){
  if(r && c); else return (Mat){0, 0, 0};
  Mat mat = {r, c, 0};
  mat.data = calloc(1ull * r * c, 4);
  return mat;
}

Mat idMat(unsigned d){
  if(!d) return (Mat){0, 0, 0};
  Mat id = {d, d, 0};
  id.data = malloc(4ull * d * d);
  Matrix_expr(id, i == j);
  return id;
}

void fillfMat(Mat mat, ...){
  va_list args;
  va_start(args, mat);
  for(unsigned long long sz = 0; sz < mat.r * mat.c; sz++)
    mat.data[sz] = va_arg(args, double);
  va_end(args);
}

void fillMat(Mat mat, ...){
  va_list args;
  va_start(args, mat);
  for(unsigned long long sz = 0; sz < mat.r * mat.c; sz++)
    mat.data[sz] = va_arg(args, int);
  va_end(args);
}

float readMat(Mat mat, unsigned r, unsigned c){
  return mat.data[r * mat.c + c];
}
float *viewMat(Mat mat, unsigned r, unsigned c){
  return &mat.data[r * mat.c + c];
}
void setMat(Mat mat, unsigned r, unsigned c, float val){
  mat.data[r * mat.c + c] = val;
}

void randMat(Mat mat){
  #pragma omp parallel for
  for(unsigned long long i = 0; i < mat.r * mat.c; i++)
    setMat(mat, 0, i, (float) rand() / (float) RAND_MAX * 2.f - 1.f);
}

void addMat(Mat dst, Mat val){
  if(dst.r != val.r || dst.c != val.c) return;
  #pragma omp parallel for
  for(unsigned long long i = 0; i < dst.r * dst.c; i++)
    dst.data[i] += val.data[i];
}

void subMat(Mat dst, Mat val){
  if(dst.r != val.r || dst.c != val.c) return;
  #pragma omp parallel for
  for(unsigned long long i = 0; i < dst.r * dst.c; i++)
    dst.data[i] -= val.data[i];
}

void scaleMat(Mat dst, float val){
  #pragma omp parallel for
  for(unsigned long long i = 0; i < dst.r * dst.c; i++)
    dst.data[i] *= val;
}

Mat composeMat(Mat a, Mat b){
  if(a.c != b.r) return (Mat){0, 0, 0};
  Mat mat = newMat(a.r, b.c);
  #pragma omp parallel for collapse(3)
  for(unsigned i = 0; i < a.r; i++){
    for(unsigned k = 0; k < a.c; k++){
      for(unsigned j = 0; j < b.c; j++){
        *viewMat(mat, i, j) += readMat(a, i, k) * readMat(b, k, j);
      }
    }
  }
  return mat;
}

void destroyMat(Mat *mat){
  free(mat->data);
  *mat = (Mat){0, 0, 0};
}

void freeMat(Mat mat){
  free(mat.data);
}

void overwriteMat(Mat *var, Mat val){
  free(var->data);
  *var = val;
}

Mat copyMat(Mat mat){
  Mat ret = newMat(mat.r, mat.c);
  #pragma omp parallel for
  for(size_t i = 0; i < mat.r * mat.c; i++)
    ret.data[i] = mat.data[i];
  return ret;
}

Mat transposeMat(Mat mat){
  Mat ret = newMat(mat.c, mat.r);
  #pragma omp parallel for collapse(2)
  for (unsigned i = 0; i < mat.r; i++)
    for (unsigned j = 0; j < mat.c; j++)
      *viewMat(ret, j, i) = readMat(mat, i, j);
  return ret;
}

Mat reduceMat(Mat mat, unsigned x, unsigned y){
  if(mat.r && mat.c); else return (Mat){0, 0, 0};
  Mat res = newMat(mat.r - 1, mat.c - 1);
  size_t count = 0;
  for(unsigned i = 0; i < mat.r; i++){
    if(i == y) continue;
    for(unsigned j = 0; j < mat.c; j++){
      if(j == x) continue;
      res.data[count] = readMat(mat, i, j);
      count++;
    }
  }
  return res;
}

float determinantMat(Mat mat){
  if(mat.r != mat.c) return (0.f / 0.f);
  if(mat.r && mat.c); else return (0.f / 0.f);
  if(mat.r == 1){
    return mat.data[0];
  }
  else if(mat.r == 2){
    return mat.data[0] * mat.data[3] - mat.data[1] * mat.data[2];
  }
  else if(mat.r == 3){
    return mat.data[0] * (mat.data[4] * mat.data[8] - mat.data[5] * mat.data[7])
    + mat.data[1] * (mat.data[5] * mat.data[6] - mat.data[3] * mat.data[8])
    + mat.data[2] * (mat.data[3] * mat.data[7] - mat.data[4] * mat.data[6]);
  }
  else{
    float coSum = 0.f;
    for(unsigned i = 0; i < mat.r; i++){
      Mat temp = reduceMat(mat, i, 0);
      coSum += mat.data[i] * (i & 1 ? -1.f : 1.f) * determinantMat(temp);
      free(temp.data);
    }
    return coSum;
  }
}

#endif