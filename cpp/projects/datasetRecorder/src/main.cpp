///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//		Author:	Pablo Ramon Soria
//		Date:	2015-11-26
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <core/comm/Socket.h>
#include <core/time/time.h>
#include <implementations/sensors/MavrosSensor.h>
#include <opencv2/opencv.hpp>


#include <fstream>
#include <thread> 
#include <mutex>
#include <string>

//---------------------------------------------------------------------------------------------------------------------
#ifdef _WIN32
	#include <Windows.h>
	inline void do_mkdir(std::string _filename) {
		CreateDirectory(_filename.c_str(), NULL);
	}
#elif __linux__
	#include <sys/stat.h>
	#include <sys/types.h>
	inline void do_mkdir(std::string _filename) {
		mkdir(_filename.c_str(), 0700);
	}
#endif


//---------------------------------------------------------------------------------------------------------------------
volatile bool running = true;
volatile bool display = false;
std::mutex displayMutex;


int main(int _argc, char ** _argv) {
	#if defined(_HAS_ROS_LIBRARIES_)
	std::cout <<  "Initializing ros" << std::endl;
	ros::init(_argc, _argv, "Dataset_Recorder");
	#endif

	BOViL::comm::Socket *socket = nullptr;


	BOViL::STime::init();
	BOViL::STime *timerg = BOViL::STime::get();
	

	// Start a thread to stop other ones.
	std::thread stopThread([&]() {
		int cmd = -1;
		while(cmd != 0) {
			std::cout << "If you want to close, enter 0" << std::endl;
			std::cout << "If you want to enable/disable sent of images throught a socket, enter 1" << std::endl;
			std::cin >> cmd;
			if (cmd == 1) {
				display = !display;
			}
		}
		if (socket != nullptr && socket->getSocketDescriptor() != -1) {
			closesocket(socket->getSocketDescriptor());
			socket = nullptr;
		}

		display = false;
		running = false;
	});

	std::string folderName = "experiment_"+std::to_string(time(NULL))+"/";
	do_mkdir(folderName);

	// Start a thread for capturing imu.
	std::thread imuThread([&]() {
		MavrosSensor imu;
		BOViL::STime *timer = BOViL::STime::get();
		ImuData imuData;

		timer->delay(1);
		std::ofstream file(folderName+"imudata.txt");

		while (running) {
		#if defined(_HAS_ROS_LIBRARIES_)
		ros::spinOnce();
		#endif
			double t = timer->getTime();
			imuData = imu.get();

			file << t << ", " <<
					imuData.mAltitude << ", " <<
					imuData.mQuaternion[0] << ", " <<
					imuData.mQuaternion[1] << ", " <<
					imuData.mQuaternion[2] << ", " <<
					imuData.mQuaternion[3] << ", " <<
					imuData.mLinearSpeed[0] << ", " <<
					imuData.mLinearSpeed[1] << ", " <<
					imuData.mLinearSpeed[2] << ", " <<
					imuData.mLinearAcc[0] << ", " <<
					imuData.mLinearAcc[1] << ", " <<
					imuData.mLinearAcc[2] << ", " <<
					imuData.mAngularSpeed[0] << ", " <<
					imuData.mAngularSpeed[1] << ", " <<
					imuData.mAngularSpeed[2] << ", " <<
					imuData.mAngularAcc[0] << ", " <<
					imuData.mAngularAcc[1] << ", " <<
					imuData.mAngularAcc[2] << std::endl;
			timer->mDelay(1);
		}
	});

	timerg->delay(1);
	cv::Mat sFrame1, sFrame2;
	// Start a thread for capturing images
	std::thread frameThread([&]() {
		cv::VideoCapture cam1(0);
		cv::VideoCapture cam2(1);
		BOViL::STime *timer = BOViL::STime::get();
		cv::Mat frame1, frame2;	
		
		std::ofstream timeSpan(folderName+"timespan.txt");
		int index = 0;
		while (running) {
			double t = timer->getTime();
			cam1 >> frame1;
			cam2 >> frame2;

			if (display) {
				displayMutex.lock();
				frame1.copyTo(sFrame1);
				frame2.copyTo(sFrame2);
				displayMutex.unlock();
			}

			timeSpan << t << std::endl;
			cv::imwrite(folderName+ "cam1_"+std::to_string(index)+".jpg", frame1);
			cv::imwrite(folderName+ "cam2_"+std::to_string(index)+".jpg", frame2);
			index++;
		}
	});

	// Display thread
	std::thread displayThread([&]() {
		while (running) {
			while (display) {
				if(socket == nullptr)
					socket = BOViL::comm::Socket::createSocket(BOViL::comm::eSocketType::serverTCP, "5098");
				
				unsigned char *inBuff = new unsigned char[1024];
				int inLen = socket->receiveMsg(inBuff, 1024);
				if (inLen == -1) {
					socket->closeSocket();
					socket = nullptr;
					timerg->delay(5);
				}
				else {
					std::string inMsg = std::string((char*)inBuff, inLen);
					if (inMsg == "send") {
						cv::Mat frame;
						displayMutex.lock();
						cv::hconcat(sFrame1, sFrame2, frame);
						displayMutex.unlock();

						cv::resize(frame, frame, cv::Size(240, 80));

						int sizes[] = {frame.rows, frame.cols, frame.channels()};
						int outLen = socket->sendMsg((unsigned char*) sizes, sizeof(int)*3);
						if (outLen == -1) {
							socket->closeSocket();
							socket = nullptr;
							timerg->delay(5);
						}

						outLen = socket->sendMsg(frame.data, frame.rows*frame.cols*frame.channels());
						if (outLen == -1) {
							socket->closeSocket();
							socket = nullptr;
							timerg->delay(5);
						}
					}
					else {
						socket->closeSocket();
						socket = nullptr;
						timerg->delay(5);
					}
				}

				delete inBuff;
			}
		}
	});

	BOViL::STime *timer = BOViL::STime::get();
	while (running) {
		timer->mDelay(100);
	}

	timerg->delay(1);

	if(stopThread.joinable())
		stopThread.join();

	if(imuThread.joinable())
		imuThread.join();

	if(frameThread.joinable())
		frameThread.join();

	if(displayThread.joinable())
		displayThread.join();


}


