#include <iostream>
#include <vector>
#include <fstream>
#include <map>

void printHelp() {
	std::cout << "Usage: NALUProcessing <Source> <Inject> <output> <NALid Source> [<NALidInject> <NALnum Source> <NALnum Inject> <APSrestore> <Codec>]" << std::endl << std::endl;
	std::cout << "        <input Source>                 input .264/.265/.266 Annex B stream" << std::endl;
	std::cout << "        <input Inject>                 input .264/.265/.266 Annex B stream" << std::endl;
	std::cout << "        <output>                       output .264/.265/.266 Annex B stream" << std::endl;
	std::cout << "        <NALid Source>                 NAL start pos to exchange, start count at 0" << std::endl;
	std::cout << "        <NALid Inject = NALid Source>  NAL start pos to exchange, start count at 0" << std::endl;
	std::cout << "        <NALnum Source = 1>            number of NAL to exchange" << std::endl;
	std::cout << "        <NALnum Inject = 1>            number of NAL to exchange" << std::endl;
	std::cout << "        <APSrestore = 0>               0:  disable 1: restore lost APS after NAL insert (only H.266); 2: NALid only counts video slices" << std::endl;
	std::cout << "        <Codec = 0>                    0: H.264/AVC; 1: H.265/HEVC; 2:H.266/VVC" << std::endl;
	std::cout << std::endl;
}
enum CodecType {
	CODEC_AVC = 0,
	CODEC_HEVC = 1,
	CODEC_VVC = 2
};

void FindNalEnd(std::vector<unsigned char>::iterator& nalendinput, std::vector<unsigned char>& bufferinputfile) {
	bool NALendfound = false;
	while (!NALendfound && nalendinput + 2 < bufferinputfile.end()) {
		if (nalendinput[0] == 0 && nalendinput[1] == 0 && nalendinput[2] == 1) {
			// Start of next NALU found
			NALendfound = true;
			break;
		}
		++nalendinput;
	}
	if (!(nalendinput + 2 < bufferinputfile.end())) {
		++nalendinput;
		++nalendinput;
	}
}

bool IsAps(std::vector<unsigned char>::iterator& nalstartinput, int codec) {
	if (codec == CODEC_VVC) {
		int nalutype = (nalstartinput[4] >> 3);
		if (nalutype == 17 || nalutype == 18)
			return true;
	}
	return false;
}
bool IsVcl(std::vector<unsigned char>::iterator& NALstartinput, int codec) {
	if (codec == CODEC_AVC) {
		int nalutype = (NALstartinput[3] & (unsigned char) 31);
		if (nalutype < 6)
			return true;
	}
	else if (codec == CODEC_HEVC) {
		int nalutype = (NALstartinput[3] >> 1);
		if (nalutype < 32)
			return true;
	}
	else if (codec == CODEC_VVC) {
		int nalutype = (NALstartinput[4] >> 3);
		if (nalutype < 12)
			return true;
	}
	return false;
}

void CheckAndSaveAps(std::vector<unsigned char>::iterator& NALstartinput, std::vector<unsigned char>::iterator& NALendinput, std::map<int, std::vector<unsigned char> >& bufferAPS, int codec) {
	if (IsAps(NALstartinput, codec)) { //if VVC NALU type PREFIX_APS_NUT or SUFFIX_APS_NUT
		int apsid = NALstartinput[5];
		std::vector<unsigned char> tmp(NALstartinput, NALendinput);
		bufferAPS.erase(apsid);
		bufferAPS.insert(std::make_pair( apsid, tmp ));
	}
}

