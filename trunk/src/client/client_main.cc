#include <iostream>
#include <signal.h>
#include "client.hh"
#include "../common/garbagecollector.hh"
#include "../common/debug.hh"
#include "../coding/raid0coding.hh"
#include "../coding/raid1coding.hh"
#include "../coding/raid5coding.hh"
#include "../coding/rscoding.hh"
#include "../../lib/logger.hh"
#include "boost/program_options.hpp"

using namespace std;

// handle ctrl-C for profiler
void sighandler(int signum) {
	if (signum == SIGINT) {
		debug_yellow("%s\n", "SIGINT received\n");
		exit(42);
	}
}

/// Client Object
Client* client;

/// Config Layer
ConfigLayer* configLayer;

int main(int argc, char *argv[]) {

//	if (argc < 4 || argc > 5) {
//		cout << "Upload: ./CLIENT [CLIENT ID] upload [SRC]" << endl;
//		cout << "Download: ./CLIENT [CLIENT ID] download [FILEID] [DST]" << endl;
//		exit(-1);
//	}

// handle signal for profiler
	signal(SIGINT, sighandler);

	configLayer = new ConfigLayer("clientconfig.xml");
	client = new Client(atoi(argv[1]));
	ClientCommunicator* communicator = client->getCommunicator();

	// setup log
	FILELog::ReportingLevel() = logDEBUG3;
	std::string logFileName = "client_" + to_string(client->getClientId())
			+ ".log";
	FILE* log_fd = fopen(logFileName.c_str(), "a");
	Output2FILE::Stream() = log_fd;

	// start server
	communicator->createServerSocket();

	// 1. Garbage Collection Thread (lamba function hack for singleton)
	thread garbageCollectionThread(
			[&]() {GarbageCollector::getInstance().start();});

	// 2. Receive Thread
	thread receiveThread(&Communicator::waitForMessage, communicator);

	// 3. Send Thread
	thread sendThread(&Communicator::sendMessage, communicator);

	communicator->setId(client->getClientId());
	communicator->setComponentType(CLIENT);

	//communicator->connectAllComponents();
	/*	communicator->connectToMds();
	 communicator->connectToMonitor();
	 communicator->getOsdListAndConnect();
	 */
	try {
		namespace po = boost::program_options;
		po::options_description desc("Options");
		desc.add_options()
				("help", "Print help messages")
				("upload", "file upload")
				("download", "file download")
				("raid0", "raid0 coding")
				("raid1", "raid0 coding")
				("raid5", "raid0 coding")
				("rs", "RS coding")
				("n", po::value<int>(), "number of replications")
				("k", po::value<int>(), "number of rs_k")
				("m", po::value<int>(), "number of rs_m")
				("w", po::value<int>(), "number of rs_w")
				("i", po::value<int>(), "file ID")
				("d", po::value<string>(), "destination file path")
				("f", po::value<string>(), "file path");

		po::variables_map vm;
		try {
			po::store(po::parse_command_line(argc, argv, desc), vm); // can throw

			/** --help option
			 */
			if (vm.count("help")) {
				std::cout << "Basic Command Line Parameter App" << std::endl
						<< desc << std::endl;
			}

			if (vm.count("upload")) {

				CodingScheme codingScheme;
				string codingSetting;
				if (vm.count("raid0")) {
					codingScheme = RAID0_CODING;
					std::cout << vm["n"].as<int>() << std::endl;
//	    	  	  		    	  	  		  S	codingSetting = Raid0Coding::generateSetting(vm["n"].as<uint32_t>());
				}

				if (vm.count("raid1")) {
					codingScheme = RAID1_CODING;
					codingSetting = Raid1Coding::generateSetting(
							vm["n"].as<int>());
				}
				if (vm.count("raid5")) {
					codingScheme = RAID5_CODING;
					codingSetting = Raid5Coding::generateSetting(
							vm["n"].as<int>());
				}

				if (vm.count("rs")) {
					codingScheme = RS_CODING;
					codingSetting = RSCoding::generateSetting(vm["k"].as<int>(),
							vm["m"].as<int>(), vm["w"].as<int>());
				}

				client->uploadFileRequest(vm["f"].as<string>(), codingScheme,
						codingSetting);

			}

			if (vm.count("download")) {
				client->downloadFileRequest(vm["i"].as<int>(),
						vm["d"].as<string>());

			}
			po::notify(vm);
			// throws on error, so do after help in case
			// there are any problems
		} catch (po::error& e) {
			std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
			std::cerr << desc << std::endl;
			return 1;
		}
	} catch (std::exception& e) {
		std::cerr << "Unhandled Exception reached the top of main: " << e.what()
				<< ", application will now exit" << std::endl;
		return 2;

	}

	garbageCollectionThread.join();
	receiveThread.join();
	sendThread.join();

	return 0;
}
