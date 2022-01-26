#include"recognise_middle.h"

// 盲区面积筛选阈值
#define MIDDLE_AREA_THRESHOLD 500
// 盲区对比度筛选阈值
#define MIDDLE_COMPARE 5

/// <summary>
/// 盲区识别函数
/// </summary>
/// <param name="image"></param>
/// <param name="img"></param>
/// <param name="saveimg"></param>
/// <param name="flaw_total"></param>
/// <param name="max_compare"></param>
/// <param name="min_threshold"></param>
/// <param name="max_threshold"></param>
/// <param name="position"></param>
void recognise_flaw_for_middle(Mat image, Mat& img, Mat& saveimg, vector<flawimformation>& flaw_total,
	int& max_compare, int min_threshold, int max_threshold, int position)
{

	int min_area = MIDDLE_AREA_THRESHOLD;
	int deep_px = MIDDLE_COMPARE;

	int gan = 10;

	Mat image_RGB;
	cvtColor(image, image_RGB, COLOR_GRAY2BGR);

	Mat image_remove_px_double;
	remove_px_double(image, image_remove_px_double, min_threshold, max_threshold);

	Mat image_1;
	mask_to_image(image_remove_px_double, image, image_1);

	hstack(image_RGB, image_1, saveimg);

	Mat image_remove_small_object;
	remove_small_objects(image_remove_px_double, image_remove_small_object, min_area);

	Mat image_2;
	mask_to_image(image_remove_small_object, image, image_2);

	hstack(saveimg, image_2, saveimg);

	Mat image_compare_px;
	show_compare_px(image, image_remove_small_object, image_compare_px, max_compare, gan);

	hstack(saveimg, image_compare_px, saveimg);

	Mat image_deep_recognise;
	deep_recognise_for_middle(image, image_remove_small_object, image_deep_recognise, flaw_total, gan, deep_px, position);

	Mat image_3;
	mask_to_image(image_deep_recognise, image, image_3);

	hstack(saveimg, image_3, saveimg);

	img = image_3;
}

/// <summary>
/// 盲区对比度筛选函数
/// </summary>
/// <param name="image_src"></param>
/// <param name="image_mask"></param>
/// <param name="mask"></param>
/// <param name="flaw_total"></param>
/// <param name="gan"></param>
/// <param name="px_threshold"></param>
/// <param name="postion"></param>
void deep_recognise_for_middle(Mat& image_src, Mat& image_mask, Mat& mask, vector<flawimformation>& flaw_total, int gan, int px_threshold, int postion)
{

	int image_row = image_src.rows;
	int image_col = image_src.cols;

	vector<vector<Point>>contours;

	vector<Vec4i>hierarchy;

	findContours(image_mask, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE);

	int contour_length = contours.size();

	mask = Mat::zeros(image_mask.size(), CV_8UC1);

	int px_flaw = 0, px_out = 0;

	int type;

	for (int i = 0; i < contour_length; i++)
	{

		Mat draw_image_edge = Mat::zeros(image_mask.size(), CV_8UC1);

		Mat draw_image_out = Mat::zeros(image_mask.size(), CV_8UC1);

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

			double area_real = area * 0.000028;

			double length = radius * 2 * 0.005369;

			if (px_out > 128)// 大白为缺肉
			{
				type = MISRUN;
			}
			else// 大黑为清洁度
			{
				type = CLEAR;
			}

			flaw_total.push_back(flawimformation{ contours[i], center,area_real,length ,postion,type ,0 });
		}
	}
}

/// <summary>
/// 截取盲区ROI区域
/// </summary>
/// <param name="image"></param>
/// <param name="image_res"></param>
/// <param name="ROI_x"></param>
/// <param name="ROI_y"></param>
/// <param name="h"></param>
/// <param name="w"></param>
/// <returns></returns>
bool cut_rectangle_from_image(Mat image, Mat& image_res, int ROI_x, int ROI_y, int h, int w)
{

	Rect rect(ROI_x, ROI_y, h, w);

	image.copyTo(image_res);

	int image_row = image_res.rows;
	int image_col = image_res.cols;

	int min_precent = 90;
	int max_precent = 5;

	vector<int>px_total;

	int px_num = 0;

	statistics(image_res, px_total, px_num);

	int min_num = px_num * min_precent / 100;
	int min_sum = 0;
	int min_px = 1;

	int max_num = px_num * max_precent / 100;
	int max_sum = 0;
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

	//printf("this px is min: %d max: %d \n", min_px, max_px);
	Mat mask_min;
	threshold(image_res, mask_min, 90, 255, THRESH_BINARY);

	//show_image(mask_min, 2);

	rectangle(mask_min, rect, 0, -1);

	vector<vector<Point>>contours_min;

	vector<Vec4i>hierarchy_min;

	findContours(mask_min, contours_min, hierarchy_min, RETR_EXTERNAL, CHAIN_APPROX_NONE);

	int contour_length = contours_min.size();

	int area_min_max = 0;
	int area_min_i = 0;

	for (int i = 0; i < contour_length; i++)
	{
		if (area_min_max < contours_min[i].size())
		{
			area_min_max = contours_min[i].size();
			area_min_i = i;
		}
	}

	Mat mask = Mat::zeros(mask_min.size(), CV_8UC1);

	drawContours(mask, contours_min, area_min_i, Scalar(1), -1);

	//show_image(mask, 2);

	//image_res = image_res.mul(mask);

	vector<vector<Point>>contours_max;

	vector<Vec4i>hierarchy_max;

	int area_max_max = 0;
	int area_max_i = 0;

	Mat mask_max;
	threshold(image_res, mask_max, max_px, 255, THRESH_BINARY);

	findContours(mask_max, contours_max, hierarchy_max, RETR_EXTERNAL, CHAIN_APPROX_NONE);

	contour_length = contours_max.size();

	for (int i = 0; i < contour_length; i++)
	{
		if (area_max_max < contours_max[i].size())
		{
			area_max_max = contours_max[i].size();
			area_max_i = i;
		}
	}

	drawContours(mask, contours_max, area_max_i, Scalar(0), -1);



	image_res = image_res.mul(mask);

	/*Mat mask;

	bitwise_xor(mask_min, mask_max, mask);

	show_image(mask_min, 2);*/

	return true;
}

/// <summary>
/// 盲区识别主函数
/// </summary>
/// <param name="image"></param>
/// <param name="OUT"></param>
/// <returns></returns>
bool main_recognise_for_middle(Mat image, recogn_OUT& OUT)
{

	Mat image_gray = image;

	Mat ROI = image;

	int ROI_x = 500;
	int ROI_y = 0;

	int w = image.cols - ROI_x;
	int h = image.rows - ROI_y;

	if (!cut_rectangle_from_image(image_gray, ROI, ROI_x, ROI_y, w, h))
	{
		cout << "分割失败" << endl;
		return false;
	}

	//show_image(ROI, 2);

	Mat image_res;

	Mat saveimage;

	int max_compare = 0;
	int min_threshold = 2;
	int max_threshold = 2;
	int position = POSITION_MIDDLE;

	recognise_flaw_for_middle(ROI, image_res, saveimage, OUT.flaw_total, max_compare, min_threshold, max_threshold, position);

	sort(OUT.flaw_total.begin(), OUT.flaw_total.end(), cmp_for_priority_of_area);

	OUT.image.push_back(image_res);

	OUT.image.push_back(saveimage);

	OUT.max_compare.push_back(max_compare);

	return true;
}