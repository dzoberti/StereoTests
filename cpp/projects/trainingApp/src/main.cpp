///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//		Author:	Pablo Ramon SoriacvImagesPath
//		Date:	2015-10-02
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../../objectsMap/src/vision/ml/models/BoW.h"
#include "../../objectsMap/src/vision/StereoCameras.h"
#include "../../objectsMap/src/vision/ImageFilteringTools.h"

#include <sstream>
#include <Windows.h>
#include <opencv2/opencv.hpp>
#include <pcl/point_types.h>
#include <pcl/point_cloud.h>
#include <cjson/json.h>
#include <fstream>

using namespace algorithm;
using namespace std;
using namespace cv;
using namespace cjson;

void initConfig(string _path, Json &_config);
void initBow(BoW &_bow, Json &_config);
StereoCameras * initCameras(Json &_config);

// To do in a loop
void calculatePointCloud(StereoCameras *_cameras, vector<Point3f> &_cloud, Json &_config);
void reprojectPoints(const vector<Point3f> &_cloud, vector<Point2f> &_leftPoints, vector<Point2f> &_rightPoints);
void getSubImagesAndGroundTruth();

// For training
void trainModel(BoW &_bow, vector<Mat> &_images, Mat &_groundTruth);



//---------------------------------------------------------------------------------------------------------------------
int main(int _argc, char ** _argv) {
	BoW bow;
	StereoCameras *cameras;
	Json config;
	
	assert(_argc == 2);	// Added path to training configuration file.
	initConfig(string(_argv[1]), config);
	initBow(bow, config["recognitionSystem"]);
	cameras = initCameras(config["cameras"]);

	vector<Mat> images;
	Mat groundTruth;
	for (;;) {
		vector<Point3f> cloud;
		calculatePointCloud(cameras, cloud, config);
		
		waitKey();
		
		if(cloud.size() == 0)
			break;
		
		// 666 Todo next things.
	}

	trainModel(bow, images, groundTruth);


	// 666 TEST IMAGES.

	/*BoW bow;

	BoW::Params params;
	params.extractorType = BoW::Params::eExtractorType::SIFT;
	params.descriptorType = BoW::Params::eDescriptorType::SIFT;
	params.imageSizeTrain = 640;
	params.nScalesTrain = 1;
	params.scaleFactor = 0.5;
	params.vocSize = 500;

	bow.params(params);

	SvmModel svm;
	svm.setParams(2.25, 0.16384, cv::ml::SVM::Types::C_SVC,  cv::ml::SVM::KernelTypes::RBF);
	bow.model(svm);

	string imageTemplate = "C:/programming/datasets/objectsMaps/training (%d).jpg";
	string gtFile = "C:/programming/datasets/objectsMaps/gt.txt";
	bow.train(imageTemplate, gtFile);
	bow.save("canjuiboxcrasen2");

	string cvImagesPath = "C:/programming/datasets/objectsMaps/";
	vector<cv::Mat> cvImages;
	for (int i = 1;i<999;i=i+20) {
		cv::Mat image = cv::imread(cvImagesPath + "training (" + to_string(i) +").jpg");
		if(image.rows == 0)
			break;

		cv::resize(image, image, cv::Size(320,240));
		cvImages.push_back(image);
	}


	vector<vector<pair<unsigned,float>>> results;
	int index = 0;
	for (cv::Mat image : cvImages) {
		vector<cv::Rect> regions;
		regions.push_back(cv::Rect(0, 0, image.cols, image.rows));
		results.push_back(bow.evaluate(image, regions));

		stringstream ss;

		ss << "Image " << index << ". Label " << results[index][0].first << ". Prob " << results[results.size() -1][0].second;

		cv::putText(image, ss.str(), cv::Point2i(30,30), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0), 2);
		cv::imshow("display", image);
		cv::waitKey();
		index++;
	}

	system("PAUSE");*/

}

void initConfig(string _path, Json & _data) {
	ifstream file;
	file.open(_path);
	assert(file.is_open());
	_data.parse(file);
}

cv::ml::SVM::Types decodeSvmType(std::string _string) {
	if (_string == "c_svc") 
		return ml::SVM::Types::C_SVC;
	else if (_string == "v_svc") 
		return ml::SVM::Types::NU_SVC;
	else if (_string == "v_svr") 
		return ml::SVM::Types::NU_SVR;
	else if (_string == "eps_svr") 
		return ml::SVM::Types::EPS_SVR;
	else {
		assert(false);
		return  ml::SVM::Types::EPS_SVR;	// 666 never reach this.
	}
}
cv::ml::SVM::KernelTypes decodeKernelType(std::string _string) {
	if (_string == "RBF") {
		return ml::SVM::KernelTypes::RBF;
	} else {
		assert(false);
		return  ml::SVM::KernelTypes::RBF;	// 666 never reach this.
	}
}

void setModel(BoW & _bow, cjson::Json _params) {
	if (_params["name"] == "SVM") {
		SvmModel * mlModel = new SvmModel();
		static_cast<SvmModel*>(mlModel)->setParams(_params["params"]["c"], _params["params"]["gamma"], decodeSvmType(_params["params"]["svmType"]), decodeKernelType(_params["params"]["kernel"]));
		_bow.model(*mlModel);
		mlModel = nullptr;
	} else if (_params["name"] = "LDA") {		// 666 fill this too. 
	} else {
		assert(false);
	}
}

