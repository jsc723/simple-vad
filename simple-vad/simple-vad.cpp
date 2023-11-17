// simple-vad.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>
#include <cstdio>
#include <cmath>
#include <complex>
#include <list>
#include <iterator>
#include <random>
#include <queue>
#include <unordered_set>
#include "fvad.h"
#include "fftw3.h"
#include "samplerate.h"


using namespace std;

struct WavHeader {
    char riff[4];
    uint32_t fileSize;
    char wave[4];
    char fmt[4];
    uint32_t fmtSize;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
};

const size_t windowDurationMS = 20; //ms
const size_t windowSize = windowDurationMS * 8; // num of sample (short) per window
const size_t windowSizeByte = windowSize * sizeof(short);

WavHeader CreateWavHeaderTemplate() {
    WavHeader h;
    memcpy(h.riff, "RIFF", 4);
    memcpy(h.wave, "WAVE", 4);
    memcpy(h.fmt, "fmt ", 4);
    h.fmtSize = 16;
    return h;
}

void writeWavHeader(std::ofstream& file, const WavHeader& header) {
    file.write(reinterpret_cast<const char*>(&header), sizeof(WavHeader));
}

void writeChannelToWAV(const std::string fileName, const vector<char> &data) {
    // Save channel1Data to a binary file
    std::ofstream outputFile(fileName, std::ios::binary);
    if (!outputFile.is_open()) {
        std::cerr << "Error opening output file." << std::endl;
        exit(1);
    }

    WavHeader header = CreateWavHeaderTemplate();
    header.fileSize = sizeof(WavHeader) + 8 + data.size() - 8;
    header.audioFormat = 1;
    header.numChannels = 1;
    header.sampleRate = 8000;
    header.byteRate = 16000;
    header.blockAlign = 4;
    header.bitsPerSample = 16;

    outputFile.write(reinterpret_cast<const char*>(&header), sizeof(WavHeader));
    outputFile.write("data", 4);
    size_t sz = data.size();
    outputFile.write(reinterpret_cast<const char*>(&sz), 4);
    outputFile.write(data.data(), data.size());
    outputFile.close();
}

struct Segment {
    int start;
    int length;
    int id;
    int end() {
        return start + length;
    }
    Segment(int start, int length, int id) : start(start), length(length), id(id) {}
};


struct PQItemData {
    int leftId, rightId;
    list<Segment>::iterator left;
    list<Segment>::iterator right;
};
struct PQItem {
    int cost;
    PQItemData* data;
    PQItem(int cost, PQItemData* data) : cost(cost), data(data) {}
};

