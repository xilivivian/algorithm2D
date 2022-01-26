#include"recognise_function.h"

/// <summary>
/// 筛选疑似像素（方法一）
/// </summary>
/// <param name="image_src"></param>
/// <param name="image"></param>
/// <param name="min_precent"></param>
/// <param name="max_precent"></param>
void remove_px_double(Mat& image_src, Mat& image, int min_precent, int max_precent)
{

	vector<int>px_total;

	int px_num = 0;

	statistics(image_src, px_total, px_num);

	int min_num = px_num * min_precent / 100;
	int max_num = px_num * max_precent / 100;

	int min_sum = 0;
	int max_sum = 0;

	int min_px = 1;
	int max_px = 255;

	while (min_sum < min_num)
	{
		min_sum += px_total[min_px];
		min_px += 1;
	}

	while (max_sum < max_num)
	{
		max_sum += px_total[max_px];
		max_px -= 1;
	}

	image = Mat::zeros(image_src.size(), CV_8UC1);

	int image_row = image_src.rows;
	int image_col = image_src.cols;

	for (int i = 0; i < image_row; i++)
	{
		uchar* data = image_src.ptr<uchar>(i);
		for (int j = 0; j < image_col; j++)
		{
			if (data[j] && (data[j]<min_px || data[j]>max_px))
			{
				image.at<uchar>(i, j) = 1;
			}
		}
	}
}

/// <summary>
/// 截取圆形区域（手动）
/// </summary>
/// <param name="image"></param>
/// <param name="ROI"></param>
/// <param name="center"></param>
/// <param name="radius"></param>
/// <returns></returns>
bool cut_circle_from_image(Mat& image, Mat& ROI, Point2f center, int radius)
{

	int image_row = image.rows;
	int image_col = image.cols;

	int x = center.x;
	int y = center.y;

	int ROI_x_min = max(0, x - radius);
	int ROI_y_min = max(0, y - radius);

	int ROI_x_max = min(image_row, x + radius);
	int ROI_y_max = min(image_col, y + radius);

	Mat mask = Mat::zeros(image.size(), CV_8UC1);

	circle(mask, center, radius, 1, -1);

	Mat image_mask = image.mul(mask);

	Rect rect(ROI_x_min, ROI_y_min, ROI_x_max - ROI_x_min, ROI_y_max - ROI_y_min);

	try
	{
		ROI = Mat(image_mask, rect);
	}
	catch (const std::exception&)
	{
		return false;
	}

	return true;
}

/// <summary>
///  截取环形区域（手动）
/// </summary>
/// <param name="image"></param>
/// <param name="image_res"></param>
/// <param name="center"></param>
/// <param name="radius"></param>
/// <param name="ring_radius"></param>
/// <returns></returns>
bool cut_ring_from_image(Mat& image, Mat& image_res, Point2f center, int radius, int ring_radius)
{

	int h = image.rows;
	int w = image.cols;

	int ring_x_min = max(0, int(center.x - radius - ring_radius));
	int ring_x_max = min(w, int(center.x + radius + ring_radius));

	int ring_y_min = max(0, int(center.y - radius - ring_radius));
	int ring_y_max = min(h, int(center.y + radius + ring_radius));

	Mat mask = Mat::zeros(image.size(), CV_8UC1);

	circle(mask, center, int(radius) + ring_radius, 1, -1);

	circle(mask, center, int(radius) + 10, 0, -1);

	Mat ROI = image.mul(mask);

	Rect rect(ring_x_min, ring_y_min, ring_x_max - ring_x_min, ring_y_max - ring_y_min);

	try
	{
		image_res = Mat(ROI, rect);
	}
	catch (const std::exception&)
	{
		return false;
	}



	return true;
}


