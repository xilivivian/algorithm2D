#pragma once

#include"recognise_function.h"

void recognise_flaw_for_bottom(Mat& image, Mat& img, Mat& saveimg, vector<flawimformation>& flaw_total,
	int& max_compare, int min_threshold, int max_threshold, int position);

bool main_recognise_for_bottom(Mat image, recogn_OUT& OUT);

bool image_ROI_cut(Mat& image, Mat& circle_image, Mat& ring_image);

void deep_recognise_for_bottom(Mat& image_src, Mat& image_mask, Mat& mask, vector<flawimformation>& flaw_total, int gan, int px_threshold, int postion);