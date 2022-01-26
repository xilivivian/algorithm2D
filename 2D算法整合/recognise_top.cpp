#include"recognise_top.h"

// 顶部面积筛选阈值
#define TOP_AREA_THRESHOLD 200
// 顶部对比度筛选阈值
#define TOP_COMPARE 50

/// <summary>
/// 顶部识别函数
/// </summary>
/// <param name="image"></param>
/// <param name="img"></param>
/// <param name="saveimg"></param>
/// <param name="flaw_total"></param>
/// <param name="max_compare"></param>
/// <param name="min_threshold"></param>
/// <param name="max_threshold"></param>
/// <param name="position"></param>
void recognise_flaw_for_top(Mat& image, Mat& img, Mat& saveimg, vector<flawimformation>& flaw_total,
	int& max_compare, int min_threshold, int max_threshold, int position)
{

	int image_row = image.rows;
	int image_col = image.cols;

	int min_area = TOP_AREA_THRESHOLD;
	int deep_px = TOP_COMPARE;

	int gan = 10;

	Mat image_RGB;
	cvtColor(image, image_RGB, COLOR_GRAY2BGR);

	Mat image_remove_px_double;
	//remove_px_double(image, image_remove_px_double, min_threshold, max_threshold);
	adapt_remove_px(image, image_remove_px_double, 98, 100, 100 / 10);

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
	deep_recognise_for_top(image, image_remove_small_object, image_deep_recognise, flaw_total, gan, deep_px, position);

	Mat image_3;
	mask_to_image(image_deep_recognise, image, image_3);

	hstack(saveimg, image_3, saveimg);

	img = image_3;
}

/// <summary>
/// 顶部识别主函数
/// </summary>
/// <param name="image_src"></param>
/// <param name="image_mask"></param>
/// <param name="mask"></param>
/// <param name="flaw_total"></param>
/// <param name="gan"></param>
/// <param name="px_threshold"></param>
/// <param name="postion"></param>
void deep_recognise_for_top(Mat& image_src, Mat& image_mask, Mat& mask, vector<flawimformation>& flaw_total, int gan, int px_threshold, int postion)
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

			double area_real = area * 0.000041;

			double length = radius * 2 * 0.006389;

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
/// 顶部ROI区域
/// </summary>
/// <param name="image"></param>
/// <param name="circle_image"></param>
/// <returns></returns>
bool recognise_ROI(Mat image, Mat& circle_image)
{

	int image_row = image.rows;
	int image_col = image.cols;

	vector<int>px_point = get_ravel(image);

	int px_percent = image_row * image_col / 10;

	int px = 255;
	int sum_num = 0;

	while (sum_num < px_percent)
	{
		sum_num += px_point[px];
		px -= 1;
	}

	Mat ROI_binary;
	threshold(image, ROI_binary, px, 255, cv::THRESH_BINARY);

	Point2f center;
	float radius;

	get_circle(ROI_binary, circle_image, center, radius);

	Mat image_roi_1 = image.mul(circle_image);

	int circle_x_min = max(0, int(center.x - radius));
	int circle_x_max = min(image_col, int(center.x + radius));

	int circle_y_min = max(0, int(center.y - radius));
	int circle_y_max = min(image_row, int(center.y + radius));

	Rect rect_circle(circle_x_min, circle_y_min, circle_x_max - circle_x_min, circle_y_max - circle_y_min);

	try
	{
		circle_image = Mat(image_roi_1, rect_circle);
	}
	catch (const std::exception&)
	{
		return false;
	}

	return true;

}

/// <summary>
/// 顶部识别主函数
/// </summary>
/// <param name="image"></param>
/// <param name="OUT"></param>
/// <returns></returns>
bool main_recognise_for_top(Mat& image, recogn_OUT& OUT)
{

	/*Rect rect(1340, 752, 1700 - 1340, 1100 - 752);

	Mat ROI;
	ROI = Mat(image, rect);*/

	Mat ROI = image;

	//show_image(image_gray, 1);

	/*Point2f center;
	center.x = 0;
	center.y = 0;

	int radius = 500;*/

	/*if (!recognise_ROI(image_gray, ROI))
	{
		cout << "分割失败" << endl;
		return false;
	}

	if (ROI.empty())
	{
		return false;
	}*/

	Mat image_res;
	Mat saveimage;

	int max_compare = 0;
	int min_threshold = 2;
	int max_threshold = 2;
	int position = POSITION_TOP;

	recognise_flaw_for_top(ROI, image_res, saveimage, OUT.flaw_total, max_compare, min_threshold, max_threshold, position);

	sort(OUT.flaw_total.begin(), OUT.flaw_total.end(), cmp_for_priority_of_area);

	OUT.image.push_back(image_res);

	OUT.image.push_back(saveimage);

	OUT.max_compare.push_back(max_compare);

	return true;
}