/// <summary>
///  圆形ROI截取
/// </summary>
/// <param name="image"></param>
/// <param name="image_circle"></param>
/// <param name="center"></param>
/// <param name="radius"></param>
void get_circle(Mat image, Mat& image_circle, Point2f& center, float& radius)
{

	vector<vector<Point>>contours;
	vector<Vec4i>hierarchy;

	int image_row = image.rows;
	int image_col = image.cols;

	findContours(image, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE);

	image_circle = Mat::zeros(image.size(), CV_8UC1);

	Mat image_big = Mat::zeros(image.size(), CV_8UC1);

	int contours_length = contours.size();

	int area_max = 0;
	int contours_id = 0;

	for (int i = 0; i < contours_length; i++)
	{
		int area = contours[i].size();

		if (area > area_max) {
			area_max = area;
			contours_id = i;
			minEnclosingCircle(contours[i], center, radius);
		}
	}

	if (radius <= 35)
	{
		return;
	}

	drawContours(image_big, contours, contours_id, 1, -1);

	circle(image_circle, center, int(radius) - 35, 1, -1);
}

/// <summary>
/// 对像素进行排序
/// </summary>
/// <param name="px"></param>
/// <param name="px_sort"></param>
void argsort(vector<int>& px, vector<int>& px_sort)
{

	px_sort = vector<int>(px.size());

	int px_num = px.size();

	int old_j = 0;

	for (int l = 0; l < px_num; l++)
	{
		int j = old_j;
		for (int i = 0; i < px_num; i++)
		{
			if (px[l] > px[i])
			{
				j += 1;
			}
		}
		if (px_sort[j] != 0)
		{
			while (px_sort[j] != 0)
			{
				j += 1;
			}
		}
		px_sort[j] = l;

	}
}

/// <summary>
/// 筛选疑似像素（方法二）
/// </summary>
/// <param name="image"></param>
/// <param name="image_res"></param>
/// <param name="threshold"></param>
void remove_px(Mat& image, Mat& image_res, int threshold)
{

	int image_row = image.rows;
	int image_col = image.cols;

	image_res = Mat::zeros(image.size(), CV_8UC1);

	vector<int>px_total, px_sort;

	int px_num = 0;

	statistics(image, px_total, px_num);

	argsort(px_total, px_sort);

	int px_max = px_num * threshold / 100;

	int px_i = 255;

	int px_sum = 0;

	while (px_sum < px_max)
	{
		px_sum += px_total[px_sort[px_i]];
		px_i -= 1;
	}

	vector<int>px_list = vector<int>(256, 0);

	for (int i = 0; i < px_i; i++)
	{
		px_list[px_sort[i]] = 1;
	}

	for (int x = 0; x < image_row; x++)
	{
		uchar* data = image.ptr<uchar>(x);
		for (int y = 0; y < image_col; y++)
		{
			if (data[y] && px_list[data[y]])
			{
				image_res.at<uchar>(x, y) = 1;
			}
		}
	}
}

/// <summary>
/// 筛选疑似像素（方法三）
/// </summary>
/// <param name="image"></param>
/// <param name="image_res"></param>
/// <param name="threshold"></param>
/// <param name="box_weight"></param>
/// <param name="step"></param>
void adapt_remove_px(Mat& image, Mat& image_res, int threshold, int box_weight, int step)
{

	int image_row = image.rows;
	int image_col = image.cols;

	int box_hight = box_weight;

	int image_row_ = image_row - box_weight;
	int image_col_ = image_col - box_hight;

	image_res = Mat::zeros(image.size(), CV_8UC1);

	for (int i = 0; i < image_row_; i += step)
	{
		for (int j = 0; j < image_col_; j += step)
		{

			Rect rect(j, i, box_weight, box_hight);

			Mat ROI = Mat(image, rect);

			vector<int>px_total, px_sort;

			int px_num = 0;

			statistics(ROI, px_total, px_num);

			argsort(px_total, px_sort);

			int px_max = px_num * threshold / 100;

			int px_i = 255;

			int px_sum = 0;

			while (px_sum < px_max)
			{
				px_sum += px_total[px_sort[px_i]];
				px_i -= 1;
			}

			vector<int>px_list = vector<int>(256);

			for (int i = 0; i < px_i; i++)
			{
				px_list[px_sort[i]] = 1;
			}

			for (int x = 0; x < box_weight; x++)
			{
				uchar* data = ROI.ptr<uchar>(x);
				for (int y = 0; y < box_hight; y++)
				{
					if (data[y] && px_list[data[y]])
					{
						image_res.at<uchar>(x + i, y + j) = 1;
					}
				}
			}
		}
	}

}