void postProcess(vector<int>& result) {
    const int minDurationMS = 1000;
    const int minLen = minDurationMS / windowDurationMS; // 1 len unit = 20 ms
    const int thresh = 500;
    
    list<Segment> segs;
    int segNextId = 0;
    Segment cur(0, 0, segNextId++);
    for (int i = 0; i < result.size(); i++) {
        if (result[i]) {
            if (cur.length == 0) {
                cur.start = i;
            }
            cur.length++;
        }
        else {
            if (cur.length > 0) {
                segs.push_back(cur);
                cur = Segment(0, 0, segNextId++);
            }
        }
    }
    int merged = 0;
    printf("size of segs %d\n", segs.size());
    auto cmp = [](const PQItem& p, const PQItem& q) {
        if (p.cost != q.cost) {
            return p.cost > q.cost;
        }
        if (p.data->leftId != q.data->leftId) {
            return p.data->leftId < q.data->leftId;
        }
        return p.data->rightId < q.data->rightId;
    };
    
    priority_queue<PQItem, vector<PQItem>, decltype(cmp)> pq(cmp);


    auto computeCost = [](list<Segment>::iterator& left, list<Segment>::iterator& right) {
        int d = right->start - left->end();
        if (d < 10) {
            return 0;
        }
        return min(left->length, right->length) * (right->start - left->end());
    };

    for (auto it = segs.begin(); it != segs.end() && std::next(it) != segs.end(); it++) {
        auto nxt = std::next(it);
        const int cost = computeCost(it, nxt);
        if (cost < thresh) {
            auto data = new PQItemData();
            data->left = it;
            data->right = nxt;
            data->leftId = it->id;
            data->rightId = nxt->id;
            //printf("push it=%d[%d %d], nxt=%d[%d, %d], cost=%d\n", data->leftId, it->start, it->end(), data->rightId, nxt->start, nxt->end(), cost);
            pq.emplace(cost, data);
        }
    }

    unordered_set<int> removedIds;
    while (!pq.empty() && segs.size() > 1) {
        auto t = pq.top();
        pq.pop();
        //printf("pull it=%d[%d %d], nxt=%d[%d, %d], cost=%d\n", t.data->leftId, t.data->left->start, t.data->left->end(),
        //    t.data->rightId, t.data->right->start, t.data->right->end(), t.cost);
        if (t.cost > thresh) {
            delete t.data;
            t.data = nullptr;
            continue;
        }
        if (removedIds.count(t.data->leftId) || removedIds.count(t.data->rightId)) {
            //printf("ignore\n");
            delete t.data;
            t.data = nullptr;
            continue;
        }
        merged++;
        //printf("merged\n");
        for (int k = t.data->left->end(); k < t.data->right->start; k++) {
            result[k] = 1;
        }
        t.data->left->length = t.data->right->end() - t.data->left->start;
        removedIds.emplace(t.data->rightId);
        removedIds.emplace(t.data->leftId);
        segs.erase(t.data->right);
        t.data->left->id = segNextId++;
        t.data->leftId = t.data->left->id;
        
        auto it = t.data->left;

        if (it != segs.begin()) {
            auto prv = std::prev(it);
            int cost = computeCost(prv, it);
            if (cost < thresh) {
                auto data = new PQItemData();
                data->left = prv;
                data->right = it;
                data->leftId = prv->id;
                data->rightId = it->id;
                pq.emplace(cost, data);
                //printf("push it=%d[%d %d], nxt=%d[%d, %d], cost=%d\n", prv->id, prv->start, prv->end(), it->id, it->start, it->end(), cost);
            }
        }

        if (std::next(it) != segs.end()) {
            auto nxt = std::next(it);
            int cost = computeCost(it, nxt);
            if (cost < thresh) {
                auto data = new PQItemData();
                data->left = it;
                data->right = nxt;
                data->leftId = it->id;
                data->rightId = nxt->id;
                pq.emplace(cost, data);
                //printf("push it=%d[%d %d], nxt=%d[%d, %d], cost=%d\n", it->id, it->start, it->end(), nxt->id, nxt->start, nxt->end(), cost);
            }
        }
        
    }
    while (pq.size()) {
        delete pq.top().data;
        pq.pop();
    }

    for (auto it = segs.begin(); it != segs.end() && std::next(it) != segs.end(); it++) {
        auto nxt = std::next(it);
        int dist = nxt->start - it->end();
        if (dist < 10) {
            printf("bug it=%d[%d %d], nxt=%d[%d, %d]\n", it->id, it->start, it->end(), nxt->id, nxt->start, nxt->end());
        }
    }

    //for (auto it = segs.begin(); it != segs.end() && std::next(it) != segs.end(); it++) {
    //    auto nxt = std::next(it);
    //    int leftLen = it->length;
    //    int rightLen = nxt->length;
    //    int dist = (nxt->start - it->end());
    //    if (min(leftLen, rightLen) * dist < 500 || dist < 5) {
    //        merged++;
    //        for (int k = it->end(); k < nxt->start; k++) {
    //            result[k] = 1;
    //        }
    //        it->length = nxt->end() - it->start;
    //        segs.erase(nxt);
    //    }
    //}


    for (auto it = segs.begin(); it != segs.end(); it++) {
        while (it->length < minLen) {
            auto nxt = std::next(it);
            auto prv = std::prev(it);
            if (it != segs.begin() && it->start - prv->end() < 10) {
                merged++;
                for (int k = prv->end(); k < it->start; k++) {
                    result[k] = 1;
                }
                int it_end = it->end();
                it->start = prv->start;
                it->length = it_end - prv->start;
                segs.erase(prv);
            }
            else if (nxt != segs.end() && nxt->start - it->end() <= minLen - it->length) {
                merged++;
                for (int k = it->end(); k < nxt->start; k++) {
                    result[k] = 1;
                }
                it->length = nxt->end() - it->start;
                segs.erase(nxt);
            } 
            else {
                for (int k = it->end(); k < it->start + minLen; k++) {
                    result[k] = 1;
                }
                it->length = minLen;
            }
        }
    }
    printf("merged %d\n", merged);


    if (result.back() == 1) {
        result.back() = 0;
    }
}


// Function to apply FFT using FFTW
void applyFFT(const std::vector<double>& input, std::vector<std::complex<double>>& output) {
    fftw_plan plan = fftw_plan_dft_r2c_1d(input.size(), const_cast<double*>(input.data()), reinterpret_cast<fftw_complex*>(output.data()), FFTW_MEASURE);
    fftw_execute(plan);
    fftw_destroy_plan(plan);
}

// Function to apply Inverse FFT using FFTW
void applyInverseFFT(const std::vector<std::complex<double>>& input, std::vector<double>& output) {
    fftw_plan plan = fftw_plan_dft_c2r_1d(output.size(), reinterpret_cast<fftw_complex*>(const_cast<std::complex<double>*>(input.data())), output.data(), FFTW_MEASURE);
    fftw_execute(plan);
    fftw_destroy_plan(plan);

    // Normalize the output
    for (auto& val : output) {
        val /= input.size();
    }
}

