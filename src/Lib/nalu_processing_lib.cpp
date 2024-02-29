#include "nalu_processing_lib.h"
#include <fstream>

int read_file_to_vector(std::string filename,
                        std::vector<uint8_t> &vectorbuffer) {
  int returnval = 0;
  std::ifstream file;
  file.open(filename, std::ios::binary);
  if (!file.is_open()) {
    std::cout << "Could not open Source file '" << filename << "'!"
              << std::endl;
    returnval = -1;
  }
  vectorbuffer = std::vector<uint8_t>((std::istreambuf_iterator<char>(file)),
                                      std::istreambuf_iterator<char>());
  file.close();
  vectorbuffer.push_back(
      0); // add 1 element to the end to be able to include the last NAL
  return returnval;
}

bool find_nal_unit(std::vector<uint8_t>::iterator &nalendinput,
                   std::vector<uint8_t> &bufferinputfile) {
  bool nal_end_found = false;
  bool eos_found = false;
  if (nalendinput >= bufferinputfile.end()) eos_found = true;
  while (!nal_end_found && !eos_found) {
    if (nalendinput + 3 >= bufferinputfile.end()) {
      ++nalendinput;
      ++nalendinput;
      nal_end_found = true;
      break;
    }
    if (nalendinput[0] == 0 && nalendinput[1] == 0 && nalendinput[2] == 1) {
      // Start of next NALU found
      nal_end_found = true;
      break;
    }
    ++nalendinput;
  }
  return !eos_found;
}

bool is_aps(std::vector<uint8_t>::iterator &nalstartinput, CodecType codec) {
  int start_idx = 3; // first NAL has 00 00 00 01 instead of 00 00 01
  if (nalstartinput[2] != 1) {
    start_idx = 4;
  }
  if (codec == CODEC_VVC) {
    int nalutype = (nalstartinput[start_idx + 1] >> 3);
    if (nalutype == 17 || nalutype == 18) return true;
  }
  return false;
}

bool is_vcl(std::vector<uint8_t>::iterator &NALstartinput, CodecType codec) {
  int start_idx = 3; // first NAL has 00 00 00 01 instead of 00 00 01
  if (NALstartinput[2] != 1) {
    start_idx = 4;
  }
  if (codec == CODEC_AVC) {
    int nalutype = (NALstartinput[start_idx] & (uint8_t)31);
    if (nalutype < 6) return true;
  } else if (codec == CODEC_HEVC) {
    int nalutype = (NALstartinput[start_idx] >> 1);
    if (nalutype < 32) return true;
  } else if (codec == CODEC_VVC) {
    int nalutype = (NALstartinput[start_idx + 1] >> 3);
    if (nalutype < 12) return true;
  }
  return false;
}

NaluType get_nalu_type(std::vector<uint8_t>::iterator &NALstartinput,
                       CodecType codec) {
  int start_idx = 3; // first NAL has 00 00 00 01 instead of 00 00 01
  if (NALstartinput[2] != 1) {
    start_idx = 4;
  }
  if (codec == CODEC_AVC) {
    int nalutype = (NALstartinput[start_idx] & (uint8_t)31);
    if (nalutype < 6) return VCL;
    if (nalutype == 7) return SPS;
    if (nalutype == 8) return PPS;
    if (nalutype == 13) return SPSExt;
  } else if (codec == CODEC_HEVC) {
    int nalutype = (NALstartinput[start_idx] >> 1);
    if (nalutype < 32) return VCL;
    if (nalutype == 32) return VPS;
    if (nalutype == 33) return SPS;
    if (nalutype == 34) return PPS;
  } else if (codec == CODEC_VVC) {
    int nalutype = (NALstartinput[start_idx + 1] >> 3);
    if (nalutype < 12) return VCL;
    if (nalutype == 14) return VPS;
    if (nalutype == 15) return SPS;
    if (nalutype == 16) return PPS;
    if (nalutype == 17) return APS;
    if (nalutype == 18) return APS;
  }
  return OTHER;
}

int get_apsid(std::vector<uint8_t>::iterator &NALstartinput, CodecType codec) {
  if (codec == CODEC_VVC) {
    int start_idx = 3; // first NAL has 00 00 00 01 instead of 00 00 01
    if (NALstartinput[2] != 1) {
      start_idx = 4;
    }
    return NALstartinput[start_idx + 2];
  }
  return 0;
}

