#include <Windows.h>
#include <Mmsystem.h>
#include <chrono>
#include <fftw3.h>
#include <iostream>
#include <fstream>
#include <string>


#include "VisNode.hpp"
#include "EmitterTopNode.hpp"


#pragma comment(lib, "winmm.lib")

#define FFT_SIZE 4096
#define WINDOW_SIZE 1024
#define GAMMA 1000
#define AVE_WINDOW 9
#define PEAK_THRESH .05
#define GAMMA_PITCH 10


namespace GLOO {
  VisNode::VisNode(char* filename, double integration_step, double k, double drag, float attack, float transient_target) {

    SetupAudio(std::string(filename));
    next_peak_ = 0;

    auto poly_node = make_unique<PolyNode>(integration_step, k, drag, attack, transient_target, &bands_, &curr_ix_, &colors_);
    poly_node_ = poly_node.get();
    AddChild(std::move(poly_node));
    auto emitter = make_unique<EmitterTopNode>(integration_step, &pitches_, &curr_ix_, &colors_);
    emitter_node_ = emitter.get();
    AddChild(std::move(emitter));
    
    std::string name = std::string(filename);
    PlaySound(TEXT(filename), NULL, SND_FILENAME | SND_ASYNC);
    begin_ = Clock::now();

  }

  void VisNode::Update(double delta_time) {
    TimePoint now = Clock::now();
    double elapsed = (now - begin_).count();
    curr_ix_ = (elapsed * sample_rate_ + FFT_SIZE / 2) / WINDOW_SIZE;
    if (next_peak_ != peaks_.size() && curr_ix_ > peaks_[next_peak_]) {
      next_peak_++;
      poly_node_->TriggerTransients();
    }

  }