/// <summary>
/// 将标记的位置显示到图片上
/// </summary>
/// <param name="mask"></param>
/// <param name="image_src"></param>
/// <param name="image"></param>
void mask_to_image(Mat& mask, Mat& image_src, Mat& image)
{

	cvtColor(image_src, image, COLOR_GRAY2BGR);

	int image_row = image.rows;
	int image_col = image.cols;

	for (int i = 0; i < image_row; i++)
	{
		uchar* data = mask.ptr<uchar>(i);
		for (int j = 0; j < image_col; j++)
		{
			if (data[j])
			{
				image.at<Vec3b>(i, j)[0] = 0;
				image.at<Vec3b>(i, j)[1] = 255;
				image.at<Vec3b>(i, j)[2] = 0;
			}
		}
	}
}

/// <summary>
/// 对比度筛选
/// </summary>
/// <param name="image_src"></param>
/// <param name="image_mask"></param>
/// <param name="mask"></param>
/// <param name="flaw_total"></param>
/// <param name="gan"></param>
/// <param name="px_threshold"></param>
/// <param name="postion"></param>
/// <param name="area_gan"></param>
/// <param name="length_gan"></param>
void deep_recognise(Mat& image_src, Mat& image_mask, Mat& mask, vector<flawimformation>& flaw_total, int gan, int px_threshold, int postion, double area_gan, double length_gan)
{

	int image_row = image_src.rows;
	int image_col = image_src.cols;

	vector<vector<Point>>contours;

	vector<Vec4i>hierarchy;

	findContours(image_mask, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE);

	int contour_length = contours.size();

	Mat draw_image_edge = Mat::zeros(image_mask.size(), CV_8UC1);

	Mat draw_image_out = Mat::zeros(image_mask.size(), CV_8UC1);

	mask = Mat::zeros(image_mask.size(), CV_8UC1);

	int px_flaw = 0, px_out = 0;

	int type;

	for (int i = 0; i < contour_length; i++)
	{

		drawContours(draw_image_edge, contours, i, 1, -1);
		drawContours(draw_image_out, contours, i, 1, gan);

		bitwise_or(draw_image_edge, draw_image_out, draw_image_out);

		px_flaw = median(image_src, draw_image_edge);

		bitwise_xor(draw_image_edge, draw_image_out, draw_image_out);

		px_out = median(image_src, draw_image_out);

		if (abs(px_flaw - px_out) > px_threshold)
		{
			drawContours(mask, contours, i, 1, -1);

			Point2f center;
			float radius;
			double area = contourArea(contours[i]);
			minEnclosingCircle(contours[i], center, radius);

			//double area_real = area * 0.004717;
			//double length = radius * 2 * 0.01325;
			double area_real = area * area_gan;

			double length = radius * 2 * length_gan;

			if (px_out > 128)// 大白为缺肉
			{
				type = 1;
			}
			else// 大黑为清洁度
			{
				type = 3;
			}

			flaw_total.push_back(flawimformation{ contours[i], center,area_real,length ,postion,type ,0 });
		}
	}
}

