#pragma once

#include"recognise_function.h"

void recognise_flaw_for_top(Mat& image, Mat& img, Mat& saveimg, vector<flawimformation>& flaw_total,
	int& max_compare, int min_threshold, int max_threshold, int position);

bool main_recognise_for_top(Mat& image, recogn_OUT& OUT);

void deep_recognise_for_top(Mat& image_src, Mat& image_mask, Mat& mask, vector<flawimformation>& flaw_total, int gan, int px_threshold, int postion);

bool recognise_ROI(Mat image, Mat& circle_image);