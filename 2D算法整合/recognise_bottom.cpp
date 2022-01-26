#include"recognise_bottom.h"


// 底部面积筛选阈值
#define BOTTOM_AREA_THRESHOLD 300
// 底部对比度筛选阈值
#define BOTTOM_COMPARE 50

/// <summary>
/// 底部识别函数
/// </summary>
/// <param name="image"></param>
/// <param name="img"></param>
/// <param name="saveimg"></param>
/// <param name="flaw_total"></param>
/// <param name="max_compare"></param>
/// <param name="min_threshold"></param>
/// <param name="max_threshold"></param>
/// <param name="position"></param>
void recognise_flaw_for_bottom(Mat& image, Mat& img, Mat& saveimg, vector<flawimformation>& flaw_total,
	int& max_compare, int min_threshold, int max_threshold, int position)
{

	int min_area = BOTTOM_AREA_THRESHOLD;
	int deep_px = BOTTOM_COMPARE;

	int gan = 50;

	Mat image_RGB;
	cvtColor(image, image_RGB, COLOR_GRAY2BGR);

	int image_row = image.rows;
	int image_col = image.cols;

	Mat image_remove_px_double;
	remove_px_double(image, image_remove_px_double, min_threshold, max_threshold);

	//adapt_remove_px(image, image_remove_px_double, 98);

	Mat mask = Mat::ones(image.size(), CV_8UC1);

	circle(mask, Point(image_row / 2, image_col / 2), 50, 0, -1);

	image_remove_px_double = image_remove_px_double.mul(mask);

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
	deep_recognise_for_bottom(image, image_remove_small_object, image_deep_recognise, flaw_total, gan, deep_px, position);

	Mat image_3;
	mask_to_image(image_deep_recognise, image, image_3);

	hstack(saveimg, image_3, saveimg);

	img = image_3;
}

/// <summary>
/// 基于对比度阈值筛选
/// </summary>
/// <param name="image_src"></param>
/// <param name="image_mask"></param>
/// <param name="mask"></param>
/// <param name="flaw_total"></param>
/// <param name="gan"></param>
/// <param name="px_threshold"></param>
/// <param name="postion"></param>
void deep_recognise_for_bottom(Mat& image_src, Mat& image_mask, Mat& mask, vector<flawimformation>& flaw_total, int gan, int px_threshold, int postion)
{

	int image_row = image_src.rows;
	int image_col = image_src.cols;

	vector<vector<Point>>contours;

	vector<Vec4i>hierarchy;

	findContours(image_mask, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE);

	int contour_length = contours.size();

	int px_flaw = 0, px_out = 0;

	int type;

	mask = Mat::zeros(image_mask.size(), CV_8UC1);

	for (int i = 0; i < contour_length; i++)
	{

		Mat draw_image_edge = Mat::zeros(image_mask.size(), CV_8UC1);

		Mat draw_image_out = Mat::zeros(image_mask.size(), CV_8UC1);

		drawContours(draw_image_edge, contours, i, 255, -1);
		drawContours(draw_image_out, contours, i, 255, gan);

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

			double area_real = area * 0.004717;

			double length = radius * 2 * 0.01325;

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
/// 底部ROI获取
/// </summary>
/// <param name="image_src"></param>
/// <param name="circle_image"></param>
/// <param name="ring_image"></param>
/// <returns></returns>
bool image_ROI_cut(Mat& image_src, Mat& circle_image, Mat& ring_image)
{

	Mat image;

	image_src.copyTo(image);

	Mat mask = Mat::zeros(image.size(), CV_8UC1);

	int gan = 60;
	Point2f center;
	center.x = 690.10;
	center.y = 740.13;
	int radius_first = 370;

	circle(mask, center, radius_first + gan, 1, -1);

	circle(mask, center, radius_first, 0, -1);

	int image_row = image.rows;
	int image_col = image.cols;

	int h = image_row;
	int w = image_col;

	Mat ROI_gray = image;

	int ring_radius = 200;

	vector<int>px_point = get_ravel(ROI_gray);

	int px_percent = (h * w) / 10;

	int px = get_percent_px(px_point, px_percent);

	Mat ROI_binary;
	//printf("this px is %d\n", px);

	threshold(image_src, ROI_binary, px, 255, cv::THRESH_BINARY_INV);

	ROI_binary = ROI_binary.mul(mask);

	//show_image(ROI_binary, 2);

	float radius;

	get_circle(ROI_binary, circle_image, center, radius);

	/*if (int(radius) < 380 || int(radius) > 450)
	{
		return false;
	}*/

	center.x = 690.10;
	center.y = 740.13;
	radius = 370;

	//printf("this circle center x: %.2f  y: %.2f\n", center.x, center.y);
	//printf("this radius is %.2f\n", radius);

	Mat image_roi_1 = ROI_gray.mul(circle_image);

	//show_image(image_roi_1, 2);

	int circle_x_min = max(0, int(center.x - radius));
	int circle_x_max = min(w, int(center.x + radius));

	int circle_y_min = max(0, int(center.y - radius));
	int circle_y_max = min(h, int(center.y + radius));

	Rect rect_circle(circle_x_min, circle_y_min, circle_x_max - circle_x_min, circle_y_max - circle_y_min);

	circle_image = Mat(image_roi_1, rect_circle);

	cut_ring_from_image(ROI_gray, ring_image, center, radius, ring_radius);

	return true;
}

/// <summary>
/// 底部识别主函数
/// </summary>
/// <param name="image"></param>
/// <param name="OUT"></param>
/// <returns></returns>
bool main_recognise_for_bottom(Mat image, recogn_OUT& OUT)
{

	/*Rect rect(831, 232, 2288 - 831, 1636 - 232);
	image = Mat(image, rect);*/

	Mat circle_image, ring_image;

	if (!image_ROI_cut(image, circle_image, ring_image))
	{
		cout << "分割失败" << endl;
		return false;
	}

	int position_circle = POSITION_BOTTOM_CIRCLE;
	int position_ring = POSITION_BOTTOM_RING;

	int min_threshold = 3;
	int max_threshold = 3;

	Mat image_circle_res;
	Mat image_ring_res;

	Mat save_circle_image;
	Mat save_ring_image;

	int max_compare_circle = 0;
	int max_compare_ring = 0;

	recognise_flaw_for_bottom(circle_image, image_circle_res, save_circle_image, OUT.flaw_total,
		max_compare_circle, min_threshold, max_threshold, position_circle);

	recognise_flaw_for_bottom(ring_image, image_ring_res, save_ring_image, OUT.flaw_total,
		max_compare_ring, min_threshold, max_threshold, position_ring);

	sort(OUT.flaw_total.begin(), OUT.flaw_total.end(), cmp_for_priority_of_area);

	OUT.image.push_back(image_circle_res);
	OUT.max_compare.push_back(max_compare_circle);

	OUT.image.push_back(image_ring_res);
	OUT.max_compare.push_back(max_compare_ring);

	OUT.image.push_back(save_circle_image);
	OUT.image.push_back(save_ring_image);

	return true;
}