# NALUProcessing
Process NAL units of video streams.

## Preparation of video files
NALUProcessing only takes Annex B input video streams compressed using the following codecs: H.264/AVC, H.265/HEVC, H.266/VVC.
The container (MP4, MKV, ...) must be removed from the video stream before processing can be performed by the NALUProcessing application.
```
ffmpeg -i video.mp4 -codec copy -bsf:v h264_mp4toannexb video.h264
```


## CLI interface
![NALUProcessing Interface](https://github.com/IDLabMedia/NALUProcessing/blob/main/docs/NALUProcessing_Interface.png)

```
Usage:
  Usage: ./NALUCombiner <Source> <Inject> <output> <NALid Source> [<NALidInject> <NALnum Source> <NALnum Inject> <APSrestore> <Codec>]

    <input Source>                 input .264/.265/.266 Annex B stream
    <input Inject>                 input .264/.265/.266 Annex B stream
    <output>                       output .264/.265/.266 Annex B stream
    <NALid Source>                 NAL start pos to exchange, start count at 0
    <NALid Inject = NALid Source>  NAL start pos to exchange, start count at 0
    <NALnum Source = 1>            number of NAL to exchange
    <NALnum Inject = 1>            number of NAL to exchange
    <APSrestore = 0>               0: disable 
	                               1: restore lost APS after NAL insert (only H.266); 
								   2: option 1 + NALid only counts video slices
	<Codec = 0>                    0: H.264/AVC 
	                               1: H.265/HEVC 
								   2:H.266/VVC 
```
## Usage example H.264/AVC
![NALUProcessing Example H.264/AVC](https://github.com/IDLabMedia/NALUProcessing/blob/main/docs/NALUProcessing_ExampleH264.png)


## Usage example H.266/VVC
![NALUProcessing Example H.266/VVC](https://github.com/IDLabMedia/NALUProcessing/blob/main/docs/NALUProcessing_ExampleH266.png)



## Reference
This work was published in [MDPI Electronics](https://media.idlab.ugent.be/keyframe-insertion-electronics) and the [Data Compression Conference (DCC) 2022](https://media.idlab.ugent.be/keyframe-insertion).

```js
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