  void VisNode::SetupAudio(std::string filename) {
    std::ifstream f(filename, std::ifstream::in | std::ios_base::binary);

    void* buf = malloc(44);
    char* metadata = (char*)buf;

    f.read(metadata, 44);
    if (*(int*)(metadata) != 0x46464952) {
      printf("RIFF header invalid\n");
      exit(1);
    }
    if (*(int*)(metadata + 8) != 0x45564157) {
      printf("not wave file\n");
      exit(1);
    }
    if (*(char*)(metadata + 20) != 1) {
      printf("not PCM\n");
      exit(1);
    }
    sample_rate_ = (*(int*)(metadata + 24));
    printf("sample rate: %i\n", sample_rate_);

    if (*(char*)(metadata + 22) != 2) {
      printf("not stereo\n");
      exit(1);
    }

    if (*(char*)(metadata + 34) != 16) {
      printf("not 16bit depth\n");
      exit(1);
    }

    if (*(int*)(metadata + 36) != 0x61746164) {
      printf("very confused\n");
      exit(1);
    }
    int dataSize = *(int*)(metadata + 40);
    std::cout << "total dataSize " << dataSize << std::endl;
    buf = realloc(buf, FFT_SIZE * 2 * 2);
    float* converted_samples = fftwf_alloc_real(FFT_SIZE);
    fftwf_complex* frequencies = fftwf_alloc_complex(FFT_SIZE / 2 + 1);
    float* frequency_mags = (float*)malloc(sizeof(float) * FFT_SIZE);
    char* data = (char*)buf;
    int16_t* samples = (int16_t*)data;
    float* converted_samples_cpy = fftwf_alloc_real(FFT_SIZE);
    fftwf_plan plan = fftwf_plan_dft_r2c_1d(FFT_SIZE, converted_samples_cpy, frequencies, FFTW_MEASURE);


    std::vector<float> hann_buf = std::vector<float>(FFT_SIZE);
    std::vector<float> ave = std::vector<float>(AVE_WINDOW);
    float pi = glm::pi<float>();
    float bin_size = (float)sample_rate_ / FFT_SIZE;
    std::ofstream outFile;
    outFile.open("out.csv");
    for (int i = 0; i < FFT_SIZE; i++) {
      hann_buf[i] = .5 - .5 * glm::cos(2 * pi * i / (FFT_SIZE - 1));
    }
    float sum = 0;
    for (int i = 0; i < AVE_WINDOW; i++) {
      sum += .5 - .5 * glm::cos(2 * pi * i / (AVE_WINDOW - 1));
      ave[i] = .5 - .5 * glm::cos(2 * pi * i / (AVE_WINDOW - 1));
    }
    for (int i = 0; i < AVE_WINDOW; i++) {
      ave[i] /= sum;
    }

    uint16_t samples_copy[FFT_SIZE];
    f.read(data, (FFT_SIZE / 2 - WINDOW_SIZE) * 2 * 2);
    for (int i = 0; i < FFT_SIZE / 2 - WINDOW_SIZE; i++) {
      converted_samples[FFT_SIZE / 2 + i + WINDOW_SIZE] = (samples[2 * i] / 32768.f / 2 + samples[2 * i + 1] / 32768.f / 2);
    }

    float max_bass = 0;
    float max_melody = 0;
    float max_all = 0;

    int last = 0;
    int n = 0;
    std::vector<float> prev = std::vector<float>(FFT_SIZE / 2 + 1);
    std::vector<float> diffs;
    float max_band = 0;
    while (true) {
      if (n > sample_rate_*10 + last) {
        std::cout << n / 44100 << "seconds analyzed" << std::endl;
        last = n;
      }
      n += WINDOW_SIZE;
      f.read(data, WINDOW_SIZE * 2 * 2);
      int c = f.gcount();
      if (c != WINDOW_SIZE * 2 * 2) {
        std::cout << "got" << c << std::endl;
        break;
      }
      for (int i = WINDOW_SIZE; i < FFT_SIZE; i++) {
        converted_samples[i - WINDOW_SIZE] = converted_samples[i];
      }
      for (int i = 0; i < WINDOW_SIZE; i++) {
        converted_samples[FFT_SIZE - WINDOW_SIZE + i] = samples[2 * i] / 32768.f / 2 + samples[2 * i + 1] / 32768.f / 2;
      }
      for (int i = 0; i < FFT_SIZE; i++) {
        converted_samples_cpy[i] = hann_buf[i] * converted_samples[i];
      }
      fftwf_execute(plan);
      float cum_diff = 0;
      
      std::vector<float> slice = std::vector<float>(12);
      std::vector<float> pitch_slice = std::vector<float>(36);
      //all, bass, melody

      for (int pitch = 24; pitch < 120; pitch++) {
        float start_f = pow(2, ((float)pitch - .5 - 69) / 12)*440;
        float end_f = pow(2, ((float)pitch + .5 - 69) / 12) * 440;
        int low_bin = ceil(start_f / bin_size);
        int high_bin = floor(end_f / bin_size);
        for (int i = low_bin; i <= high_bin; i++) {
          float re = frequencies[i][0];
          float im = frequencies[i][1];
          pitch_slice[pitch % 12] += re * re + im * im;
          if (pitch < 60) pitch_slice[12 + (pitch % 12)] += re * re + im * im;
          if (pitch >= 60) pitch_slice[24 + (pitch%12)] += re * re + im * im;
          
        }
      }

      for (int i = 0; i < pitch_slice.size(); i++) {
        pitch_slice[i] = log10(1 + pitch_slice[i] * GAMMA_PITCH);
      }
      for (int i = 0; i < 12; i++) {
        if (pitch_slice[i] > max_all) max_all = pitch_slice[i];
      }
      for (int i = 12; i < 24; i++) {
        if (pitch_slice[i] > max_bass) max_bass = pitch_slice[i];
      }
      for (int i = 24; i < 36; i++) {
        if (pitch_slice[i] > max_melody) max_melody = pitch_slice[i];
      }

      pitches_.push_back(pitch_slice);


      for (int i = 0; i < FFT_SIZE / 2 + 1; i++) {
        float re = frequencies[i][0];
        float im = frequencies[i][1];
        frequency_mags[i] = log10(1 + (re * re + im * im) * GAMMA);

        //outFile << frequency_mags[i] << ",";
        if (i != 0) {
          cum_diff += frequency_mags[i] - prev[i] > 0 ? frequency_mags[i] - prev[i] : 0;
        }
        prev[i] = frequency_mags[i];

        float center_freq = i * bin_size;
        //add to ba
        if (center_freq < 20) continue;
        else if (center_freq < 200) slice[0] += frequency_mags[i] * 4;
        else if (center_freq < 400) slice[1] += frequency_mags[i] * 4;
        else if (center_freq < 630) slice[2] += frequency_mags[i] * 2.7;
        else if (center_freq < 920) slice[3] += frequency_mags[i] * 2.4;
        else if (center_freq < 1270) slice[4] += frequency_mags[i] * 2.1;
        else if (center_freq < 1720) slice[5] += frequency_mags[i] * 1.5;
        else if (center_freq < 2320) slice[6] += frequency_mags[i] * 1.3;
        else if (center_freq < 3150) slice[7] += frequency_mags[i];
        else if (center_freq < 4400) slice[8] += frequency_mags[i] * .7;
        else if (center_freq < 6400) slice[9] += frequency_mags[i] * .4;
        else if (center_freq < 9500) slice[10] += frequency_mags[i] * .3;
        else if (center_freq < 15500) slice[11] += frequency_mags[i] * .2;
      }
      //outFile << std::endl;
      bands_.push_back(slice);
      if (n != WINDOW_SIZE) diffs.push_back(cum_diff);

    }
    outFile.close();
    std::vector<float> diffs_convolved = std::vector<float>(diffs.size());
    for (int i = 0; i < diffs.size(); i++) {
      diffs_convolved[i] = 0;
      for (int j = -(AVE_WINDOW / 2); j < AVE_WINDOW / 2 + 1; j++) {
        if (i + j > 0 && (i + j < diffs.size())) diffs_convolved[i] += ave[j + AVE_WINDOW / 2] * diffs[i + j];
      }
      diffs_convolved[i] *= 1.02;

    }

    std::cout << diffs.size() << std::endl;
    float m = 0;
    for (int i = 0; i < diffs.size(); i++) {
      diffs[i] = (diffs[i] - diffs_convolved[i] > 0 ? diffs[i] - diffs_convolved[i] : 0);
      if (diffs[i] > m) m = diffs[i];
    }
    std::cout << "all: " << max_all << std::endl;
    std::cout << "bass: " << max_bass << std::endl;
    std::cout << "melody: " << max_melody << std::endl;
    for (int i = 0; i < diffs.size(); i++) {
      diffs[i] /= m;
      if (i > 1 && diffs[i - 1] - diffs[i - 2] > 0 && diffs[i] - diffs[i - 1] < 0 && diffs[i - 1] > PEAK_THRESH) peaks_.push_back(i - 1);
    }

  }
}