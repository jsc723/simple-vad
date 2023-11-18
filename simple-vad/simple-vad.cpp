#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <complex>
#include <list>
#include <iterator>
#include <random>
#include <queue>
#include <unordered_set>
#include <algorithm>
#include <windows.h>
#include "fvad.h"
#include "fftw3.h"
#include "samplerate.h"
#include "args_parser.h"


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
const size_t hopSize = windowSize / 2;
const size_t windowSizeByte = windowSize * sizeof(short);
const size_t targetSampleRate = 8000;
const double minValidFrameInSegmentRatio = 0.3;

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

void writeChannelToWAV(const std::string fileName, const vector<int16_t> &data) {
    int byteSize = data.size() * 2;
    // Save channel1Data to a binary file
    std::ofstream outputFile(fileName, std::ios::binary);
    if (!outputFile.is_open()) {
        std::cerr << "Error opening output file." << std::endl;
        exit(1);
    }

    WavHeader header = CreateWavHeaderTemplate();
    header.fileSize = sizeof(WavHeader) + 8 + byteSize - 8;
    header.audioFormat = 1;
    header.numChannels = 1;
    header.sampleRate = 8000;
    header.byteRate = 16000;
    header.blockAlign = 4;
    header.bitsPerSample = 16;

    outputFile.write(reinterpret_cast<const char*>(&header), sizeof(WavHeader));
    outputFile.write("data", 4);
    size_t sz = byteSize;
    outputFile.write(reinterpret_cast<const char*>(&sz), 4);
    outputFile.write((char *)data.data(), byteSize);
    outputFile.close();
}

struct UserParameters {
    string outputFilename = "output.srt";

    bool useFiltering = true;
    int minFreq = 0;
    int maxFreq = targetSampleRate;
    double energyThreshold = 0.0;
    string filterOutputFile = "";

    int vadMode = 1; //0-3

    double minValidTopFreqEnergyRatio = 0.85;
    int mergeThreshold = 500;
    int minValidDuration = 1000; //ms
    int minGapDuration = 200; //ms
    int startMargin = 0; //ms
    int endMargin = 100; //ms
};

struct FreqInfo {
    double totalEnergy = 0.0;
    double topEnergy = 0.0;
};

void showHelpPage()
{
    cout << "Simple Voice Activity Detector by @jsc723 - version 1.3.0 - 2023\n\n";
    cout << "Usage: ./simple-vad.exe [options] <input_file>" << endl;
    cout << "-o FILENAME               : specify the name of the output subtitle file, default output.srt\n\n";
    
    cout << "--no-filtering            : do not perform filtering, default disabled\n";
    cout << "--min-freq  INT           : filter out all frequency below this number, default 0\n";
    cout << "--max-freq  INT           : filter out all frequency above this number, default 8000\n";
    cout << "--energy-threshold FLOAT  : filter out all sound frame whose energy is below this number, default 0.001\n";
    cout << "--out-filtered FILENAME   : output the audio after filtering, before it is passed to vad, no output by default\n\n";

    cout << "--vad-mode INT            : set the mode for the fvad library, larger => less chance to be classified as voice,\n";
    cout << "                            range from 0-3, default 1\n\n";
    cout << "--merge-threshold INT     : merge small and close voice segments, larger => merge more, default 500\n";
    cout << "--min-valid-duration INT  : merge or extend the voice segments shorter than this number, default 1000(ms)\n";
    cout << "--min-gap-duration INT    : merge the gaps between voice segments shorter than this number, default 200(ms)\n\n";
    cout << "--min-clear-ratio FLOAT   : range 0-1, remove all segments which doesn't have enough portion of frames\n";
    cout << "                            with a top frequency bin energy to total energy above this number, default 0.85";
    cout << "--start-margin INT        : add a margin before each segment, default 0(ms)\n";
    cout << "--end-margin INT          : add a margin after each segment, default 100(ms)\n";
    cout << "-h                        : show this help page";
    cout << endl;
}



