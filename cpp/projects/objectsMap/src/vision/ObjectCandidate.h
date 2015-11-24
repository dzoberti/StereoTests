//
//
//
//
//


#ifndef OBJECTCANDIDATE_H_
#define OBJECTCANDIDATE_H_

#include <pcl/PointIndices.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <opencv2/opencv.hpp>

class ObjectCandidate
{
public:
	ObjectCandidate(pcl::PointIndices _pointIndeces, pcl::PointCloud<pcl::PointXYZ>::Ptr _cloud, bool _copyCloudPoints);
	ObjectCandidate(pcl::PointCloud<pcl::PointXYZ>::Ptr _cloud = pcl::PointCloud<pcl::PointXYZ>::Ptr());
	~ObjectCandidate();

	/// Add a view to it's history
	void addView(cv::Mat _view);

	/// Get point cloud
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud() const;

	unsigned R() const;
	unsigned G() const;
	unsigned B() const;


	void addView(cv::Mat _view, std::vector<double> _probs);
	std::pair<int, double>  cathegory() const;

private:
	void calculateLabelsStatistics();

private:
	pcl::PointIndices mPointIndices;
	pcl::PointCloud<pcl::PointXYZ>::Ptr mCloud;
	unsigned mR, mG, mB;

	std::vector<cv::Mat>				mViewHistory;
	std::vector<std::vector<double>>	mLabelsHistory;
	std::vector<std::pair<double, double>> mLabelsStatistics;
};

#endif
