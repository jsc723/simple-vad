# simple-vad

自动打轴工具

读取一个WAV音频文件（必须是双声道），生成一个SRT字幕文件

包含一个命令行工具(simple-vad.exe)和一个图形界面(simple-vad-gui.exe)

### 命令行工具使用说明

```c++
void showHelpPage()
{
    cout << "Simple Voice Activity Detector by @jsc723 - version 1.2 - 2023\n\n";
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
    cout << "-h                        : show this help page";
    cout << endl;
}

```

例：

```
./simple-vad.exe -o test.srt --energe-threshold 0.1 --min-gap-duration 300 input.wav
```

### 用户界面使用说明
每个设置都和命令行的选项对应，更方便使用一些

![img1](https://github.com/jsc723/simple-vad/blob/master/img/1.png?raw=true)