/// <summary>
/// 取均值
/// </summary>
/// <param name="image_src"></param>
/// <param name="draw_image_mask"></param>
/// <returns></returns>
int median(Mat& image_src, Mat& draw_image_mask)
{

	int image_row = image_src.rows;
	int image_col = image_src.cols;

	int num = 0;

	vector<int>px;

	for (int i = 0; i < image_row; i++)
	{
		uchar* data = draw_image_mask.ptr<uchar>(i);
		for (int j = 0; j < image_col; j++)
		{
			if (data[j] && image_src.at<uchar>(i, j))
			{
				px.push_back(image_src.at<uchar>(i, j));
			}
		}
	}
	sort(px.begin(), px.end());

	int px_size = px.size();

	if (px_size % 2)
	{
		num = px[px_size / 4 * 3];
	}
	else
	{
		num = (px[px_size / 4 * 3] + px[(px_size / 4 * 3) + 1]) / 2;
	}

	return num;
}

// 取中值
//int median(Mat& image_src, Mat& draw_image_mask)
//{
//
//	int image_row = image_src.rows;
//	int image_col = image_src.cols;
//
//	int num = 0;
//
//	int px_total = 0;
//
//	for (int i = 0; i < image_row; i++)
//	{
//		uchar* data = draw_image_mask.ptr<uchar>(i);
//		for (int j = 0; j < image_col; j++)
//		{
//			if (data[j] && image_src.at<uchar>(i, j))
//			{
//				num += 1;
//				px_total += image_src.at<uchar>(i, j);
//			}
//		}
//	}
//	return (px_total / num);
//}

/// <summary>
/// 像素统计
/// </summary>
/// <param name="image"></param>
/// <param name="px"></param>
/// <param name="num"></param>
void statistics(Mat& image, vector<int>& px, int& num)
{

	px = vector<int>(256, 0);

	int image_row = image.rows;
	int image_col = image.cols;

	for (int i = 0; i < image_row; i++)
	{
		uchar* data = image.ptr<uchar>(i);
		for (int j = 0; j < image_col; j++)
		{
			if (data[j])
			{
				px[data[j]] += 1;
				num += 1;
			}
		}
	}
}

/// <summary>
/// 剔除连通域较小的缺陷
/// </summary>
/// <param name="image"></param>
/// <param name="mask"></param>
/// <param name="area_max"></param>
void remove_small_objects(Mat& image, Mat& mask, int area_max)
{

	vector<vector<Point>>contours;
	vector<Vec4i>hierarchy;

	findContours(image, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE);

	int contours_length = contours.size();

	mask = Mat::zeros(image.size(), CV_8UC1);

	for (int i = 0; i < contours_length; i++)
	{
		int area = contourArea(contours[i]);

		if (area > area_max)
		{
			drawContours(mask, contours, i, 1, -1);
		}
	}
}

/// <summary>
/// 图像拼接（显示给算法看）
/// </summary>
/// <param name="image_src1"></param>
/// <param name="image_src2"></param>
/// <param name="image"></param>
void hstack(Mat image_src1, Mat image_src2, Mat& image)
{

	int totalCols = image_src1.cols + image_src2.cols;
	image = Mat::zeros(image_src1.rows, totalCols, image_src1.type());
	Mat submat = image.colRange(0, image_src1.cols);
	image_src1.copyTo(submat);
	submat = image.colRange(image_src1.cols, totalCols);
	image_src2.copyTo(submat);
}