struct Segment {
    int start;
    int length;
    int id;
    int extended;
    int end() {
        return start + length;
    }
    Segment(int start, int length, int id) : start(start), length(length), id(id), extended(0) {}
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

//result and info must have the same length
void postProcess(vector<int>& result, const UserParameters &params, vector<FreqInfo> &info) {
    const int minDurationMS = params.minValidDuration;
    const int minLen = minDurationMS / windowDurationMS; // 1 len unit = 20 ms
    const int thresh = params.mergeThreshold;
    const int minGapFrame = params.minGapDuration / 20;

    std::cout << "--- post process ---" << "\n";
    std::cout << "minValidDuration: " << minDurationMS << "ms" << "\n";
    std::cout << "mergeThreshold: " << thresh << "\n";
    std::cout << "minGapDuration: " << params.minGapDuration << "ms" << "\n";
    std::cout << "minGapFrame: " << minGapFrame << std::endl;

    
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

    std::cout << "min clear ratio = " << params.minValidTopFreqEnergyRatio << "\n";
    int removedSeg = 0;
    for (auto it = segs.begin(); it != segs.end(); ++it) {
        if (it->length == 0) {
            continue;
        }
        int validCount = 0;
        for (int k = it->start; k < it->end(); k++) {
            double ratio = (double)info[k].topEnergy / (info[k].totalEnergy + 1e-9);
            if (ratio >= params.minValidTopFreqEnergyRatio) {
                validCount++;
            }
        }
        if ((double)validCount / it->length < minValidFrameInSegmentRatio) {
            for (int k = it->start; k < it->end(); k++) {
                result[k] = 0;
            }
            removedSeg++;
            segs.erase(it);
        }
    }
    cout << "post process: removed " << removedSeg << " segments\n";

    int merged = 0;
    cout << "post process: segments count = " << segs.size() << "\n";
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


    auto computeCost = [minGapFrame](list<Segment>::iterator& left, list<Segment>::iterator& right) {
        int d = right->start - left->end();
        if (d < minGapFrame) {
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
                    it->extended++;
                }
                it->length = minLen;
            }
        }
    }

    cout << "post process: merged " << merged << " segments" << endl;

    const int endMarginLen = params.endMargin / windowDurationMS;
    const int startMarginLen = params.startMargin / windowDurationMS;
    const int minGap = 2;

    for (auto it = segs.begin(); it != segs.end() && next(it) != segs.end(); it++) {
        auto nxt = next(it);
        int i;
        for (i = it->end(); it->extended < endMarginLen && i + minGap + startMarginLen < nxt->start; i++) {
            result[i] = 1;
            it->extended++;
        }
        it->length = i - it->start;
    }

    segs.push_front(Segment(0, 0, -1)); // dummy header, so the first segment won't be a special case

    for (auto it = segs.begin(); it != segs.end() && next(it) != segs.end(); it++) {
        auto nxt = next(it);
        const int wantedNewStart = nxt->start - startMarginLen;
        const int originalEnd = nxt->end();
        int i;
        for (i = nxt->start - 1; i >= 0 && i > it->end() + minGap && i > wantedNewStart; i--) {
            result[i] = 1;
            nxt->extended++;
        }
        nxt->start = i + 1;
        nxt->length = originalEnd - nxt->start;
    }

    segs.pop_front(); //delete dummy header


    if (result.back() == 1) {
        result.back() = 0;
    }
}


// Function to apply FFT using FFTW
void applyFFT(const std::vector<double>& input, std::vector<std::complex<double>>& output) {
    fftw_plan plan = fftw_plan_dft_r2c_1d(input.size(), const_cast<double*>(input.data()), 
        reinterpret_cast<fftw_complex*>(output.data()), FFTW_MEASURE);
    fftw_execute(plan);
    fftw_destroy_plan(plan);
}

