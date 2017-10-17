//此模块利用已经等到几条标定线线性拟合出中间的深度值，返回一个可table.yml文件
#include "stdafx.h"
#include "indexTable.h"
#include <vector>
//补齐激光线，并生成索引表
void indexTable(Mat& imgMap, int N, string output)
{

	for (int j = 0; j< imgMap.cols; j++)  //从图像中间开始索引
	{
		int num = 0;  //对找到的平面计数，若少于应有的点则跳过该列
		vector<int> PointOfEveryCalplane;
		for (int i = 0; i < imgMap.rows; i++)
		{
			if (imgMap.at<float>(i, j)>1E-8)
			{
				if (imgMap.at<float>(i, j)>200)
					imgMap.at<float>(i, j) = 0.0;
				num++;
				PointOfEveryCalplane.push_back(i);
			}
		}
		if (num == N)//最后一张图
		{
			for (int i = 0; i < N - 1; i++)
			{
				int up = PointOfEveryCalplane[i], next = PointOfEveryCalplane[i + 1];
				float div = (imgMap.at<float>(up, j) - imgMap.at<float>(next, j)) / (up - next);//相邻线之间的斜率
				for (int k = up + 1; k < next; k++)
				{
					imgMap.at<float>(k, j) = imgMap.at<float>(k - 1, j) + div;
				}
			}
		}
	}
	FileStorage fs(output, FileStorage::WRITE);
	fs << "TABLE" << imgMap;
	fs.release();

}