/// <summary>
/// 将对比度显示（显示给算法看）
/// </summary>
/// <param name="image_src"></param>
/// <param name="mask"></param>
/// <param name="img"></param>
/// <param name="max_compare"></param>
/// <param name="gan"></param>
void show_compare_px(Mat& image_src, Mat mask, Mat& img, int& max_compare, int gan)
{

	cvtColor(image_src, img, COLOR_GRAY2BGR);

	int image_row = image_src.rows;
	int image_col = image_src.cols;

	vector<vector<Point>>contours;

	vector<Vec4i>hierarchy;

	findContours(mask, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE);

	int contour_length = contours.size();

	Mat draw_image_edge = Mat::zeros(image_src.size(), CV_8UC1);

	Mat draw_image_out = Mat::zeros(image_src.size(), CV_8UC1);

	int px_flaw = 0, px_out = 0, value;

	Point2f center;
	float radius;

	int x = 0, y = 0;

	for (int i = 0; i < contour_length; i++)
	{

		drawContours(draw_image_edge, contours, i, 1, -1);

		drawContours(draw_image_out, contours, i, 1, gan);

		bitwise_or(draw_image_edge, draw_image_out, draw_image_out);

		px_flaw = median(image_src, draw_image_edge);

		bitwise_xor(draw_image_edge, draw_image_out, draw_image_out);

		px_out = median(image_src, draw_image_out);

		value = abs(px_flaw - px_out);

		if (value > max_compare)
		{
			max_compare = value;
		}

		minEnclosingCircle(contours[i], center, radius);

		x = center.x;
		y = center.y;

		putText(img, to_string(px_flaw), Point(x, y - 5),
			FONT_HERSHEY_SIMPLEX, 0.4, Scalar(255, 0, 0), 1);

		putText(img, to_string(px_out), Point(x, y + 5),
			FONT_HERSHEY_SIMPLEX, 0.4, Scalar(0, 0, 255), 1);

		drawContours(img, contours, i, Scalar(0, 255, 0), 1);
	}
}

/// <summary>
/// 获得像素百分比（自适应阈值）
/// </summary>
/// <param name="px_point"></param>
/// <param name="px_percent"></param>
/// <returns></returns>
int get_percent_px(vector<int>px_point, int px_percent)
{

	int sum = 0;
	int i = 0;

	while (sum < px_percent)
	{
		sum += px_point[i];
		i++;
	}

	return i;
}

/// <summary>
/// 获取像素部分情况
/// </summary>
/// <param name="image"></param>
/// <returns></returns>
vector<int> get_ravel(Mat& image)
{

	vector<int>px_total(256, 0);

	int image_col = image.cols;
	int image_row = image.rows;

	for (int i = 0; i < image_row; i++)
	{
		uchar* data = image.ptr<uchar>(i);

		for (int j = 0; j < image_col; j++)
		{
			px_total[data[j]] += 1;
		}
	}

	return px_total;
}

/// <summary>
/// 显示图像
/// </summary>
/// <param name="image"></param>
/// <param name="gan"></param>
void show_image(Mat& image, int gan)
{

	Mat image_res;

	resize(image, image_res, Size(image.cols / gan, image.rows / gan), 0, 0);

	imshow("1", image_res);
	waitKey(0);
}

/// <summary>
/// 面积优先排序
/// </summary>
/// <param name="A"></param>
/// <param name="B"></param>
/// <returns></returns>
bool cmp_for_priority_of_area(flawimformation A, flawimformation B)
{
	if (A.area != B.area)//若面积不一样，则输出面积最大的缺陷
	{
		return A.area > B.area;
	}
	else if (A.area == B.area && A.length != B.length)// 若面积一样，长度不一样，则输出长度最大的缺陷
	{
		return A.length > B.length;
	}
	else//若面积一样，长度一样，输出最深的缺陷
	{
		return abs(A.depth) > abs(B.depth);
	}

	return true;
}

/// <summary>
/// 深度优先排序
/// </summary>
/// <param name="A"></param>
/// <param name="B"></param>
/// <returns></returns>
bool cmp_for_priority_of_depth(flawimformation A, flawimformation B)
{
	if (A.depth != B.depth)// 若深度不同，则输出最深的
	{
		return abs(A.depth) > abs(B.depth);
	}
	else if (A.area != B.area)//若深度一样而，面积不一样
	{
		return A.area > B.area;
	}
	else // 若面积一样、深度也一样则，则输出最长的
	{
		return A.length > B.length;
	}

	return true;
}
