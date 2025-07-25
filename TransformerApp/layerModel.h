#ifndef LAYERMODELH
#define LAYERMODELH

#include "activationFn.h"

struct Layer;
typedef struct Layer Layer;
Layer newLayer(unsigned inp, unsigned out);
void outputLayer(Layer, Mat in, Mat *out);
void freeLayer(Layer);
struct LayerModel;
typedef struct LayerModel LayerModel;
struct LayerData;
typedef struct LayerData LayerData;
LayerModel newLayerModel(size_t layers, unsigned inp, ...);
void outputLayerModel(LayerModel, Mat in, Mat *out);
float costLayerModel(LayerModel, LayerData);
void trainLayerModel(LayerModel, LayerData, float eps, float rate);
void freeLayerModel(LayerModel);
LayerData newLayerData(LayerModel, unsigned exampleAmount);
void fillLayerData(LayerData, ...);
void freeLayerData(LayerData);

#endif

#ifdef LAYERMODELIMPL

struct Layer{
  Mat weight, bias;
  enum LayerFunc act;
};

Layer newLayer(unsigned inp, unsigned out){
  Layer ly = {newMat(inp, out), newMat(1, out), EnumLayerLinear};
  randMat(ly.weight);
  randMat(ly.bias);
  return ly;
}

void outputLayer(Layer ly, Mat in, Mat *out){
  overwriteMat(out, composeMat(in, ly.weight));
  addMat(*out, ly.bias);
  if (ly.act) LayerFuncList[ly.act](*out);
}

void freeLayer(Layer ly){
  freeMat(ly.weight);
  freeMat(ly.bias);
}

struct LayerModel{
  size_t layerSize;
  Layer *layer;
  enum LossFunc loss;
};

struct LayerData{
  Mat input, output;
};

LayerModel newLayerModel(size_t layers, unsigned inp, ...){
  LayerModel lm = {layers, malloc(layers * sizeof(Layer)), EnumLossSquared};
  va_list args;
  va_start(args, inp);
  for(size_t i = 0; i < layers; i++){
    unsigned next = va_arg(args, unsigned);
    lm.layer[i] = newLayer(inp, next);
    inp = next;
  }
  va_end(args);
  return lm;
}

void outputLayerModel(LayerModel lm, Mat in, Mat *out){
  overwriteMat(out, newMat(in.r, lm.layer[lm.layerSize - 1ull].weight.c));
  Mat subOut = {};
  for(unsigned u = 0; u < in.r; u++){
    Mat subIn = {1, in.c, viewMat(in, u, 0)};
    outputLayer(lm.layer[0], subIn, &subOut);
    for (size_t i = 1; i < lm.layerSize; i++){
      outputLayer(lm.layer[i], subOut, &subOut);
    }
    for(unsigned v = 0; v < subOut.c; v++)
      setMat(*out, u, v, subOut.data[v]);
  }
  freeMat(subOut);
}

float costLayerModel(LayerModel lm, LayerData ld){
  Mat aux = {};
  outputLayerModel(lm, ld.input, &aux);
  float total = LossFuncList[lm.loss](aux, ld.output);
  freeMat(aux);
  return total / ld.input.r;
}

void trainLayerModel(LayerModel lm, LayerData ld, float eps, float rate){
  float original, dcost;
  for (size_t l = 0; l < lm.layerSize; ++l) {
    Layer ly = lm.layer[l];
    for (size_t i = 0; i < ly.weight.r * ly.weight.c; i++){
      original = ly.weight.data[i];
      ly.weight.data[i] -= eps;
      dcost = costLayerModel(lm, ld);
      ly.weight.data[i] = original + eps;
      dcost = costLayerModel(lm, ld) - dcost;
      dcost /= 2.f * eps;
      ly.weight.data[i] = original - rate * dcost;
    }
    for (unsigned j = 0; j < ly.bias.c; j++){
      original = ly.bias.data[j];
      ly.bias.data[j] -= eps;
      dcost = costLayerModel(lm, ld);
      ly.bias.data[j] = original + eps;
      dcost = costLayerModel(lm, ld) - dcost;
      dcost /= 2.f * eps;
      ly.bias.data[j] = original - rate * dcost;
    }
  }
}

void freeLayerModel(LayerModel lm){
  for(size_t i = 0; i < lm.layerSize; i++)
    freeLayer(lm.layer[i]);
  free(lm.layer);
}

LayerData newLayerData(LayerModel lm, unsigned size){
  LayerData ld = {
    newMat(size, lm.layer[0].weight.r),
    newMat(size, lm.layer[lm.layerSize - 1].weight.c)
  };
  return ld;
}

void fillLayerData(LayerData ld, ...){
  unsigned ic = ld.input.c, oc = ld.output.c;
  va_list args;
  va_start(args, ld);
  for (unsigned i = 0; i < ld.input.r; i++) {
    for (unsigned j = 0; j < ic; j++)
      setMat(ld.input, i, j, va_arg(args, int));
    for (unsigned j = 0; j < oc; j++)
      setMat(ld.output, i, j, va_arg(args, int));
  }
  va_end(args);
}

void fillfLayerData(LayerData ld, ...){
  unsigned ic = ld.input.c, oc = ld.output.c;
  va_list args;
  va_start(args, ld);
  for (unsigned i = 0; i < ld.input.r; i++) {
    for (unsigned j = 0; j < ic; j++)
      setMat(ld.input, i, j, va_arg(args, double));
    for (unsigned j = 0; j < oc; j++)
      setMat(ld.output, i, j, va_arg(args, double));
  }
  va_end(args);
}

void freeLayerData(LayerData ld){
  freeMat(ld.input);
  freeMat(ld.output);
}

#endif