int get_temporal_id(std::vector<uint8_t>::iterator &NALstartinput,
                    CodecType codec) {
  int start_idx = 3; // first NAL has 00 00 00 01 instead of 00 00 01
  if (NALstartinput[2] != 1) {
    start_idx = 4;
  }
  if (codec == CODEC_VVC) {
    int nuh_temporal_id_plus1 = (NALstartinput[start_idx + 1] & (uint8_t)7);
    return nuh_temporal_id_plus1 - 1;
  }
  return 0;
}

int vector_to_nalu_vector(std::vector<uint8_t> &vectorbuffer,
                          std::vector<Nalu> &vectornalu, CodecType codec) {
  // iterators to pinpoint the location of individual NALUs while walking
  // through the bitstream
  std::vector<uint8_t>::iterator nalendinput1 = vectorbuffer.begin();
  std::vector<uint8_t>::iterator nalstartinput1 = vectorbuffer.begin();

  while (find_nal_unit(nalendinput1, vectorbuffer)) {
    if (nalendinput1 - nalstartinput1 < 2) {
      ++nalendinput1;
      continue;
    }
    NaluType type = get_nalu_type(nalstartinput1, codec);
    int aps_id = 0;
    if (type == APS) {
      aps_id = get_apsid(nalstartinput1, codec);
    }
    int temp_id = get_temporal_id(nalstartinput1, codec);
    Nalu current_nalu(type, temp_id, aps_id, nalstartinput1, nalendinput1);
    vectornalu.push_back(current_nalu);
    nalstartinput1 = nalendinput1;
    ++nalendinput1;
  }
  return 0;
}

int write_nalu_vector_to_file(std::string filename,
                              std::vector<Nalu> &vectorbuffer) {
  int returnval = 0;
  std::ofstream outputfile;
  outputfile.open(filename, std::ios::binary);
  if (!outputfile.is_open()) {
    std::cout << "Could not open output file '" << filename << "'!"
              << std::endl;
    returnval = -1;
  }
  for (std::vector<Nalu>::iterator naloutput = vectorbuffer.begin();
       naloutput != vectorbuffer.end(); ++naloutput) {
    outputfile.write(reinterpret_cast<const char *>(&naloutput->start[0]),
                     (naloutput->end - naloutput->start) * sizeof(uint8_t));
  }
  outputfile.close();
  return returnval;
}

void check_and_save_aps(std::vector<uint8_t>::iterator &NALstartinput,
                        std::vector<uint8_t>::iterator &NALendinput,
                        std::map<int, std::vector<uint8_t>> &bufferAPS,
                        CodecType codec) {
  // if VVC NALU type PREFIX_APS_NUT or SUFFIX_APS_NUT
  if (is_aps(NALstartinput, codec)) {
    int apsid = NALstartinput[5];
    std::vector<uint8_t> tmp(NALstartinput, NALendinput);
    bufferAPS.erase(apsid);
    bufferAPS.insert(std::make_pair(apsid, std::move(tmp)));
  }
}

void insert_aps(std::map<int, Nalu> &bufferaps, Nalu &nalu) {
  bufferaps.erase(nalu.aps_id);
  bufferaps.insert(std::make_pair(nalu.aps_id, std::move(nalu)));
}

