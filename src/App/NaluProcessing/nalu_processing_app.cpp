#include "nalu_processing_lib.h"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

void print_help() {
  // clang-format off
  std::cout
	<< "Usage: NALUProcessing <Source> <Inject> <Output> <NALid Source> [<NALidInject> <NALnum Source> <NALnum Inject> <APSrestore> <Codec>]" << std::endl
  << std::endl
	<< "    <Source>                       input .264/.265/.266 Annex B stream" << std::endl
	<< "    <Inject>                       input .264/.265/.266 Annex B stream" << std::endl
	<< "    <Output>                       output .264/.265/.266 Annex B stream" << std::endl
	<< "    <NALid Source>                 NAL start pos to exchange, start count at 0" << std::endl
	<< "    <NALid Inject = NALid Source>  NAL start pos to exchange, start count at 0" << std::endl
	<< "    <NALnum Source = 1>            number of NAL to exchange" << std::endl
	<< "    <NALnum Inject = 1>            number of NAL to exchange" << std::endl
	<< "    <Codec = 0>                    0: H.264/AVC; 1: H.265/HEVC; 2:H.266/VVC" << std::endl
	<< "    <APSrestore = 0>               0: disable; 1: restore lost APS from Source after NAL insert (only H.266); 2: restore APS from Inject; 3: restore APS from both " << std::endl
	<< "    <Count = 0>                    0: Count NAL id; 1: Count video slices (Source) and NALid (Inject); 2: Count NALid (Source) and video slices (Inject); 3: Count both video slices" << std::endl
	<< std::endl;
  // clang-format on
}

int main(int argc, char *argv[]) {
  std::ifstream inputfile1;
  std::ifstream inputfile2;
  std::ofstream outputfile;
  int nalid1 = 0;
  int nalid2 = 0;
  int nalnum1 = 0;
  int nalnum2 = 0;
  int apsrestore = 0;
  int countnal = 0;
  CodecType codec = CODEC_AVC;
  // Check command line arguments
  int returnval = 0;
  if (argc != 5 && argc != 8 && argc != 9 && argc != 10 && argc != 11) {
    std::cout << "Incorrect number of arguments!" << std::endl;
    returnval = -1;
  } else if (argc > 4 && !isdigit(argv[4][0])) {
    std::cout << "NALid should be integer!" << std::endl;
    returnval = -1;
  } else {
    // Open IO files
    inputfile1.open(argv[1], std::ios::binary);
    if (!inputfile1.is_open()) {
      std::cout << "Could not open Source file '" << argv[1] << "'!"
                << std::endl;
      returnval = -1;
    }
    inputfile2.open(argv[2], std::ios::binary);
    if (!inputfile2.is_open()) {
      std::cout << "Could not open Inject file '" << argv[2] << "'!"
                << std::endl;
      returnval = -1;
    }
    outputfile.open(argv[3], std::ios::binary);
    if (!outputfile.is_open()) {
      std::cout << "Could not open output file '" << argv[3] << "'!"
                << std::endl;
      returnval = -1;
    }
    nalid1 = atoi(argv[4]);
    nalid2 = nalid1;
    nalnum1 = 1;
    nalnum2 = 1;
    apsrestore = 0;
    codec = CODEC_AVC;
    if (argc >= 8) {
      if (isdigit(argv[5][0]) && isdigit(argv[6][0]) && isdigit(argv[7][0])) {
        nalid2 = atoi(argv[5]);
        nalnum1 = atoi(argv[6]);
        nalnum2 = atoi(argv[7]);
      } else {
        std::cout << "Arguments NALid or NALnum are not integer!" << std::endl;
        returnval = -1;
      }
    }
    if (argc >= 9) {
      if (isdigit(argv[8][0])) {
        codec = (CodecType) atoi(argv[8]);
      } else {
        std::cout << "Argument Codec is not integer!" << std::endl;
        returnval = -1;
      }
    }
    if (argc >= 10) {
      if (isdigit(argv[9][0])) {
        apsrestore = atoi(argv[9]);
      } else {
        std::cout << "Argument APSrestore is not integer!" << std::endl;
        returnval = -1;
      }
    }
    if (argc >= 11) {
      if (isdigit(argv[10][0])) {
        countnal = atoi(argv[10]);
      } else {
        std::cout << "Argument Count NAL is not integer!" << std::endl;
        returnval = -1;
      }
    }
  }
  if (returnval < 0) {
    std::cout << std::endl;
    print_help();
    return returnval;
  }

  // Store input in vectors for easy processing
  std::vector<uint8_t> bufferinputfile1(
      (std::istreambuf_iterator<char>(inputfile1)),
      std::istreambuf_iterator<char>());
  std::vector<uint8_t> bufferinputfile2(
      (std::istreambuf_iterator<char>(inputfile2)),
      std::istreambuf_iterator<char>());
  inputfile1.close();
  inputfile2.close();

  std::vector<uint8_t> bufferoutputfile;

  process(bufferinputfile1, bufferinputfile2, bufferoutputfile, nalid1, nalid2,
          nalnum1, nalnum2, apsrestore, countnal, codec);

  outputfile.write(reinterpret_cast<const char *>(&bufferoutputfile[0]),
                   (bufferoutputfile.end() - bufferoutputfile.begin()) *
                       sizeof(uint8_t));
  outputfile.close();
  return returnval;
}
