#include <iostream>
#include <signal.h>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include "client.hh"
#include "../common/garbagecollector.hh"
#include "../common/debug.hh"
#include "../coding/raid0coding.hh"
#include "../coding/raid1coding.hh"
#include "../coding/raid5coding.hh"
#include "../coding/rscoding.hh"
#include "../coding/embrcoding.hh"
#include "../coding/evenoddcoding.hh"
#include "../coding/rdpcoding.hh"

using namespace std;

// handle ctrl-C for profiler
void sighandler(int signum) {
	if (signum == SIGINT) {
		debug_yellow("%s\n", "SIGINT received\n");
		exit(42);
	}
}

/// Client Segment
Client* client;

/// Config Layer
ConfigLayer* configLayer;

int main(int argc, char *argv[]) {

//	if (argc < 4 || argc > 5) {
//		cout << "Upload: ./CLIENT [CLIENT ID] upload [SRC]" << endl;
//		cout << "Download: ./CLIENT [CLIENT ID] download [FILEID] [DST]" << endl;
//		exit(-1);
//	}

	int clientId;
	int n, k, m, w;
	int fileId;
	string action, coding, file;
	CodingScheme codingScheme = DEFAULT_CODING;
	string codingSetting;

	// START PROGRAM OPTIONS

	try {
		namespace po = boost::program_options;
		po::options_description desc("Options");
		desc.add_options()
				("help", "Print help messages")
				("action,a", po::value<string>(&action)->required(),
						"Action: upload/download")
				("coding,c", po::value<string>(&coding),
						"Coding Scheme: raid0/raid1/raid5/rs/embr")
				("cid,i", po::value<int>(&clientId)->required(),"client ID")
				("n,n", po::value<int>(&n),"number of stripes")
				("k,k", po::value<int>(&k),"number of rs_k")
				("m,m", po::value<int>(&m), "number of rs_m")
				("w,w", po::value<int>(&w), "number of rs_w")
				("fid,f", po::value<int>(&fileId), "file ID")
				("target,t", po::value<string>(&file)->required(), "dst/src file");

		po::positional_options_description positionalOptions;
		    positionalOptions.add("target", 1);

		po::variables_map vm;
		try {
			po::store(
					po::command_line_parser(argc, argv).options(desc).positional(
							positionalOptions).run(), vm);

			/** --help option
			 */
			if (vm.count("help")) {
				cout << endl << "UPLOAD: ./CLIENT -i [ID] -a upload -c [coding] -n [stripes] [SRC]" << endl;
				cout << "DOWNLOAD: ./CLIENT -i [ID] -a download -f [file_ID] [DST]" << endl;
				cout << "NCVFS Client Help" << endl << desc << endl;
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


		if (action == "upload") {
			if (!vm.count("coding")) {
				cout << "Please specify [coding]" << endl;
				return 1;
			}
			coding = boost::to_lower_copy (coding);
			if (coding == "raid0") {
				codingScheme = RAID0_CODING;
				if (vm.count("n")) {
					codingSetting = Raid0Coding::generateSetting(n);
				} else {
					cout << "Please specify [n]" << endl;
					return 1;
				}
			}
			else if (coding == "raid1") {
				codingScheme = RAID1_CODING;
				if (vm.count("n")) {
					codingSetting = Raid1Coding::generateSetting(n);
				} else {
					cout << "Please specify [n]" << endl;
					return 1;
				}
			}
			else if (coding == "raid5") {
				codingScheme = RAID5_CODING;
				if (vm.count("n")) {
					codingSetting = Raid5Coding::generateSetting(n);
				} else {
					cout << "Please specify [n]" << endl;
					return 1;
				}
			}
			else if (coding == "rs") {
				codingScheme = RS_CODING;
				if (vm.count("k") && vm.count("m") && vm.count("w")) {
					codingSetting = RSCoding::generateSetting(k,m,w);
				} else {
					cout << "Please specify [k,m,w]" << endl;
					return 1;
				}
			}
			else if (coding == "embr") {
				codingScheme = EMBR_CODING;
				if (vm.count("n") && vm.count("k") && vm.count("w")) {
					codingSetting = EMBRCoding::generateSetting(n,k,w);
				} else {
					cout << "Please specify [n,k,w]" << endl;
					return 1;
				}
			}
			else if (coding == "evenodd") {
				codingScheme = EVENODD_CODING;
				if (vm.count("n")) {
					codingSetting = EvenOddCoding::generateSetting(n);
				} else {
					cout << "Please specify [n]" << endl;
					return 1;
				}
			}
			else if (coding == "rdp") {
				codingScheme = RDP_CODING;
				if (vm.count("n")) {
					codingSetting = RDPCoding::generateSetting(n);
				} else {
					cout << "Please specify [n]" << endl;
					return 1;
				}
			}

		} else { // download
			if (!vm.count("fid")) {
				cout << "Please specify [fileId]" << endl;
				return 1;
			}
		}

	}  catch (std::exception& e) {
		std::cerr << "Unhandled Exception reached the top of main: " << e.what()
						<< ", application will now exit" << std::endl;
		return -1;
	}

	// END PROGRAM OPTIONS

// handle signal for profiler
	signal(SIGINT, sighandler);

	configLayer = new ConfigLayer("clientconfig.xml");
	client = new Client(clientId);
	ClientCommunicator* communicator = client->getCommunicator();

	// setup log
	/*
	FILELog::ReportingLevel() = logDEBUG3;
	std::string logFileName = "client_" + to_string(clientId)
			+ ".log";
	FILE* log_fd = fopen(logFileName.c_str(), "a");
	Output2FILE::Stream() = log_fd;
	*/

	// start server
	communicator->createServerSocket();

	// 1. Garbage Collection Thread (lamba function hack for singleton)
	thread garbageCollectionThread(
			[&]() {GarbageCollector::getInstance().start();});

	// 2. Receive Thread
	thread receiveThread(&Communicator::waitForMessage, communicator);

	communicator->setId(clientId);
	communicator->setComponentType(CLIENT);

	//communicator->connectAllComponents();
	communicator->connectToMds();
	communicator->connectToMonitor();
	communicator->getOsdListAndConnect();

	if (action == "upload") {
		client->uploadFileRequest(file, codingScheme, codingSetting);
	} else {
		client->downloadFileRequest(fileId, file);
	}

	cout << "Now Sleep 5 Seconds then Exit" << endl;
	sleep(5);
	exit(0);

	garbageCollectionThread.join();
	receiveThread.join();

	return 0;
}
