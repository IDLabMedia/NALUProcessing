# NALUProcessing

Process Network Abstraction Layer Units (NALU) of video streams encoded using the H.264/AVC, H.265/HEVC, or H.266/VVC standard.

This tool is created to manipulate video streams without having to re-encode the stream. Re encoding would otherwise result in loss of quality.
Supported manipulations are:
* Cutting videos at specified packets or frames.
* Simulating packet loss by removing certain packets from the video stream.
* Removing layers from scalable video streams.
* Inserting frames from a different video stream, e.g. for the [Keyframe Insertion technique ](https://media.idlab.ugent.be/keyframe-insertion-electronics).

## Preparation of video files

NALUProcessing only takes Annex B input video streams compressed using the following codecs: 
* H.264/AVC
* H.265/HEVC
* H.266/VVC

The container (MP4, MKV, ...) must be removed from the video stream before processing can be performed by the NALUProcessing application.

If the MP4 container contains H.264/AVC video, use the following command. 
```
ffmpeg -i video.mp4 -c:v copy -bsf:v h264_mp4toannexb video.h264
```

If the MP4 container contains H.265/HEVC video, use the following command. 
```
ffmpeg -i video.mp4 -c:v copy -bsf:v hevc_mp4toannexb video.h265
```

## CLI interface NALUProcessing

![NALUProcessing Interface](https://github.com/IDLabMedia/NALUProcessing/blob/main/docs/NALUProcessing_Interface.png)

```
Usage:
  Usage: ./NALUProcessing <Source> <Inject> <output> <NALid Source> [<NALidInject> <NALnum Source> <NALnum Inject> <APSrestore> <Codec>]

    <input Source>                 input .264/.265/.266 Annex B stream
    <input Inject>                 input .264/.265/.266 Annex B stream
    <output>                       output .264/.265/.266 Annex B stream
    <NALid Source>                 NAL start pos to exchange, start count at 0
    <NALid Inject = NALid Source>  NAL start pos to exchange, start count at 0
    <NALnum Source = 1>            number of NAL to exchange
    <NALnum Inject = 1>            number of NAL to exchange
    <Codec = 0>                    0: H.264/AVC 
                                   1: H.265/HEVC 
                                   2: H.266/VVC 
    <APSrestore = 0>               0: disable; 
                                   1: restore lost APS from Source after NAL insert (only H.266); 
                                   2: restore APS from Inject; 
                                   3: restore APS from both
    <Count = 0>                    0: Count NAL id; 
                                   1: Count video slices (Source) and NALid (Inject); 
                                   2: Count NALid (Source) and video slices (Inject); 
                                   3: Count both video slices
```

### Usage example H.264/AVC

![NALUProcessing Example H.264/AVC](https://github.com/IDLabMedia/NALUProcessing/blob/main/docs/NALUProcessing_ExampleH264.png)

### Usage example H.266/VVC

![NALUProcessing Example H.266/VVC](https://github.com/IDLabMedia/NALUProcessing/blob/main/docs/NALUProcessing_ExampleH266.png)


## CLI interface StreamProcessing
```
Usage:
  Usage: ./StreamProcessing <Source> <Inject> <Output> <TempId|File> [<Codec>]]

    <input Source>                 input .264/.265/.266 Annex B stream
    <input Inject>                 input .264/.265/.266 Annex B stream
    <output>                       output .264/.265/.266 Annex B stream
    <TempId|File>                  Temporal layers to switch (<TempId from Source, >=TempId from Inject) or file with frame numbers
    <Codec = 0>                    0: H.264/AVC; 1: H.265/HEVC; 2:H.266/VVC
```



## Reference

This work was published in:
* [MDPI Electronics - Keyframe Insertion Enabling Low-Latency Random Access and Packet Loss Repair](https://media.idlab.ugent.be/keyframe-insertion-electronics) 
* [Data Compression Conference (DCC) 2022 - Keyframe Insertion in H.264/AVC, H.265/HEVC, and H.266/VVC](https://media.idlab.ugent.be/keyframe-insertion)
* [Picture Coding Symposium 2022 - Mixed-Resolution HESP](https://media.idlab.ugent.be/hesp-mixed-res)

If you use the software, please cite the following paper:
```bibtex
@Article{electronics10060748,
AUTHOR = {Van Wallendael, Glenn and Mareen, Hannes and Vounckx, Johan and Lambert, Peter},
TITLE = {Keyframe Insertion: Enabling Low-Latency Random Access and Packet Loss Repair},
JOURNAL = {Electronics},
VOLUME = {10},
YEAR = {2021},
NUMBER = {6},
ARTICLE-NUMBER = {748},
URL = {https://doi.org/10.3390/electronics10060748},
ISSN = {2079-9292},
} 
```
