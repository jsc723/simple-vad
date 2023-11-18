# SimpleVAD

自动打轴工具

读取一个WAV音频文件，生成一个SRT字幕文件

包含一个命令行工具(simple-vad.exe)和一个图形界面(simple-vad-gui.exe)

### 命令行工具使用说明

```c++
Usage: ./simple-vad.exe [options] <input_file>
-o FILENAME               : specify the name of the output subtitle file, default output.srt

--no-filtering            : do not perform filtering, default disabled
--min-freq  INT           : filter out all frequency below this number, default 0
--max-freq  INT           : filter out all frequency above this number, default 8000
--energy-threshold FLOAT  : filter out all sound frame whose energy is below this number, default 0.001
--out-filtered FILENAME   : output the audio after filtering, before it is passed to vad, no output by default

--vad-mode INT            : set the mode for the fvad library, larger => less chance to be classified as voice,
                            range from 0-3, default 1

--merge-threshold INT     : merge small and close voice segments, larger => merge more, default 500
--min-valid-duration INT  : merge or extend the voice segments shorter than this number, default 1000(ms)
--min-gap-duration INT    : merge the gaps between voice segments shorter than this number, default 200(ms)

--min-clear-ratio FLOAT   : range 0-1, remove all segments which doesn't have enough portion of frames
                            with a top frequency bin energy to total energy above this number, default 0.85
--start-margin INT        : add a margin before each segment, default 0(ms)
--end-margin INT          : add a margin after each segment, default 100(ms)
-h                        : show this help page

```

例：

```
./simple-vad.exe -o test.srt --energe-threshold 0.1 --min-gap-duration 300 --start-margin 100 input.wav
```

### 用户界面使用说明
每个设置都和命令行的选项对应，更方便使用一些

![img1](https://github.com/jsc723/simple-vad/blob/master/img/1.png?raw=true)