int process(std::vector<uint8_t> &bufferinputfile1,
            std::vector<uint8_t> &bufferinputfile2,
            std::vector<uint8_t> &bufferoutputfile, int nalid1, int nalid2,
            int nalnum1, int nalnum2, int apsrestore, int countnal,
            CodecType codec) {

  // bufferAPS contains APSs with their type and id as int identifier
  std::map<int, std::vector<uint8_t>> bufferapssource;
  std::map<int, std::vector<uint8_t>> bufferapsinject;

  // iterators to pinpoint the location of individual NALUs while walking
  // through the bitstream
  std::vector<uint8_t>::iterator nalendinput1 = bufferinputfile1.begin();
  std::vector<uint8_t>::iterator nalstartinput1 = bufferinputfile1.begin();
  std::vector<uint8_t>::iterator nalendinput2 = bufferinputfile2.begin();
  std::vector<uint8_t>::iterator nalstartinput2 = bufferinputfile2.begin();

  // Check for leading 0x00 and write to the output if present
  find_nal_unit(nalendinput1, bufferinputfile1);
  find_nal_unit(nalendinput2, bufferinputfile2);
  if (nalstartinput1 < nalendinput1) {
    bufferoutputfile.insert(bufferoutputfile.end(), nalstartinput1,
                            nalendinput1);
  }

  // Start writing from Source/input1
  nalstartinput1 = nalendinput1;
  std::vector<uint8_t>::iterator NALstartinputtmp = nalstartinput1;
  int i = 0;
  while (i < nalid1) {
    if (nalendinput1 + 1 > bufferinputfile1.end()) break;
    ++nalendinput1;
    find_nal_unit(nalendinput1, bufferinputfile1);
    if (apsrestore == 1 || apsrestore == 3)
      check_and_save_aps(NALstartinputtmp, nalendinput1, bufferapssource,
                         codec);
    if ((countnal == 0 || countnal == 2) || is_vcl(NALstartinputtmp, codec))
      ++i;
    NALstartinputtmp = nalendinput1;
  }
  if (nalstartinput1 < nalendinput1) {
    bufferoutputfile.insert(bufferoutputfile.end(), nalstartinput1,
                            nalendinput1);
  }

  // Start reading from Inject/input2
  nalstartinput2 = nalendinput2;
  NALstartinputtmp = nalstartinput2;
  i = 0;
  while (i < nalid2) {
    if (nalendinput2 + 1 >= bufferinputfile2.end()) break;
    ++nalendinput2;
    find_nal_unit(nalendinput2, bufferinputfile2);
    if (apsrestore == 2 || apsrestore == 3)
      check_and_save_aps(NALstartinputtmp, nalendinput2, bufferapsinject,
                         codec);
    if ((countnal == 0 || countnal == 1) || is_vcl(NALstartinputtmp, codec))
      ++i;
    NALstartinputtmp = nalendinput2;
  }

  // Continue with reading from Source/input1
  nalstartinput1 = nalendinput1;
  NALstartinputtmp = nalstartinput1;
  i = 0;
  while (i < nalnum1) {
    if (nalendinput1 + 1 >= bufferinputfile1.end()) break;
    ++nalendinput1;
    find_nal_unit(nalendinput1, bufferinputfile1);
    if (apsrestore == 1 || apsrestore == 3)
      check_and_save_aps(NALstartinputtmp, nalendinput1, bufferapssource,
                         codec);
    if ((countnal == 0 || countnal == 2) || is_vcl(NALstartinputtmp, codec))
      ++i;
    NALstartinputtmp = nalendinput1;
  }

  // write encountered APS packets from inject
  nalstartinput2 = nalendinput2;
  if (apsrestore == 2 || apsrestore == 3) {
    // first check if there is an additional APS in the inject stream
    ++nalendinput2;
    find_nal_unit(nalendinput2, bufferinputfile2);
    // if so, save it in the buffer aps
    check_and_save_aps(nalstartinput2, nalendinput2, bufferapsinject, codec);
    // if it was an APS, skip over it during the next process, otherwise rewind
    // and consider the NAL
    if (is_aps(nalstartinput2, codec))
      nalstartinput2 = nalendinput2;
    else
      nalendinput2 = nalstartinput2;

    for (auto const &x : bufferapsinject) {
      bufferoutputfile.insert(bufferoutputfile.end(), x.second.begin(),
                              x.second.end());
    }
  }

  // Continue with writing from Inject/input2
  nalstartinput2 = nalendinput2;
  NALstartinputtmp = nalstartinput2;
  i = 0;
  while (i < nalnum2) {
    if (nalendinput2 + 1 >= bufferinputfile2.end()) break;
    ++nalendinput2;
    find_nal_unit(nalendinput2, bufferinputfile2);
    if ((countnal == 0 || countnal == 1) || is_vcl(NALstartinputtmp, codec))
      ++i;
    NALstartinputtmp = nalendinput2;
  }
  if (nalstartinput2 < nalendinput2)
    bufferoutputfile.insert(bufferoutputfile.end(), nalstartinput2,
                            nalendinput2);

  // write encountered APS packets back in
  nalstartinput1 = nalendinput1;
  if (apsrestore == 1 || apsrestore == 3) {
    // check if the first NAL in source is not an APS, because then there could
    // be duplicate APS with the same index
    ++nalendinput1;
    find_nal_unit(nalendinput1, bufferinputfile1);
    check_and_save_aps(nalstartinput1, nalendinput1, bufferapssource, codec);
    // if the first NAL in source was an APS, write it from the APS buffer, and
    // skip it in the next write procedure.
    if (is_aps(nalstartinput1, codec)) nalstartinput1 = nalendinput1;

    for (auto const &x : bufferapssource) {
      bufferoutputfile.insert(bufferoutputfile.end(), x.second.begin(),
                              x.second.end());
    }
  }

  // write remaining stream from Source/input1
  if (nalstartinput1 < bufferinputfile1.end())
    bufferoutputfile.insert(bufferoutputfile.end(), nalstartinput1,
                            bufferinputfile1.end());

  return 0;
}
