#define ACTIVATIONFNIMPL
#define LAYERMODELIMPL
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "layerModel.h"

LayerModel loadLayerModel(FILE *file){
  LayerModel lm;
  fread(&lm.layerSize, sizeof(size_t), 1, file);
  fread(&lm.loss, sizeof(lm.loss), 1, file);
  lm.layer = malloc(sizeof(Layer) * lm.layerSize);
  for(size_t i = 0; i < lm.layerSize; i++){
    fread(&lm.layer[i].act, sizeof(enum LayerFunc), 1, file);
    fread(&lm.layer[i].weight.r, sizeof(unsigned), 1, file);
    fread(&lm.layer[i].weight.c, sizeof(unsigned), 1, file);
    lm.layer[i].weight.data = malloc(lm.layer[i].weight.r * lm.layer[i].weight.c * sizeof(float));
    fread(lm.layer[i].weight.data, sizeof(float), lm.layer[i].weight.r * lm.layer[i].weight.c, file);
    fread(&lm.layer[i].bias.r, sizeof(unsigned), 1, file);
    fread(&lm.layer[i].bias.c, sizeof(unsigned), 1, file);
    lm.layer[i].bias.data = malloc(lm.layer[i].bias.r * lm.layer[i].bias.c * sizeof(float));
    fread(lm.layer[i].bias.data, sizeof(float), lm.layer[i].bias.r * lm.layer[i].bias.c, file);
  }
  return lm;
}

int main(){
  FILE *f = fopen("Model.bin", "rb");
  if(!f){
    puts("File could not be opened");
    return 1;
  }
  LayerModel lm = loadLayerModel(f);
  fclose(f);

  Mat input = newMat(1, 15), output = {};

  while(1){

    outputLayerModel(lm, input, &output);
    
    printMat(output);
    Sleep(500);
  }
  
  
  destroyMat(&output);
  freeMat(input);
  freeLayerModel(lm);
}