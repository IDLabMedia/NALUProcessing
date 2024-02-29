#include "nalu_processing_lib.h"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

void print_help() {
  // clang-format off
  std::cout
	<< "Usage: StreamProcessing <Source> <Inject> <Output> <TempId|File> [<Codec>]" << std::endl
  << std::endl
	<< "    <Source>                       input .264/.265/.266 Annex B stream" << std::endl
	<< "    <Inject>                       input .264/.265/.266 Annex B stream" << std::endl
	<< "    <Output>                       output .264/.265/.266 Annex B stream" << std::endl
	<< "    <File>                         file with frame numbers" << std::endl
	<< "    <Codec = 0>                    0: H.264/AVC; 1: H.265/HEVC; 2:H.266/VVC" << std::endl
	<< std::endl;
  // clang-format on
}
enum Mode { TEMPORAL, FILEBASED };

int main(int argc, char *argv[]) {
  std::string inputfilename1;
  std::string inputfilename2;
  std::string inputfilename_frame;
  std::string outputfilename;
  Mode mode = TEMPORAL;
  int temp_id = 0;

  CodecType codec = CODEC_AVC;
  // Check command line arguments
  int returnval = 0;
  if (argc != 5 && argc != 6) {
    std::cout << "Incorrect number of arguments!" << std::endl;
    returnval = -1;
  } else {
    // Open IO files
    inputfilename1 = std::string(argv[1]);
    inputfilename2 = std::string(argv[2]);
    outputfilename = std::string(argv[3]);
    if (isdigit(argv[4][0])) {
      temp_id = atoi(argv[4]);
      mode = TEMPORAL;
    } else {
      inputfilename_frame = std::string(argv[4]);
      mode = FILEBASED;
    }

    codec = CODEC_AVC;
    if (argc >= 6) {
      if (isdigit(argv[5][0])) {
        codec = (CodecType)atoi(argv[5]);
      } else {
        std::cout << "Argument Codec is not integer!" << std::endl;
        returnval = -1;
      }
    }
  }
  if (returnval < 0) {
    std::cout << std::endl;
    print_help();
    return returnval;
  }

  std::ifstream inputfile_frame(inputfilename_frame);
  std::istream_iterator<int> start(inputfile_frame), end;
  std::vector<int> replaceframes(start, end);
  
  // Store input in vectors for easy processing
  std::vector<uint8_t> bufferinputfile1;
  std::vector<uint8_t> bufferinputfile2;

  read_file_to_vector(inputfilename1, bufferinputfile1);
  read_file_to_vector(inputfilename2, bufferinputfile2);

  std::vector<Nalu> bufferinputnalu1;
  std::vector<Nalu> bufferinputnalu2;

  vector_to_nalu_vector(bufferinputfile1, bufferinputnalu1, codec);
  vector_to_nalu_vector(bufferinputfile2, bufferinputnalu2, codec);

  // bufferAPS contains APSs with their type and id as int identifier
  std::map<int, Nalu> bufferapssource;
  std::map<int, Nalu> bufferapsvclsource;
  std::map<int, Nalu> bufferapsinject;
  std::map<int, Nalu> bufferapsvclinject;

  std::vector<Nalu> bufferoutputnalu;

  int vcl1_idx = 0;
  int nalu2_idx = 0;
  int vcl2_idx = 0;
  bool vcl1_increase = false;
  bool vcl2_increase = false;
  bool injecting = false; // true if we have been writing from inject, false if
                          // we have been writing from the source.
  for (int nalu1_idx = 0; nalu1_idx < bufferinputnalu1.size(); ++nalu1_idx) {
    vcl1_increase = false;
    vcl2_increase = false;
    if (bufferinputnalu1[nalu1_idx].type == VPS ||
        bufferinputnalu1[nalu1_idx].type == SPS ||
        bufferinputnalu1[nalu1_idx].type == PPS) {
      bufferoutputnalu.push_back(bufferinputnalu1[nalu1_idx]);
      ++nalu2_idx; // Assume SPS and PPS are the same in both streams
    } else {
      // store src APS
      if (bufferinputnalu1[nalu1_idx].type == APS) {
        insert_aps(bufferapssource, bufferinputnalu1[nalu1_idx]);
        insert_aps(bufferapsvclsource, bufferinputnalu1[nalu1_idx]);
      } else if (bufferinputnalu1[nalu1_idx].type == VCL) {
        // store src in output if larger temp id
        if (std::find(replaceframes.begin(), replaceframes.end(), vcl1_idx) ==
            replaceframes.end()) {
          bool first_VCL_after_change = (injecting) ? true : false;
          injecting = false;
          if (first_VCL_after_change) { // push all APS parameter sets
            for (auto nalu : bufferapssource) {
              bufferoutputnalu.push_back(nalu.second);
            }
          } else { // if there is no switch from inject to source, just insert
                   // the APS of this VCL
            for (auto nalu : bufferapsvclsource) {
              bufferoutputnalu.push_back(nalu.second);
            }
          }
          // Push the actual VCL
          bufferoutputnalu.push_back(bufferinputnalu1[nalu1_idx]);
        }
        // signal that vcl has been seen
        ++vcl1_idx;
        bufferapsvclsource.clear();
        vcl1_increase = true;
      }
      // if vcl_increase then work on inject stream
      if (vcl1_increase) {
        while (!vcl2_increase) {
          // store inject APS
          if (bufferinputnalu2[nalu2_idx].type == APS) {
            insert_aps(bufferapsinject, bufferinputnalu2[nalu2_idx]);
            insert_aps(bufferapsvclinject, bufferinputnalu2[nalu2_idx]);
          } else { // VCL
            // store inject in output if smaller temp id
            if (std::find(replaceframes.begin(), replaceframes.end(),
                          vcl2_idx) != replaceframes.end()) {
              bool first_VCL_after_change = (!injecting) ? true : false;
              injecting = true;
              if (first_VCL_after_change) { // push all APS parameter sets
                for (auto nalu : bufferapsinject) {
                  bufferoutputnalu.push_back(nalu.second);
                }
              } else { // if there is no switch from inject to source, just
                       // insert the APS of this VCL
                for (auto nalu : bufferapsvclinject) {
                  bufferoutputnalu.push_back(nalu.second);
                }
              }
              // Push the actual VCL
              bufferoutputnalu.push_back(bufferinputnalu2[nalu2_idx]);
            }
            // signal that vcl has been seen
            if (bufferinputnalu2[nalu2_idx].type == VCL) {
              ++vcl2_idx;
              bufferapsvclinject.clear();
              vcl2_increase = true;
            }
          }

          ++nalu2_idx;
        }
      }
    }
  }

  write_nalu_vector_to_file(outputfilename, bufferoutputnalu);

  return returnval;
}