int main(int argc, char* argv[]) {
	std::ifstream inputfile1;
	std::ifstream inputfile2;
	std::ofstream outputfile;
	int nalid1 = 0;
	int nalid2 = 0;
	int nalnum1 = 0;
	int nalnum2 = 0;
	int apsrestore = 0;
	int codec = CODEC_AVC;
	// Check command line arguments
	int returnval = 0;
	if (argc != 5 && argc != 8 && argc != 9 && argc != 10) {
		std::cout << "Incorrect number of arguments!" << std::endl;
		returnval = -1;
	} else if (argc > 4 && !isdigit(argv[4][0])) {
		std::cout << "NALid should be integer!" << std::endl;
		returnval = -1;
	}
	else {
		//Open IO files
		inputfile1.open(argv[1], std::ios::binary);
		if (!inputfile1.is_open()) {
			std::cout << "Could not open Source file '" << argv[1] << "'!" << std::endl;
			returnval = -1;
		}
		inputfile2.open(argv[2], std::ios::binary);
		if (!inputfile2.is_open()) {
			std::cout << "Could not open Inject file '" << argv[2] << "'!" << std::endl;
			returnval = -1;
		}
		outputfile.open(argv[3], std::ios::binary);
		if (!outputfile.is_open()) {
			std::cout << "Could not open output file '" << argv[3] << "'!" << std::endl;
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
			}
			else {
				std::cout << "Arguments NALid or NALnum are not integer!" << std::endl;
				returnval = -1;
			}
		}
		if (argc >= 9) {			
			if (isdigit(argv[8][0])) {
				apsrestore = atoi(argv[8]);
			}
			else {
				std::cout << "Argument APSrestore is not integer!" << std::endl;
				returnval = -1;
			}
		}
		if (argc >= 10) {			
			if (isdigit(argv[9][0])) {
				codec = atoi(argv[9]);
			}
			else {
				std::cout << "Argument Codec is not integer!" << std::endl;
				returnval = -1;
			}
		}
	}	
	if (returnval < 0) {
		std::cout << std::endl;
		printHelp();
		return returnval;
	}

	//bufferAPS contains APSs with their type and id as int identifier
	std::map<int, std::vector<unsigned char> > bufferaps;	

	//Store input in vectors for easy processing
	std::vector<unsigned char> bufferinputfile1((std::istreambuf_iterator<char>(inputfile1)), std::istreambuf_iterator<char>());
	std::vector<unsigned char> bufferinputfile2((std::istreambuf_iterator<char>(inputfile2)), std::istreambuf_iterator<char>());
	inputfile1.close();
	inputfile2.close();

	//iterators to pinpoint the location of individual NALUs while walking through the bitstream
	std::vector<unsigned char>::iterator nalendinput1 = bufferinputfile1.begin();
	std::vector<unsigned char>::iterator nalstartinput1 = bufferinputfile1.begin();
	std::vector<unsigned char>::iterator nalendinput2 = bufferinputfile2.begin();
	std::vector<unsigned char>::iterator nalstartinput2 = bufferinputfile2.begin();

	//Check for leading 0x00 and write to the output if present
	FindNalEnd(nalendinput1, bufferinputfile1);
	FindNalEnd(nalendinput2, bufferinputfile2);
	if (nalstartinput1 < nalendinput1)
		outputfile.write(reinterpret_cast<const char*>(&nalstartinput1[0]), (nalendinput1 - nalstartinput1) * sizeof(unsigned char));
	outputfile.flush();

	//Start writing from Source/input1
	nalstartinput1 = nalendinput1;
	std::vector<unsigned char>::iterator NALstartinputtmp = nalstartinput1;
	int i = 0;
	while (i < nalid1) {
		if (nalendinput1 + 1 > bufferinputfile1.end())
			break;
		++nalendinput1;
		FindNalEnd(nalendinput1, bufferinputfile1);
		if (apsrestore == 1 || apsrestore == 2) CheckAndSaveAps(NALstartinputtmp, nalendinput1, bufferaps, codec);
		if (apsrestore != 2 || IsVcl(NALstartinputtmp, codec))
			++i;
		NALstartinputtmp = nalendinput1;
	}
	if (nalstartinput1 < nalendinput1)
		outputfile.write(reinterpret_cast<const char*>(&nalstartinput1[0]), (nalendinput1 - nalstartinput1) * sizeof(unsigned char));
	
	//Start reading from Inject/input2
	nalstartinput2 = nalendinput2;
	NALstartinputtmp = nalstartinput2;
	i = 0;
	while (i < nalid2) {
		if (nalendinput2 + 1 >= bufferinputfile2.end())
			break;
		++nalendinput2;
		FindNalEnd(nalendinput2, bufferinputfile2);
		if (apsrestore != 2 || IsVcl(NALstartinputtmp, codec))
			++i;
		NALstartinputtmp = nalendinput2;
	}
	outputfile.flush();

	//Continue with reading from Source/input1
	nalstartinput1 = nalendinput1;
	NALstartinputtmp = nalstartinput1;
	i = 0;
	while ( i < nalnum1) {
		if (nalendinput1 + 1 >= bufferinputfile1.end())
			break;
		++nalendinput1;
		FindNalEnd(nalendinput1, bufferinputfile1);
		if (apsrestore == 1 || apsrestore == 2) CheckAndSaveAps(NALstartinputtmp, nalendinput1, bufferaps, codec);
		if (apsrestore != 2 || IsVcl(NALstartinputtmp, codec))
			++i;
		NALstartinputtmp = nalendinput1;		
	}
	
	//Continue with writing from Inject/input2
	nalstartinput2 = nalendinput2;
	NALstartinputtmp = nalstartinput2;
	i = 0;
	while ( i < nalnum2) {
		if (nalendinput2 + 1 >= bufferinputfile2.end())
			break;
		++nalendinput2;
		FindNalEnd(nalendinput2, bufferinputfile2);
		if (apsrestore != 2 || IsVcl(NALstartinputtmp, codec))
			++i;
		NALstartinputtmp = nalendinput2;
	}
	if (nalstartinput2 < nalendinput2)
		outputfile.write(reinterpret_cast<const char*>(&nalstartinput2[0]), (nalendinput2 - nalstartinput2) * sizeof(unsigned char));

	//write encountered APS packets back in
	nalstartinput1 = nalendinput1;
	if (apsrestore == 1 || apsrestore == 2) {
		++nalendinput1;
		FindNalEnd(nalendinput1, bufferinputfile1);
		CheckAndSaveAps(nalstartinput1, nalendinput1, bufferaps, codec);
		if (IsAps(nalstartinput1, codec))
			nalstartinput1 = nalendinput1;

		for (auto const& x : bufferaps)
		{
			outputfile.write(reinterpret_cast<const char*>(&x.second[0]), x.second.size() * sizeof(unsigned char));
		}
	}

	//write remaining stream from Source/input1
	if (nalstartinput1 < bufferinputfile1.end())
		outputfile.write(reinterpret_cast<const char*>(&nalstartinput1[0]), (bufferinputfile1.end() - nalstartinput1) * sizeof(unsigned char));

	outputfile.close();
	return returnval;
}
