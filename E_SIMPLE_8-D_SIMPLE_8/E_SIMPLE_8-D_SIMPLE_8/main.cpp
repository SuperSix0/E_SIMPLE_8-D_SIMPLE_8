#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <io.h>
#include <time.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>

using namespace std;
using namespace cv;

double alpha = 2.828;
int m;

void getAllFiles(string path, vector<string>& pic)
{
	long long File = 0;
	struct _finddata_t fileinfo;
	string p = path;
	if ((File = _findfirst((p + "\\*").c_str(), &fileinfo)) != -1L)
	{
		do
		{
			if (strstr(fileinfo.name, ".bmp") != NULL
				|| strstr(fileinfo.name, ".BMP") != NULL
				|| strstr(fileinfo.name, ".dib") != NULL
				|| strstr(fileinfo.name, ".jpeg") != NULL
				|| strstr(fileinfo.name, ".jpg") != NULL
				|| strstr(fileinfo.name, ".jpe") != NULL
				|| strstr(fileinfo.name, ".jp2") != NULL
				|| strstr(fileinfo.name, ".png") != NULL
				|| strstr(fileinfo.name, ".pbm") != NULL
				|| strstr(fileinfo.name, ".pgm") != NULL
				|| strstr(fileinfo.name, ".ppm") != NULL
				|| strstr(fileinfo.name, ".sr") != NULL
				|| strstr(fileinfo.name, ".ras") != NULL
				|| strstr(fileinfo.name, ".tiff") != NULL
				|| strstr(fileinfo.name, ".tif") != NULL)		//寻找符合格式的图片文件
				pic.push_back(p + "\\" + fileinfo.name);
		} while (_findnext(File, &fileinfo) == 0);
		_findclose(File);
	}
}

int D_SIMPLE_8(Mat a, vector<Mat> b)
{
	int ans = 0;
	double Z_LC;
	for (int k = 0; k < b.size(); k++)
	{
		Z_LC = 0;
		for (int i = 0; i < a.rows; i++)
		{
			for (int j = 0; j < a.cols; j++)
			{
				Z_LC += a.at<uchar>(i, j) * b[k].at<double>(i, j) / a.rows / a.cols;
			}
		}
		if (Z_LC > 0)
			ans += pow(2, (b.size() - k - 1));
	}
	return ans;
}

int reg(int a)
{
	return a > 255 ? 255 : (a < 0 ? 0 : a);
}

Mat E_SIMPLE_8(Mat src, vector<Mat> watermark,int message)
{
	int height = src.rows;
	int width = src.cols;
	Mat ans(height,width,CV_8UC1);
	int *m;
	double tmp;
	m = (int*)malloc(sizeof(int) * watermark.size());
	for (int k = watermark.size() - 1; k >= 0; k--)
	{
		m[k] = message % 2;
		message /= 2;
	}
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			tmp = 0;
			for (int k = 0; k < watermark.size(); k++)
			{
				tmp += alpha * (2 * m[k] - 1) * watermark[k].at<double>(i, j);
			}
			ans.at<uchar>(i, j) = reg(src.at<uchar>(i, j) + tmp);
		}
	}
	return ans;
}

int main()
{
	string path;
	vector <string> pic;
	vector <Mat> watermark;
	vector <Mat> pictures;

	while (pic.size() == 0)
	{
		cout << "请输入图片所在路径：\n";
		cin >> path;
		getAllFiles(path, pic);
	}
	cout << pic.size() << endl;
	for (int i = 0; i < pic.size(); i++)
		cout << pic[i] << endl;

	for (int i = 0; i < pic.size(); i++)
	{
		Mat p = imread(pic[i]);
		pictures.push_back(p);
	}

	int width, height;
	for (int i = 0; i < pictures.size(); i++)
	{
		width = max(width, pictures[i].cols);
		height = max(height, pictures[i].rows);
	}

	//随机生成1张水印
	for (int k = 0; k < 8; k++)
	{
		Mat_<double>w(height, width);
		randn(w, 0, 1);
		watermark.push_back(w);
	}

	Mat dst;
	//part1:1个水印和所有图片
	
	int message;
	int wrong = 0;
	for (int i = pictures.size() - 1; i >= 0; i--)
	{
		cout << i << endl;
		message = rand() % 256;
		dst = E_SIMPLE_8(pictures[i], watermark,message);
		if (D_SIMPLE_8(pictures[i], watermark) != message)
			wrong++;
	}
	cout << "The detection accuracy is " << (double)wrong / pictures.size() * 100 << "%.\n";
	
	//part2:40张水印和1张图片,选用第25张:lena512.bmp
	wrong = 0;
	for (int i = 0; i < 40; i++)
	{
		cout << i << endl;
		watermark.clear();
		for (int k = 0; k < 8; k++)
		{
			Mat_<double>w(height, width);
			randn(w, 0, 1);
			watermark.push_back(w);
		}
		message = rand() % 256;
		dst = E_SIMPLE_8(pictures[25], watermark, message);
		if (D_SIMPLE_8(pictures[25], watermark) != message)
			wrong++;
	}
	cout << "Part2:The detection accuracy is " << wrong * 2.5 << "%.\n";
}