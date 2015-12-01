///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//		Author:	Pablo Ramon Soria
//		Date:	2015-12-01
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include "EkfImu.h"

//---------------------------------------------------------------------------------------------------------------------
void EkfImu::updateJf(const double _incT) {
	Eigen::Matrix3d I = Eigen::Matrix3d::Identity();
	mJf.block<3, 3>(0, 3) =I*_incT;
	mJf.block<3, 3>(0, 6) =I*_incT*_incT/2;
	mJf.block<3, 3>(3, 6) =I*_incT;
	mJf.block<3, 3>(9, 12) =I*_incT;
}

//---------------------------------------------------------------------------------------------------------------------
void EkfImu::updateHZk() {

}

//---------------------------------------------------------------------------------------------------------------------
void EkfImu::updateJh() {
	mJh(0,3) = 1;
	mJh(1,4) = 1;
}