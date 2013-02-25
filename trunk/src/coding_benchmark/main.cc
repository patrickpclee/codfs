#include <iostream>
#include <time.h>
#include <chrono>
#include <iomanip>
#include "../config/config.hh"
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include "../coding/coding.hh"
#include "../coding/raid0coding.hh"
#include "../coding/raid1coding.hh"
#include "../coding/raid5coding.hh"
#include "../coding/evenoddcoding.hh"
#include "../coding/rdpcoding.hh"
#include "../coding/rscoding.hh"
#include "../coding/cauchycoding.hh"
#include "../coding/embrcoding.hh"

#include "../common/convertor.hh"
#include "../common/debug.hh"

using namespace std;

Coding* _coding;

typedef chrono::high_resolution_clock Clock;
typedef chrono::milliseconds milliseconds;

int main(int argc, char *argv[]) {
	int n, k, m, w;
	string fs;
	string ss;
	string action;
	string coding;
	string codingSetting;

	try {
		namespace po = boost::program_options;
		po::options_description desc("Options");
		desc.add_options()
			("help", "Print help messages")
			("action,a", po::value<string>(&action)->required(),
			 "Action: encode/decode/repair")
			("coding,c", po::value<string>(&coding),
			 "Coding Scheme: raid0/raid1/raid5/evenodd/rdp/cauchyrs/rs/embr")
			("n,n", po::value<int>(&n),"number of stripes")
			("k,k", po::value<int>(&k),"number of rs_k")
			("m,m", po::value<int>(&m), "number of rs_m")
			("w,w", po::value<int>(&w), "number of rs_w")
			("filesize,f", po::value<string>(&fs), "file size")
			("segmentsize,s", po::value<string>(&ss), "segment size");
			//("erasurenum,e", po::value<int>(&ne), "number of erasure");

			po::variables_map vm;

		try {
			po::store(
					po::command_line_parser(argc, argv).options(desc).run(), vm);

			/** --help option
			 */
			if (vm.count("help")) {
				cout << endl << "ENCODE: ./CODING_BENCHMARK -a encode -c [CODING] -n ... -fs [FILESIZE] -ss [SEGMENT_SIZE]" << endl;
				cout << "DECODE: ./CODING_BENCHMARK -a decode -c [CODING] -n ... -ne [NUM_OF_ERASURE] -fs [FILESIZE] -ss [SEGMENT_SIZE]" << endl;
				cout << "REPAIR: ./CODING_BENCHMARK -a repair -c [CODING] -n ... -ne [NUM_OF_ERASURE] -fs [FILESIZE] -ss [SEGMENT_SIZE]" << endl;
				cout << "DECODE&REPAIR: ./CODING_BENCHMARK -a all -c [CODING] -n ... -ne [NUM_OF_ERASURE] -fs [FILESIZE] -ss [SEGMENT_SIZE]" << endl;
				cout << "Coding Benchmark Help" << endl << desc << endl;
				return 0;
			}

			po::notify(vm);
		} catch (boost::program_options::required_option& e) {
			std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
			cout << "NCVFS Client Help" << endl << desc << endl;
			return 1;
		} catch (boost::program_options::error& e) {
			std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
			cout << "NCVFS Client Help" << endl << desc << endl;
			return 1;
		}

		if (!vm.count("action")) {
			cout << "Please specify [ACTION]" << endl;
			return 1;
		}
		if (!vm.count("coding")) {
			cout << "Please specify [CODING]" << endl;
			return 1;
		}

		coding = boost::to_lower_copy (coding);
		if (coding == "raid0") {
			//			codingScheme = RAID0_CODING;
			if (vm.count("n")) {
				codingSetting = Raid0Coding::generateSetting(n);
				_coding = new Raid0Coding();
			} else {
				cout << "Please specify [n]" << endl;
				return 1;
			}
		}
		else if (coding == "raid1") {
			//			codingScheme = RAID1_CODING;
			if (vm.count("n")) {
				codingSetting = Raid1Coding::generateSetting(n);
				_coding = new Raid1Coding();
			} else {
				cout << "Please specify [n]" << endl;
				return 1;
			}
		}
		else if (coding == "raid5") {
			//			codingScheme = RAID5_CODING;
			if (vm.count("n")) {
				codingSetting = Raid5Coding::generateSetting(n);
				_coding = new Raid5Coding();
			} else {
				cout << "Please specify [n]" << endl;
				return 1;
			}
		}
		else if (coding == "evenodd") {
			//			codingScheme = EVENODD_CODING;
			if (vm.count("n")) {
				codingSetting = EvenOddCoding::generateSetting(n);
				_coding = new EvenOddCoding();
			} else {
				cout << "Please specify [n]" << endl;
				return 1;
			}
		}
		else if (coding == "rdp") {
			//			codingScheme = RDP_CODING;
			if (vm.count("n")) {
				codingSetting = RDPCoding::generateSetting(n);
				_coding = new RDPCoding();
			} else {
				cout << "Please specify [n]" << endl;
				return 1;
			}
		}
		else if (coding == "rs") {
			//			codingScheme = RS_CODING;
			if (vm.count("k") && vm.count("m") && vm.count("w")) {
				codingSetting = RSCoding::generateSetting(k,m,w);
				_coding = new RSCoding();
			} else {
				cout << "Please specify [k,m,w]" << endl;
				return 1;
			}
		}
		else if (coding == "cauchy") {
			//			codingScheme = CAUCHY_CODING;
			if (vm.count("k") && vm.count("m") && vm.count("w")) {
				codingSetting = CauchyCoding::generateSetting(k,m,w);
				_coding = new CauchyCoding();
			} else {
				cout << "Please specify [k,m,w]" << endl;
				return 1;
			}
		} else if (coding == "embr") {
			//			codingScheme = EMBR_CODING;
			if (vm.count("n") && vm.count("k") && vm.count("w")) {
				codingSetting = EMBRCoding::generateSetting(n,k,w);
				_coding = new EMBRCoding();
			} else {
				cout << "Please specify [n,k,w]" << endl;
				return 1;
			}
		} else {
			cout << "Invalid Coding" << endl;
			return 1;
		}

		if (!vm.count("filesize")) {
			cout << "Please specify [FILESIZE]" << endl;
			return 1;
		}
		if (!vm.count("segmentsize")) {
			cout << "Please specify [SEGMENTSIZE]" << endl;
			return 1;
		}
		/*
		   if((action == "decode") || (action == "repair")) {
		   if(!vm.count("erasurenum")) {
		   cout << "Please specify [ERASURENUM]" << endl;
		   return 1;
		   }
		   }
		 */
	}  catch (std::exception& e) {
		std::cerr << "Unhandled Exception reached the top of main: " << e.what()
			<< ", application will now exit" << std::endl;
		return -1;
	}

	cout << endl;
	cout << "///Parameters///" << endl;

	uint64_t fileSize = stringToByte(fs);
	cout << "File Size: " << fs << endl;

	uint64_t segmentSize = stringToByte(ss);
	cout << "Segment Size: " << ss << endl;

	int numOfBlocks = _coding->getBlockCountFromSetting(codingSetting);
	cout << "Number of Blocks: " << numOfBlocks << endl;

	cout << endl;
	cout << "///Preparaing Data///" << endl;
	SegmentData segmentData;
	segmentData.buf = (char*)calloc(segmentSize,1);
	segmentData.info.segmentSize = segmentSize;
	segmentData.info.segmentId = 0;
	segmentData.info.segmentPath = "";
	int* intptr = (int*) segmentData.buf;
	for (uint32_t i = 0; i < segmentSize / 4; ++i) {
		intptr[i] = rand();
	}

	cout << endl;
	cout << "///Encoding Data///" << endl;

	uint32_t numOfSegments = fileSize / segmentSize;
	if ((fileSize % segmentSize) != 0)
		++numOfSegments;
	int encodedSize = 0;
	Clock::time_point tStart = Clock::now();
	vector<BlockData> encodedDataList;
	for(uint32_t i = 0; i < numOfSegments; ++i){
		if(i == 0) {
			encodedDataList = _coding->encode(segmentData, codingSetting);
			for(auto block : encodedDataList)
				encodedSize += block.info.blockSize;
		} else {
			vector<BlockData> blockDataList = _coding->encode(segmentData, codingSetting);
			for(auto block : blockDataList) {
				encodedSize += block.info.blockSize;
				free(block.buf);
			}
		}
	}
	Clock::time_point tEncode = Clock::now();
	double durationEncode = (chrono::duration_cast < milliseconds
			> (tEncode - tStart)).count() / 1024.0;
	cout << fixed << setprecision(2);
	cout << "Time Spent on Encode: " << durationEncode << " secs" << endl;
	cout << "Encode Rate: " << fileSize / durationEncode / 1024 / 1024 << " MB/s" << endl;
	cout << "Encoded Size: " << encodedSize / 1024 / 1024 << " MB" << endl;

	if (action == "upload")
		return 0;


	if ((action == "decode") || (action == "all")) {
		cout << endl;
		cout << "///Decoding Data///" << endl;

		vector<bool> blockStatus;
		for(uint32_t i = 0; i < (uint32_t)numOfBlocks; ++i) {
			blockStatus.push_back(true);
		}

		Clock::time_point tRead = Clock::now();
		Clock::time_point tDecode = Clock::now();
		double durationTotalDecode = 0.0;
		uint64_t allReadSize = 0;
		for(uint32_t i = 0; i < (uint32_t)numOfBlocks; ++i) {
			double durationDecode = 0.0;
			blockStatus[i] = false;
			cout << "Erasure: " << i << endl;
			uint64_t totalReadSize = 0;
			for(uint32_t j = 0; j < (uint32_t)numOfSegments; ++j) {
				block_list_t requiredBlockSymbols = _coding->getRequiredBlockSymbols(blockStatus, segmentSize, codingSetting);


				vector<BlockData> repairBlockDataList(numOfBlocks);
				for(auto blockSymbol : requiredBlockSymbols) {
					uint32_t readSize = 0;
					for (auto offsetLength : blockSymbol.second) {
						readSize += offsetLength.second;
					}
					BlockData blockData;
					BlockData oriBlockData = encodedDataList[blockSymbol.first];
					uint32_t offset = 0;
					blockData.buf = (char*)calloc(readSize,1);
					for (auto offsetLength : blockSymbol.second) {
						memcpy(blockData.buf + offset, oriBlockData.buf + offsetLength.first, offsetLength.second); 
					}
					blockData.info.segmentId = 0;
					blockData.info.blockId = blockSymbol.first;
					blockData.info.blockSize = readSize;

					repairBlockDataList[blockSymbol.first] = blockData;
					totalReadSize += readSize;
				}

				//cout << "Read Size: " << totalReadSize / 1024 / 1024 << " MB" << endl;

				tRead = Clock::now();

				SegmentData segmentData = _coding->decode(repairBlockDataList, requiredBlockSymbols, segmentSize, codingSetting);

				tDecode = Clock::now();

				free(segmentData.buf);
				for(auto blockSymbol : requiredBlockSymbols) {
					free(repairBlockDataList[blockSymbol.first].buf);
				}

				double durationCurrentDecode = (chrono::duration_cast <milliseconds> (tDecode - tRead)).count() / 1024.0;

				//cout << durationCurrentDecode << " secs" << endl;

				durationDecode += durationCurrentDecode;
			}

			cout << fixed << setprecision(2);
			cout << "Read Size: " << totalReadSize / 1024 / 1024 << " MB" << endl;
			cout << "Time Spent on Decode: " << durationDecode << " secs" << endl;
			cout << "Decode Rate: " << fileSize / durationDecode / 1024 / 1024 << " MB/s" << endl;
			cout << endl;
			durationTotalDecode += durationDecode;
			allReadSize += totalReadSize;

			blockStatus[i] = true;
		}

		cout << fixed << setprecision(2);
		cout << "Average Read Size: " << allReadSize / numOfBlocks / 1024 / 1024 << " MB" << endl;
		cout << "Average Time Spent on Decode: " << durationTotalDecode / numOfBlocks << " secs" << endl;
		cout << "Average Decode Rate: " << fileSize / durationTotalDecode * numOfBlocks / 1024 / 1024 << " MB/s" << endl;
	}

	if ((action == "repair") || (action == "all" )){
		cout << endl;
		cout << "///Repairing Data///" << endl;

		vector<bool> blockStatus;
		for(uint32_t i = 0; i < (uint32_t)numOfBlocks; ++i) {
			blockStatus.push_back(true);
		}

		Clock::time_point tRead = Clock::now();
		Clock::time_point tDecode = Clock::now();
		double durationTotalRepair = 0.0;
		uint64_t allReadSize = 0;
		uint64_t totalRepairSize = 0;
		for(uint32_t i = 0; i < (uint32_t)numOfBlocks; ++i) {
			double durationRepair = 0.0;	
			blockStatus[i] = false;

			cout << "Erasure: " << i << endl;
			vector<uint32_t> failedBlocks;
			failedBlocks.push_back(i);

			uint64_t totalReadSize = 0;
			uint64_t repairSize = 0;
			for(uint32_t j = 0; j < (uint32_t)numOfSegments; ++j) {
				block_list_t repairBlockSymbols = _coding->getRepairBlockSymbols(failedBlocks, blockStatus, segmentSize, codingSetting);

				vector<BlockData> repairBlockDataList(numOfBlocks);

				for(auto blockSymbol : repairBlockSymbols) {
					uint32_t readSize = 0;
					for (auto offsetLength : blockSymbol.second) {
						readSize += offsetLength.second;
					}
					BlockData blockData;
					BlockData oriBlockData = encodedDataList[blockSymbol.first];
					uint32_t offset = 0;
					blockData.buf = (char*)calloc(readSize,1);
					for (auto offsetLength : blockSymbol.second) {
						memcpy(blockData.buf + offset, oriBlockData.buf + offsetLength.first, offsetLength.second); 
					}
					blockData.info.segmentId = 0;
					blockData.info.blockId = blockSymbol.first;
					blockData.info.blockSize = readSize;

					repairBlockDataList[blockSymbol.first] = blockData;
					totalReadSize += readSize;
				}

				tRead = Clock::now();

				vector<BlockData> repairedBlocks = _coding->repairBlocks(failedBlocks, repairBlockDataList, repairBlockSymbols, segmentSize, codingSetting);

				tDecode = Clock::now();

				for(auto blockData : repairedBlocks) {
					repairSize += blockData.info.blockSize;
				}

				
				for(auto blockSymbol : repairBlockSymbols) {
					free(repairBlockDataList[blockSymbol.first].buf);
				}

				double durationCurrentRepair = (chrono::duration_cast <milliseconds> (tDecode - tRead)).count() / 1024.0;

				durationRepair += durationCurrentRepair;
			}

			cout << fixed << setprecision(2);
			cout << "Read Size: " << totalReadSize / 1024 / 1024 << " MB" << endl;
			cout << "Time Spent on Repair: " << durationRepair << " secs" << endl;
			cout << "Repair Rate: " << repairSize / durationRepair / 1024 / 1024 << " MB/s" << endl;
			cout << endl;
			durationTotalRepair += durationRepair;
			totalRepairSize += repairSize;
			allReadSize += totalReadSize;

			blockStatus[i] = true;
		}

		cout << fixed << setprecision(2);
		cout << "Average Read Size: " << allReadSize / numOfBlocks /1024 / 1024 << " MB" << endl;
		cout << "Average Time Spent on Repair: " << durationTotalRepair / numOfBlocks << " secs" << endl;
		cout << "Average Repair Rate: " << totalRepairSize / durationTotalRepair / 1024 / 1024 << " MB/s" << endl;
	}

	return 0;
}