// Function to apply Inverse FFT using FFTW
void applyInverseFFT(const std::vector<std::complex<double>>& input, std::vector<double>& output) {
    fftw_plan plan = fftw_plan_dft_c2r_1d(output.size(), 
        reinterpret_cast<fftw_complex*>(const_cast<std::complex<double>*>(input.data())), output.data(), FFTW_MEASURE);
    fftw_execute(plan);
    fftw_destroy_plan(plan);

    // Normalize the output
    for (auto& val : output) {
        val /= input.size();
    }
}


// Function to calculate Short-Time Fourier Transform (STFT)
void doFiltering(std::vector<double>& input, size_t windowSize, size_t hopSize, const UserParameters &params, vector<FreqInfo>& freqInfos) {
    cout << "--- filtering ---\n";
    cout << "use filtering = " << params.useFiltering << "\n";
    if (!params.useFiltering) {
        return;
    }

    cout << "min freq = " << params.minFreq << "\n";
    cout << "max freq = " << params.maxFreq << "\n";
    cout << "energy threshold = " << params.energyThreshold << endl;

    size_t signalSize = input.size();
    size_t halfWindowSize = windowSize / 2;
    size_t numFrames = signalSize / hopSize;
    const int freqSize = (windowSize / 2 + 1);
    const int sampleResolution = targetSampleRate / windowSize;

    std::vector<double> output(input.size());
    std::vector<std::complex<double>> freq(freqSize);

    std::vector<double> frame(windowSize);
    std::vector<double> outframe(windowSize);

    // Apply FFT to each frame
    for (size_t i = 1; i < numFrames; ++i) {
        // Extract the frame from the input signal
        std::copy(input.begin() + i * hopSize - halfWindowSize, input.begin() + i * hopSize + halfWindowSize, frame.begin());

        double energy = 0;
        for (auto sample : frame) {
            energy += sample * sample;
        }
        
        if (energy < params.energyThreshold) {
            std::fill(output.begin() + i * hopSize, output.begin() + i * hopSize + hopSize, 0);
            continue;
        }

        applyFFT(frame, freq);

        for (int j = 0; j < freqSize; j++) {
            int f = j * sampleResolution;
            freq[j] = (f >= params.minFreq && f <= params.maxFreq) ? freq[j] : 0;
        }

        double energyFreq = 1e-9;
        vector<double> binEnergy(freq.size());
        for (int i = 0; i < freq.size(); i++) {
            double norm = std::abs(freq[i]);
            double e = norm * norm;
            binEnergy[i] = e / freq.size();
            energyFreq += e;
        }
        energyFreq /= freq.size(); //a little smaller than energy due to loss during fft
        std::sort(binEnergy.rbegin(), binEnergy.rend());

        double topEnergy = 0;
        for (int i = 0; i < binEnergy.size() * 0.1; i++) {
            topEnergy += binEnergy[i];
        }

        freqInfos[i].totalEnergy = energyFreq;
        freqInfos[i].topEnergy = topEnergy;

        applyInverseFFT(freq, outframe);
        
        std::copy(outframe.begin(), outframe.begin() + hopSize, output.begin() + i * hopSize);
    }
    input = move(output);
}