// Function to calculate Short-Time Fourier Transform (STFT)
void doFiltering(std::vector<double>& input, size_t windowSize, size_t hopSize) {
    size_t signalSize = input.size();
    size_t halfWindowSize = windowSize / 2;
    size_t numFrames = (signalSize - windowSize) / hopSize + 1;
    const int freqSize = (windowSize / 2 + 1);
    const int sampleResolution = 8000 / windowSize;
    const int padding = windowSize / hopSize;
    const int minFreq = 0;
    const int maxFreq = 8000;
    const double energyThreshold = 0.001;

    std::vector<double> output(input.size());
    std::vector<std::complex<double>> freq(freqSize);

    // Apply FFT to each frame
    for (size_t i = padding; i < numFrames - padding; ++i) {
        std::vector<double> frame(windowSize);
        std::vector<double> outframe(windowSize);

        // Extract the frame from the input signal
        std::copy(input.begin() + i * hopSize - halfWindowSize, input.begin() + i * hopSize + halfWindowSize, frame.begin());

        double energy = 0;
        for (auto sample : frame) {
            energy += sample * sample;
        }
        energy /= frame.size();
        if (energy < energyThreshold) {
            std::copy(outframe.begin(), outframe.begin() + hopSize, output.begin() + i * hopSize);
            continue;
        }


        applyFFT(frame, freq);

        
        for (int j = 0; j < freqSize; j++) {
            int f = j * sampleResolution;
            freq[j] = (f >= minFreq && f <= maxFreq) ? freq[j] : 0;
        }

        applyInverseFFT(freq, outframe);
        

        std::copy(outframe.begin(), outframe.begin() + hopSize, output.begin() + i * hopSize);
    }
    input = move(output);
}

void preProcess(int16_t* buf, int buflen) {
    vector<double> dbuf(buflen);
    for (int i = 0; i < buflen; i++) {
        dbuf[i] = buf[i] / 32768.0;
    }
    doFiltering(dbuf, 160, 80);
    for (int i = 0; i < buflen; i++) {
        buf[i] = dbuf[i] * 32768;
    }
}

#include <samplerate.h>
#include <vector>

// Resample audio data from sourceSampleRate to targetSampleRate
std::vector<short> resampleAudio(const std::vector<short>& input, int sourceSampleRate, int targetSampleRate) {
    // Calculate the ratio between the source and target sample rates
    double ratio = static_cast<double>(targetSampleRate) / static_cast<double>(sourceSampleRate);

    // Set up the src_data structure
    SRC_DATA data;
    float* tmp_data_in = new float[input.size()];
    for (size_t i = 0; i < input.size(); ++i) {
        tmp_data_in[i]  = static_cast<float>(input[i]) / 32768.0f;
    }
    data.data_in = tmp_data_in;
    data.input_frames = static_cast<long>(input.size());
    data.src_ratio = ratio;

    // Calculate the output size
    long outputSize = static_cast<long>(input.size() * ratio) + 1; // Add 1 for safety

    // Allocate memory for the output data
    std::vector<short> output(outputSize);

    data.data_out = new float[outputSize];
    data.output_frames = outputSize;

    // Perform the actual resampling
    int error = src_simple(&data, 0, 1);

    if (error != 0) {
        // Handle error (you can check the error code in the libsamplerate documentation)
        // For simplicity, we print the error code here
        std::cerr << "libsamplerate error: " << error << std::endl;
        delete[] data.data_in;
        delete[] data.data_out;
        return std::vector<short>(); // Return an empty vector in case of an error
    }

    // Convert the float output data back to short
    for (size_t i = 0; i < output.size(); ++i) {
        output[i] = static_cast<short>(data.data_out[i] * 32768.0f);
    }

    // Clean up allocated memory
    delete[] data.data_in;
    delete[] data.data_out;

    return output;
}



