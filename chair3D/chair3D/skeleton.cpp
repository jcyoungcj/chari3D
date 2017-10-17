#include "stdafx.h"
#include "skeleton.h"
//通过查找上下边界，对标定激光线骨架化，并将强度值赋为相应高度
Mat skeleton(Mat& img, float h, int line)
{

	Mat imgRoi = img;
	//imgRoi = imread(filename, 0);
	Mat imgblur;//二值后的图像

	Mat img_sk = Mat::zeros(imgRoi.rows, imgRoi.cols, CV_32FC1);
	threshold(imgRoi, imgblur, 30, 255, CV_THRESH_BINARY);//二值化图像函数,光线比较暗阈值调低--------阈值修改--------
	//blur(imgblur,imgblur,Size(5,5),Point(-1,-1));

	medianBlur(imgblur, imgblur, 3);//图像平滑处理
	//dilate(imgblur,imgblur,(5,5));//腐蚀膨胀处理
	//	imshow("imgblur", imgblur);//----------------kai----------------------
	//	waitKey();

	Point2i L, R; //表示它保存的数值时int类型咯；

	int w = imgRoi.cols, l = imgRoi.rows;//col 竖列

	for (int j = 0; j < w; j++)
	{
		int isLaser = 0;//f
		int up = 0;
		int buttom = 0;
		int line_panduan = 0;//用于匹配line
		for (int i = 0; i < l; i++)
		{

			if (buttom)
				break;

			if (imgblur.at<uchar>(i, j) >30 && !isLaser) //二值图像，灰度阈值，0是黑
			{
				isLaser = 1;
				++line_panduan;
				if (line_panduan == line){ up = i; }
				//up = i;
			}
			else if (imgblur.at<uchar>(i, j) <30 && isLaser)
			{

				if (line_panduan == line){ buttom = i - 1; img_sk.at<float>((up + buttom) / 2, j) = h; break; }
				isLaser = 0;
				//img_sk.at<float>((up + buttom) / 2, j) = h;//传入高度值，将对应的线存入-------------------
				//break;
			}

			//else{
			//	img_sk.at<float>((up + buttom) / 2, j) = 0;
			//}
		}
		if (isLaser)
			R = Point2i((up + buttom) / 2, j);              //确定左右边界
		if (L == Point2i(0, 0) && isLaser)
		{
			L = Point2i((up + buttom) / 2, j);
		}
	}

	//补齐标定激光线
	for (int i = 0; i < L.y; i++)
	{
		img_sk.at<float>(L.x, i) = h;
	}
	for (int i = R.y; i < w; i++)
	{
		img_sk.at<float>(R.x, i) = h;
	}
	//imshow("img_sk",img_sk);
	//waitKey(5);
	return img_sk;
}