vector<FreqInfo> preProcess(int16_t* buf, int buflen, const UserParameters &params) {
    vector<double> dbuf(buflen);
    double maxAmplitude = 1e-9;
    for (int i = 0; i < buflen; i++) {
        dbuf[i] = buf[i] / 32768.0;
        maxAmplitude = max(maxAmplitude, std::abs(dbuf[i]));
    }
    for (int i = 0; i < buflen; i++) {
        dbuf[i] = dbuf[i] / maxAmplitude;
    }
    
    vector<FreqInfo> freqInfos(buflen / hopSize);
    doFiltering(dbuf, windowSize, hopSize, params, freqInfos);
    for (int i = 0; i < buflen; i++) {
        buf[i] = dbuf[i] * maxAmplitude * 32768;
    }
    return freqInfos;
}


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
    int error = src_simple(&data, SRC_SINC_FASTEST, 1);

    if (error != 0) {
        // Handle error (you can check the error code in the libsamplerate documentation)
        // For simplicity, we print the error code here
        std::cerr << "libsamplerate error: " << error << std::endl;
        delete[] data.data_in;
        delete[] data.data_out;
        exit(1);
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


bool endsWith(const std::string& str, const std::string& suffix) {
    if (str.length() < suffix.length()) {
        return false;
    }
    return str.substr(str.length() - suffix.length()) == suffix;
}

bool createDirectory(const std::wstring& path) {
    if (CreateDirectory(path.c_str(), NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
        return true;
    }
    else {
        std::wcerr << L"Failed to create directory: " << path << std::endl;
        return false;
    }
}

vector<FreqInfo> mergeFreqInfo(const vector<FreqInfo>& c1, const vector<FreqInfo>& c2) {
    vector<FreqInfo> res(c1.size() / 2);
    for (int i = 0; i < res.size(); i++) {
        res[i].topEnergy = std::max<double>(c1[2 * i].topEnergy + c1[2 * i + 1].totalEnergy,
            c2[2 * i].topEnergy + c2[2 * i + 1].totalEnergy);
        res[i].totalEnergy = std::max<double>(c1[2 * i].totalEnergy + c1[2 * i + 1].totalEnergy,
            c2[2 * i].totalEnergy + c2[2 * i + 1].totalEnergy);
    }
    return res;
}


int main(int argc, char **argv)
{
    SetConsoleOutputCP(CP_UTF8);
    ArgsParser args(argc, argv);
    if (argc < 2 || args.cmdOptionExists("-h")) {
        showHelpPage();
        if (argc < 2) {
            system("pause");
        }
        return 0;
    }
    string filename = args.getLastArg();
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open() || !endsWith(filename, ".wav")) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return 1;
    }

    UserParameters params;
    if (args.cmdOptionExists("-o")) {
        params.outputFilename = args.getCmdOption("-o");
    }
    if (args.cmdOptionExists("--no-filtering")) {
        params.useFiltering = false;
    }
    if (args.cmdOptionExists("--min-freq")) {
        params.minFreq = args.getIntArg("--min-freq", params.minFreq);
    }
    if (args.cmdOptionExists("--max-freq")) {
        params.maxFreq = args.getIntArg("--max-freq", params.maxFreq);
    }
    if (args.cmdOptionExists("--energy-threshold")) {
        params.energyThreshold = args.getFloatArg("--energy-threshold", params.energyThreshold);
    }
    if (args.cmdOptionExists("--out-filtered")) {
        params.filterOutputFile = args.getCmdOption("--out-filtered");
    }
    if (args.cmdOptionExists("--vad-mode")) {
        params.vadMode = args.getIntArg("--vad-mode", params.vadMode);
    }
    if (args.cmdOptionExists("--merge-threshold")) {
        params.mergeThreshold = args.getIntArg("--merge-threshold", params.mergeThreshold);
    }
    if (args.cmdOptionExists("--min-valid-duration")) {
        params.minValidDuration = args.getIntArg("--min-valid-duration", params.minValidDuration);
    }
    if (args.cmdOptionExists("--min-gap-duration")) {
        params.minGapDuration = args.getIntArg("--min-gap-duration", params.minGapDuration);
    }
    if (args.cmdOptionExists("--start-margin")) {
        params.startMargin = args.getIntArg("--start-margin", params.startMargin);
    }
    if (args.cmdOptionExists("--end-margin")) {
        params.endMargin = args.getIntArg("--end-margin", params.endMargin);
    }
    if (args.cmdOptionExists("--min-clear-ratio")) {
        params.minValidTopFreqEnergyRatio = args.getFloatArg("--min-clear-ratio", params.minValidTopFreqEnergyRatio);
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

    if (header.audioFormat != 1) {
        std::cerr << "audio format must be 1" << std::endl;
        return 1;
    }

    if (header.numChannels != 2) {
        std::cerr << "num of channels must be 2" << std::endl;
        return 1;
    }
    if (header.bitsPerSample != 16) {
        std::cerr << "bits per sample must be 16" << std::endl;
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
    vector<int16_t> channel1(dataSize / 4);
    vector<int16_t> channel2(dataSize / 4);
    
    cout << "copying channel data..." << endl;

    for (size_t i = 0; i < audioData.size(); i += header.numChannels * bytesPerSample) {
        int16_t temp;
        // Copy left channel sample
        std::copy(audioData.begin() + i, audioData.begin() + i + bytesPerSample, reinterpret_cast<char *>(&temp));
        channel1[i / 4] = temp;
        // Copy right channel sample
        std::copy(audioData.begin() + i + bytesPerSample, audioData.begin() + i + header.numChannels * bytesPerSample, reinterpret_cast<char*>(&temp));
        channel2[i / 4] = temp;
    }

    audioData.clear();

    if (header.sampleRate != targetSampleRate) {
        cout << "resampling..." << endl;
        channel1 = resampleAudio(channel1, header.sampleRate, targetSampleRate);
        channel2 = resampleAudio(channel2, header.sampleRate, targetSampleRate);
    }

    printf("number of sample: %d\n", channel1.size());

    int16_t* data1 = (int16_t*)channel1.data();
    int16_t* data2 = (int16_t*)channel2.data();


    Fvad* vad;
    vad = fvad_new();
    if (!vad) {
        std::cout << "error init";
    }
    fvad_set_mode(vad, params.vadMode);
    fvad_set_sample_rate(vad, targetSampleRate);


    const size_t resultLen = channel1.size() * 2 / windowSizeByte;
    vector<int> result1(resultLen);
    vector<int> result(resultLen);

    printf("resultLen = %d\n", resultLen);

    cout << "apply fft and filter..." << endl;
    
    vector<FreqInfo> c1Freqinfos = preProcess(data1, channel1.size(), params);
    vector<FreqInfo> c2Freqinfos = preProcess(data2, channel2.size(), params);
    vector<FreqInfo> mergedFreqInfos = mergeFreqInfo(c1Freqinfos, c2Freqinfos);

    if (resultLen != mergedFreqInfos.size()) {
        cerr << "err merged freqinfo len" << endl;
        printf("%d %d\n", resultLen, mergedFreqInfos.size());
        exit(1);
    }
    
    if (params.filterOutputFile.size()) {
        cout << "writting filtered channel 1 data..." << endl;
        writeChannelToWAV(params.filterOutputFile, channel1);
    }

    cout << "perform voice activity detection..." << endl;

    for (size_t i = 0; i < resultLen; i++) {
        result1[i] = fvad_process(vad, data1 + i * windowSize, windowSize);
    }
    for (size_t i = 0; i < resultLen; i++) {
        int res2 = fvad_process(vad, data2 + i * windowSize, windowSize);
        result[i] = (result1[i] == 1 || res2 == 1);
    }

    cout << "start post processing..." << endl;

    const int resPerSec = 1000 / windowDurationMS;
    postProcess(result, params, mergedFreqInfos);
    for (int sec = 0; sec < 30; sec++) {
        printf("%2d ", sec);
        for (int i = 0; i < resPerSec; i++) {
            if (sec * resPerSec + i >= result.size()) {
                printf("\n");
                goto end_print;
            }
            printf("%d", result[sec * resPerSec + i]);
        }
        printf("\n");
    }
    end_print:

    fvad_free(vad);

    cout << "generating result..." << endl;

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


    FILE* outsrt = fopen(params.outputFilename.c_str(), "w");
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
