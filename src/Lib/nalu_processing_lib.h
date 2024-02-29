#include <cstdint>
#include <iostream>
#include <map>
#include <vector>

enum CodecType { CODEC_AVC = 0, CODEC_HEVC = 1, CODEC_VVC = 2 };

enum NaluType { VCL = 0, APS, VPS, SPS, SPSExt, PPS, OTHER };

class Nalu {
public:
  NaluType type;
  int temp_id;
  int aps_id;
  std::vector<uint8_t>::iterator start;
  std::vector<uint8_t>::iterator end;
  Nalu(NaluType type, int temp_id, int aps_id,
       std::vector<uint8_t>::iterator start, std::vector<uint8_t>::iterator end)
      : type(type), temp_id(temp_id), aps_id(aps_id), start(start), end(end) {}
};

int read_file_to_vector(std::string filename,
                        std::vector<uint8_t> &bufferinputfile1);

bool find_nal_unit(std::vector<uint8_t>::iterator &nalendinput,
                   std::vector<uint8_t> &bufferinputfile);

bool is_aps(std::vector<uint8_t>::iterator &nalstartinput, CodecType codec);

bool is_vcl(std::vector<uint8_t>::iterator &NALstartinput, CodecType codec);

NaluType get_nalu_type(std::vector<uint8_t>::iterator &NALstartinput,
                       CodecType codec);

int get_apsid(std::vector<uint8_t>::iterator &NALstartinput, CodecType codec);

int get_temporal_id(std::vector<uint8_t>::iterator &NALstartinput,
                    CodecType codec);

int vector_to_nalu_vector(std::vector<uint8_t> &vectorbuffer,
                          std::vector<Nalu> &vectornalu, CodecType codec);

int write_nalu_vector_to_file(std::string filename,
                              std::vector<Nalu> &vectorbuffer);

void check_and_save_aps(std::vector<uint8_t>::iterator &NALstartinput,
                        std::vector<uint8_t>::iterator &NALendinput,
                        std::map<int, std::vector<uint8_t>> &bufferAPS,
                        CodecType codec);

void insert_aps(std::map<int, Nalu> &bufferaps, Nalu &nalu);

int process(std::vector<uint8_t> &bufferinputfile1,
            std::vector<uint8_t> &bufferinputfile2,
            std::vector<uint8_t> &bufferoutputfile, int nalid1 = 0,
            int nalid2 = 0, int nalnum1 = 0, int nalnum2 = 0,
            int apsrestore = 0, int countnal = 0, CodecType codec = CODEC_AVC);