int main()
{
    const char* filename = "data/test-yuihi-8k.wav";
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return 1;
    }

    WavHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(WavHeader));

    // Check if it's a valid WAV file
    if (strncmp(header.riff, "RIFF", 4) != 0 || strncmp(header.wave, "WAVE", 4) != 0) {
        std::cerr << "Not a valid WAV file: " << filename << std::endl;
        return 1;
    }


    // Print the WAV header details
    std::cout << "File Size: " << header.fileSize << " bytes" << std::endl;
    std::cout << "Audio Format: " << header.audioFormat << std::endl;
    std::cout << "Number of Channels: " << header.numChannels << std::endl;
    std::cout << "Sample Rate: " << header.sampleRate << " Hz" << std::endl;
    std::cout << "Byte Rate: " << header.byteRate << " bytes/sec" << std::endl;
    std::cout << "Block Align: " << header.blockAlign << " bytes" << std::endl;
    std::cout << "Bits Per Sample: " << header.bitsPerSample << " bits" << std::endl;

    if (header.numChannels != 2) {
        std::cerr << "num of channels must be 2" << filename << std::endl;
        return 1;
    }
    
    char chunkID[4];
    memset(chunkID, 0, sizeof(chunkID));

    file.read(chunkID, 4);
    if (memcmp(chunkID, "LIST", 4) == 0) {
        uint32_t sz;
        file.read((char *)&sz, sizeof(sz));
        file.seekg(sz, std::ios::cur);
    }
    else {
        file.seekg(-4, std::ios::cur);
    }

    file.read(chunkID, 4);
    if (memcmp(chunkID, "data", 4) != 0) {
        std::cerr << "cannot find data chunk" << std::endl;
        return 1;
    }

    // fileSize + 8 = dataSampleStart + dataSize
    uint32_t dataSize;
    file.read((char*)&dataSize, sizeof(dataSize));


    std::vector<char> audioData(dataSize);
    file.read(audioData.data(), audioData.size());

    int bytesPerSample = header.bitsPerSample / 8;

    std::cout << "Data size: " << dataSize << " bits" << std::endl;

    // Separate the interleaved channels
    std::vector<char> channel1Data(dataSize / 2);
    std::vector<char> channel2Data(dataSize / 2);
    

    cout << "copying channel data..." << endl;

    for (size_t i = 0; i < audioData.size(); i += header.numChannels * bytesPerSample) {
        // Copy left channel sample
        std::copy(audioData.begin() + i, audioData.begin() + i + bytesPerSample, channel1Data.begin() + (i / 2));
        // Copy right channel sample
        std::copy(audioData.begin() + i + bytesPerSample, audioData.begin() + i + header.numChannels * bytesPerSample, channel2Data.begin() + (i / 2));
    }

    
    //cout << "writting channel 1 data..." << endl;
    //writeChannelToWAV("data/channel1.wav", channel1Data);

    //cout << "writting channel 2 data..." << endl;
    //writeChannelToWAV("data/channel2.wav", channel2Data);


    Fvad* vad;
    vad = fvad_new();
    if (!vad) {
        std::cout << "error init";
    }
    fvad_set_mode(vad, 1);
    fvad_set_sample_rate(vad, 8000);


    const size_t resultLen = channel1Data.size() / windowSizeByte;
    vector<int> result1(resultLen);
    vector<int> result2(resultLen);
    vector<int> result(resultLen);

    int16_t * data1 = (int16_t *)channel1Data.data();
    int16_t * data2 = (int16_t *)channel2Data.data();

    preProcess(data1, channel1Data.size() / 2);
    preProcess(data2, channel2Data.size() / 2);

    cout << "writting channel 1 data..." << endl;
    writeChannelToWAV("data/channel1_filtered.wav", channel1Data);

    for (size_t i = 0; i < resultLen; i++) {
        result1[i] = fvad_process(vad, data1 + i * windowSize, windowSize);
    }
    for (size_t i = 0; i < resultLen; i++) {
        result2[i] = fvad_process(vad, data2 + i * windowSize, windowSize);
        result[i] = (result1[i] == 1 || result2[i] == 1);
    }

    const int resPerSec = 1000 / windowDurationMS;
    postProcess(result);
    for (int sec = 0; sec < 30; sec++) {
        printf("%2d ", sec);
        for (int i = 0; i < resPerSec; i++) {
            printf("%d", result[sec * resPerSec + i]);
        }
        printf("\n");
    }

    channel1Data.clear();
    channel2Data.clear();
    fvad_free(vad);

    

    vector<bool> resultDelta(result.size() + 1);
    vector<string> timeStamps;
    char tmbuf[128];
    for (int i = 0; i < resultDelta.size(); i++) {
        resultDelta[i] = result[i] != result[i + 1];
        if (resultDelta[i]) {
            int totalMS = windowDurationMS * i;
            int totalSec = totalMS / 1000;
            int ms = totalMS % 1000;
            int s = totalSec % 60;
            int m = (totalSec % 3600) / 60;
            int h = totalSec / 3600;
            sprintf(tmbuf, "%02d:%02d:%02d,%03d", h, m, s, ms);
            timeStamps.push_back(string(tmbuf));
        }
    }


    FILE* outsrt = fopen("output.srt", "w");
    if (!outsrt) {
        std::cerr << "Error opening the output.txt." << std::endl;
        return 1; 
    }
    for (int i = 0; i + 1 < timeStamps.size(); i += 2) {
        fprintf(outsrt, "%d\n", i / 2 + 1);
        fprintf(outsrt, "%s --> %s\nN\n\n", timeStamps[i].c_str(), timeStamps[i + 1].c_str());
    }
    fprintf(outsrt, "\n");
    


    std::cout << "Done" << std::endl;
}