void setBowParams(algorithm::BoW::Params & _bowParams, cjson::Json _params) {
	_bowParams.vocSize = (int) _params["vocabularySize"];
}

void setFeatures(algorithm::BoW::Params & _bowParams, cjson::Json _params) {
	// get detector
	if (_params["detector"] == "SIFT") 
		_bowParams.extractorType = BoW::Params::eExtractorType::SIFT;
	else if (_params["detector"] == "FAST") 
		_bowParams.extractorType = BoW::Params::eExtractorType::FAST;
	else if (_params["detector"] == "ORB") 
		_bowParams.extractorType = BoW::Params::eExtractorType::ORB;
	else if (_params["detector"] == "SURF") 
		_bowParams.extractorType = BoW::Params::eExtractorType::SURF;
	else if (_params["detector"] == "MSER")
		_bowParams.extractorType = BoW::Params::eExtractorType::MSER;
	else 
		assert(false);
	
	if (_params["descriptor"] == "SIFT") 
		_bowParams.descriptorType = BoW::Params::eDescriptorType::SIFT;
	else if (_params["descriptor"] == "ORB") 
		_bowParams.descriptorType = BoW::Params::eDescriptorType::ORB;
	else if (_params["descriptor"] == "SURF") 
		_bowParams.descriptorType = BoW::Params::eDescriptorType::SURF;
	else 
		assert(false);
}

//---------------------------------------------------------------------------------------------------------------
void initBow(BoW & _bow, Json &_config) {
	algorithm::BoW::Params	bowParams;
	setBowParams(bowParams, _config["bovwParams"]);
	setFeatures(bowParams, _config["features"]);
	_bow.params(bowParams);
	setModel(_bow, _config["mlModel"]);
}

//---------------------------------------------------------------------------------------------------------------
StereoCameras * initCameras(Json &_config) {
	StereoCameras *  cameras = new StereoCameras(_config["left"], _config["right"]);
	Json leftRoi = _config["leftRoi"];
	Json rightRoi = _config["rightRoi"];
	cameras->roi(	Rect(leftRoi["x"],leftRoi["y"],leftRoi["width"],leftRoi["height"]), 
		Rect(rightRoi["x"],rightRoi["y"],rightRoi["width"],rightRoi["height"]));
	cameras->load(_config["paramFile"]);
	cameras->rangeZ(_config["pointRanges"]["z"]["min"], _config["pointRanges"]["z"]["max"]);
	return cameras;
}

//---------------------------------------------------------------------------------------------------------------------
void trainModel(BoW & _bow, vector<Mat>& _images, Mat & _groundTruth) {
	_bow.train(_images, _groundTruth);
}

//---------------------------------------------------------------------------------------------------------------------
void calculatePointCloud(StereoCameras *_cameras, vector<Point3f> &_cloud, Json &_config) {
	Mat gray1, gray2, _frame1, _frame2;
	bool isBlurry1, isBlurry2;
	cout << "Blurriness: ";
	_frame1 = _cameras->camera(0).frame();
	cvtColor(_frame1, gray1, CV_BGR2GRAY);
	if (_frame1.rows != 0) {
		isBlurry1 = isBlurry(gray1, _config["cameras"]["blurThreshold"]);
	}
	else { return; }

	_frame2 = _cameras->camera(1).frame();
	cvtColor(_frame2, gray2, CV_BGR2GRAY);
	if (_frame2.rows != 0) {
		isBlurry2 = isBlurry(gray2, _config["cameras"]["blurThreshold"]);
	}
	else { return; }
	cout << endl;

	Mat imageTogether;
	if (isBlurry1 || isBlurry2) {
		hconcat(_frame1, _frame2, imageTogether);

		if (isBlurry1)
			putText(imageTogether, "Blurry Image", Point2i(20, 30), FONT_HERSHEY_SIMPLEX, 1.0, Scalar(0, 0, 255), 4);
		if (isBlurry2)
			putText(imageTogether, "Blurry Image", Point2i(20 + _frame1.cols, 30), FONT_HERSHEY_SIMPLEX, 1.0, Scalar(0, 0, 255), 4);

		imshow("StereoViewer", imageTogether);
		return;
	}
	else {
		_frame1 = _cameras->camera(0).undistort(_frame1);
		_frame2 = _cameras->camera(1).undistort(_frame2);
		hconcat(_frame1, _frame2, imageTogether);
		imshow("StereoViewer", imageTogether);

		Rect leftRoi = _cameras->roi(true);
		Rect rightRoi = _cameras->roi(false);

		cvtColor(_frame1, _frame1, CV_BGR2GRAY);
		cvtColor(_frame2, _frame2, CV_BGR2GRAY);
	}

	pair<int, int> disparityRange(_config["cameras"]["disparityRange"]["min"], _config["cameras"]["disparityRange"]["max"]);
	int squareSize = _config["cameras"]["templateSquareSize"];
	int maxReprojectionError = _config["cameras"]["maxReprojectionError"];
	pcl::PointCloud<pcl::PointXYZ> cloud;
	cloud = *_cameras->pointCloud(_frame1, _frame2, disparityRange, squareSize, maxReprojectionError);
	_cloud.clear();
	for (pcl::PointXYZ point : cloud) {
		_cloud.push_back(Point3f(point.x, point.y, point.z));
	}
}