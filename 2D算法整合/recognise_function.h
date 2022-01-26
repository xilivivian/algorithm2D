#pragma once

#ifndef RECOGNISE_FUNCTION_H

#include<vector>
#include<opencv2/opencv.hpp>
#include<iostream>
#include<algorithm>
#include<string>
#include<string.h>
#include<stdlib.h>

#endif // !RECOGNISE_FUNCTION_H

// 缺肉缺陷标识
#define MISRUN 1

// 清洁度缺陷标识
#define CLEAR 3

// 底部圆形区域
#define POSITION_BOTTOM_CIRCLE 6
// 环形区域
#define POSITION_BOTTOM_RING 7
// 盲区区域
#define POSITION_MIDDLE 9
// 顶部区域
#define POSITION_TOP 8
// 调用底部识别标识符
#define FLAT_BOTTOM 0
// 调用顶部识别标识符
#define FLAT_TOP 1
// 调用盲区识别标识符
#define FLAT_MIDDLE 2


using namespace std;
using namespace cv;

struct flawimformation
{

	/// <summary>
	/// contour 缺陷的连通域信息
	/// center 缺陷的中心坐标 x，y
	/// area 缺陷的面积
	/// length 缺陷的长度
	/// position 缺陷的位置
	/// type 缺陷类型
	/// depth 缺陷深度
	/// </summary>
	vector<Point>contour;

	Point2f center;

	double area;
	double length;
	int position;
	int type;
	double depth;

	flawimformation(vector<Point>contour, Point2f center, double area,
		double length, int position, int type, double depth) :contour(contour), center(center), area(area), length(length), position(position), type(type), depth(depth) {}
};


struct recogn_OUT
{

	/// <summary>
	/// max_compare 最大对比度
	/// image 输出结果图，前一半是单张结果图片，后半张是过程结果图片
	/// flaw_toal 所有缺陷信息
	/// </summary>
	vector<int> max_compare;

	vector<Mat> image;

	vector<flawimformation>flaw_total;

	recogn_OUT(vector<int> max_compare, vector<Mat> image, vector<flawimformation>flaw_total) :max_compare(max_compare), image(image), flaw_total(flaw_total) {}
};


void show_image(Mat& image, int gan);

void statistics(Mat& image, vector<int>& px, int& num);

void show_compare_px(Mat& image_src, Mat mask, Mat& img, int& max_compare, int gan);

int median(Mat& image_src, Mat& draw_image_mask);

void remove_small_objects(Mat& image, Mat& mask, int area_max);

void remove_px(Mat& image, Mat& image_res, int threshold);

void remove_px_double(Mat& image_src, Mat& image, int min_precent, int max_precent);

void mask_to_image(Mat& mask, Mat& image_src, Mat& image);

void argsort(vector <int>& px, vector<int>& px_sort);

vector<int> get_ravel(Mat& image);

int get_percent_px(vector<int>px_point, int px_percent);

void adapt_remove_px(Mat& image, Mat& image_res, int threshold, int box_weight, int step);

void hstack(Mat image_src1, Mat image_src2, Mat& image);

void deep_recognise(Mat& image_src, Mat& image_mask, Mat& mask, vector<flawimformation>& flaw_total, int gan, int px_threshold, int postion, double area_gan, double length_gan);

void get_circle(Mat image, Mat& image_circle, Point2f& center, float& radius);

bool cut_circle_from_image(Mat& image, Mat& ROI, Point2f center, int radius);

bool cut_ring_from_image(Mat& image, Mat& image_res, Point2f center, int radius, int ring_radius);

bool cmp_for_priority_of_depth(flawimformation A, flawimformation B);

bool cmp_for_priority_of_area(flawimformation A, flawimformation B);