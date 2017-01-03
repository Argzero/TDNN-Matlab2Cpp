/*
The MIT License (MIT)

Copyright (c) <2016> <David Hasenfratz>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <cmath>
#include "ANNClassifier.h"

const ANNClassifier::XStep ANNClassifier::X1_STEP1 = {
  {50.910000, 17.650000, 2.520000, 0.860000, 4.140000, 21.570000, 1.140000, 20.250000},
  {0.040841, 0.026539, 0.100654, 0.350263, 0.095057, 0.025461, 0.095648, 0.025743},
  -1.000000
};

const ANNClassifier::YStep ANNClassifier::Y1_STEP1 = {
  {142.130000, 4.150000, 68.110000},
  {0.022321, 0.076104, 0.031284},
  -1.000000
};

const float ANNClassifier::B1[10] = {0.411719, -1.091386, 1.016100, -1.532488, 0.246091, 0.435082, -1.268810, -0.001410, -1.488810, 1.478952};

const float ANNClassifier::B2[3] = {0.030948, -0.422022, -0.010615};

const float ANNClassifier::IW1_1[10][16] = {
  {-1.214478, 0.374400, -2.182570, 2.563325, -1.813483, 0.166170, -2.203737, 0.552004, 1.017067, -1.302013, -0.251758, -1.461752, -1.556667, 2.331765, 1.283643, 0.764011},
  {-1.416854, -1.233415, -0.883536, 0.188696, 0.129540, 0.702876, -0.728673, -2.437714, 3.651181, 1.398519, -1.492363, 2.198497, 0.218151, -1.800522, 0.460244, 1.938270},
  {-2.334714, -0.164044, 1.710167, -1.715573, 0.900127, 0.568477, 1.061502, -3.095449, -1.139938, 1.529952, 3.465098, 1.050468, -3.260787, 0.268362, 0.661339, -1.724841},
  {-2.731784, -3.997628, -2.938573, 0.591751, 1.042991, 3.240724, -0.747951, -3.378777, 2.037092, 0.212810, 2.665564, 1.877719, -1.992264, -2.054345, -0.841785, -1.478597},
  {-1.574240, -0.328645, 1.526509, -1.112232, -0.125837, 0.811624, 1.275752, -1.238381, -0.634458, 1.474918, -1.326512, 0.297534, 1.607208, -0.174498, -0.661528, -0.304679},
  {-0.828968, -0.307173, 0.787203, -1.118451, 1.465562, -1.198788, 1.410893, -0.296002, 2.100882, 0.587224, -0.573841, -0.850187, -1.268305, 1.197422, 1.615667, -0.340317},
  {4.174870, 3.263583, -5.231398, -1.963547, -2.938670, -0.137040, 3.785208, 3.575626, -12.043460, -7.734929, -8.749428, -3.672548, 8.590045, -0.849435, -5.256290, 0.721626},
  {-0.370808, 0.362702, 0.205693, 0.187412, -1.198222, 0.126916, 0.205787, 0.872311, -1.224022, -0.156225, -0.453301, -0.315121, 1.590805, 0.018910, -0.127073, -0.065077},
  {0.976520, -0.191013, -0.599387, 0.101909, 0.775720, 1.442227, -1.341700, -1.530598, -0.712025, 0.736210, 0.964142, -0.741960, 0.187624, -0.197474, -0.707095, 0.461695},
  {-1.787534, -1.239868, 2.583828, 1.044912, 0.891758, 0.476821, -0.137084, 0.137567, 3.417002, 1.318696, -4.030314, 0.049679, -0.921661, 0.532001, -1.313248, 0.213289}};

const float ANNClassifier::LW2_1[3][10] = {
  {-0.218513, 0.085176, 0.288071, 0.122684, -0.520669, 0.335742, -0.103718, 0.992838, 0.329398, 0.182064},
  {-0.104422, 0.031508, 0.158022, 0.064670, -0.314734, 0.213805, 0.007881, 0.534151, 0.237800, 0.067351},
  {-0.209830, 0.093069, 0.268703, 0.112178, -0.454659, 0.141186, -0.108754, 0.767661, 0.218329, 0.170483}};

float *ANNClassifier::Predict(Data data) {
  static float prediction[3];
  float sum = 0.0f;

  for (int i = 0; i < 3; i++) {
    prediction[i] = 0.0f;
  }

  for (int i = 0; i < 8; i++) {
    data.value[i] = (data.value[i] - X1_STEP1.xoffset[i]) * X1_STEP1.gain[i] + X1_STEP1.ymin;
  }

  _num++;
  if (_num > 2) {

    for (int n = 0; n < 10; n++) {
      sum = 0.0f;
      for (int i = 2-1; i >= 0; i--) {
        Data temp = _input.get(i);
        for (int j = 0; j < 8; j++) {
          sum += temp.value[j] * IW1_1[n][((2-1)-i)*8+j];
        }
      }
      sum += B1[n];
      sum = 2 / (1 + exp(-2*sum)) - 1;

      for (int i = 0; i < 3; i++) {
        prediction[i] += sum * LW2_1[i][n];
      }
    }
    for (int i = 0; i < 3; i++) {
      prediction[i] += B2[i];
      prediction[i] = (prediction[i] - Y1_STEP1.ymin) / Y1_STEP1.gain[i] + Y1_STEP1.xoffset[i];
    }
  }

  _input.add(data);

  return prediction;
}
