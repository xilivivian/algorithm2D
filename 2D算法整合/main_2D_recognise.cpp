#include"recognise_top.h"
#include"recognise_middle.h"
#include"recognise_bottom.h"
#include<iostream>
#include<io.h>
#include<direct.h>
#include<time.h>


bool main_recognise_for_2D(Mat& image, recogn_OUT& out, int flat)
{
	if (image.empty())
	{
		return false;
	}

	cvtColor(image, image, COLOR_BGR2GRAY);

	switch (flat)
	{
	case FLAT_BOTTOM:
	{
		return main_recognise_for_bottom(image, out);
	}break;
	case FLAT_TOP:
	{
		return main_recognise_for_top(image, out);
	}break;
	case FLAT_MIDDLE:
	{
		return main_recognise_for_middle(image, out);
	}break;
	default:
		return false;
		break;
	}

	return true;

}


struct recognise_out
{
	int position;
	int type;
	double length;
	double depth;
	double area;
	recognise_out(int position, int type, double length, double depth, double area) :position(position), type(type), length(length), depth(depth), area(area) {}
};


int main()
{

	string address = "./mid_img";

	string savepath = "./dataset";

	if (_access(savepath.c_str(), 0) == -1)
	{
		_mkdir(savepath.c_str());
	}

	// ����������������һ�� recognise_out �б��Ž��
	//vector<recognise_out>out_list;

	clock_t start = clock();

	for (int i = 0; i < 10; i++)
	{
		Mat image = imread(address + "/" + to_string(i) + ".bmp");

		vector<int>max_compare;
		vector<Mat> image_res;
		vector<flawimformation>flaw_total;
		recogn_OUT out(max_compare, image_res, flaw_total);

		if (main_recognise_for_2D(image, out, FLAT_MIDDLE))
		{
			for (int j = 0; j < 3 && j < out.flaw_total.size(); j++)
			{
				printf("�� %d ��ȱ����� %.2f mm^2\t", j, out.flaw_total[j].area);
				printf("�� %d ��ȱ�ݳ��� %.2f mm\t", j, out.flaw_total[j].length);
				printf("�� %d ȱ�ݵ���� %.2f mm\t", j, out.flaw_total[j].depth);
				printf("�� %d ȱ�ݵ�λ�� %d\t", j, out.flaw_total[j].position);
				printf("�� %d ȱ�ݵ�ȱ������ %d\n", j, out.flaw_total[j].type);

				// �����������������Ž��
				/*out_list.push_back(recognise_out{
					out.flaw_total[j].position,
					out.flaw_total[j].type,
					out.flaw_total[j].length,
					out.flaw_total[j].depth,
					out.flaw_total[j].area });*/
			}

			imwrite(savepath + "/" + to_string(out.max_compare[0]) + '_' + to_string(i) + "circle.bmp", out.image[0]);
			imwrite(savepath + "/" + to_string(out.max_compare[0]) + '_' + to_string(i) + "ring.bmp", out.image[1]);
		}
		//sort(out.flaw_total.begin(), out.flaw_total.end(), cmp);
	}

	clock_t end = clock();
	printf("%d s\n", (end - start) / CLOCKS_PER_SEC);
}


//int main()
//{
//
//	string address = "./total_img";
//
//	string savepath = "./dataset";
//
//	if (_access(savepath.c_str(), 0) == -1)
//	{
//		_mkdir(savepath.c_str());
//	}
//
//	clock_t start = clock();
//
//	vector<recogn_OUT>top_list, bottom_list, middle_list;
//
//	vector<int>max_compare;
//	vector<Mat> image_res;
//	vector<flawimformation>flaw_total;
//	recogn_OUT out(max_compare, image_res, flaw_total);
//
//	for (int i = 0; i < 6; i++)
//	{
//		Mat image = imread(address + "/" + to_string(i) + ".bmp");
//
//		if (i == 0)
//		{
//			vector<int>max_compare_bottom;
//			vector<Mat> image_res_bottom;
//			vector<flawimformation>flaw_total_bottom;
//			out = recogn_OUT(max_compare_bottom, image_res_bottom, flaw_total_bottom);
//			main_recognise_for_2D(image, out, 0);
//			bottom_list.push_back(out);
//		}
//
//		else if (i == 1)
//		{
//			vector<int>max_compare_top;
//			vector<Mat> image_res_top;
//			vector<flawimformation>flaw_total_top;
//			out = recogn_OUT(max_compare_top, image_res_top, flaw_total_top);
//			main_recognise_for_2D(image, out, 1);
//			top_list.push_back(out);
//		}
//		else
//		{
//			// ��ͬһ��out���룬���Ա��浽ͬһ��out
//			if (i == 2)
//			{
//				vector<int>max_compare_middle;
//				vector<Mat> image_res_middle;
//				vector<flawimformation>flaw_total_middle;
//				out = recogn_OUT(max_compare_middle, image_res_middle, flaw_total_middle);
//			}
//
//			main_recognise_for_2D(image, out, 2);
//			if (i == 5)
//			{
//				middle_list.push_back(out);
//			}
//		}
//	}
//	// �����е�ȱ����Ϣ���ܵ�flaw_total
//	for (int i = 0; i < 1; i++)
//	{
//		for (int j = 0; j < top_list[i].flaw_total.size(); j++)
//		{
//			flaw_total.push_back(top_list[i].flaw_total[j]);
//		}
//
//		for (int j = 0; j < bottom_list[i].flaw_total.size(); j++)
//		{
//			flaw_total.push_back(bottom_list[i].flaw_total[j]);
//		}
//
//		for (int j = 0; j < middle_list[i].flaw_total.size(); j++)
//		{
//			flaw_total.push_back(middle_list[i].flaw_total[j]);
//		}
//
//	}
//
//	// ������ȱ�ݽ�������
//	sort(flaw_total.begin(), flaw_total.end(), cmp);
//
//	// ���ǰ����ȱ����Ϣ(��������)
//	for (int i = 0; i < 3 && i < flaw_total.size(); i++)
//	{
//		printf("�� %d ��ȱ����� %.2f\t", i, flaw_total[i].area);
//		printf("�� %d ��ȱ�ݳ��� %.2f\t", i, flaw_total[i].length);
//		printf("�� %d ȱ�ݵ���� %.2f\t", i, flaw_total[i].depth);
//		printf("�� %d ȱ�ݵ�λ�� %d\t", i, flaw_total[i].position);
//		printf("�� %d ȱ�ݵ�ȱ������ %d\n", i, flaw_total[i].type);
//	}
//
//	//ȡ��ͼ������(ǰһ���Ǹ��ͻ��Ľ��ͼ����һ����ÿһ���Ĵ���Ĺ���ͼ)
//	for (int i = 0; i < bottom_list[0].image.size(); i++)
//	{
//		if (i < bottom_list[0].image.size() / 2)//���ͼ
//		{
//			imshow("1", bottom_list[0].image[i]);
//			waitKey(0);
//		}
//		else//����ͼ
//		{
//			imshow("1", bottom_list[0].image[i]);
//			waitKey(0);
//		}
//
//		imwrite(savepath + "/" + to_string(out.max_compare[0]) + '_' + to_string(i) + "circle.bmp", out.image[0]);
//		imwrite(savepath + "/" + to_string(out.max_compare[1]) + '_' + to_string(i) + "ring.bmp", out.image[1]);
//	}
//
//
//	// ��ÿ��ȱ�ݵ�ǰ�����
//
//	// ������������ ȱ�⡢���⡢����
//	vector<flawimformation>flaw_total_misrun, flaw_total_fleshiness, flaw_total_cleanliness;
//
//	for (int i = 0; i < flaw_total.size(); i++)
//	{
//		if (flaw_total[i].type == 1)
//		{
//			flaw_total_misrun.push_back(flaw_total[i]);
//		}
//		else if (flaw_total[i].type == 2)
//		{
//			flaw_total_fleshiness.push_back(flaw_total[i]);
//		}
//		else
//		{
//			flaw_total_cleanliness.push_back(flaw_total[i]);
//		}
//	}
//
//	// ��ȱ���������
//	sort(flaw_total_misrun.begin(), flaw_total_misrun.end(), cmp);
//
//	// ���ȱ��ǰ��
//	for (int i = 0; i < 3 && i < flaw_total_misrun.size(); i++)
//	{
//		printf("�� %d ��ȱ����� %.2f\t", i, flaw_total_misrun[i].area);
//		printf("�� %d ��ȱ�ݳ��� %.2f\t", i, flaw_total_misrun[i].length);
//		printf("�� %d ȱ�ݵ���� %.2f\t", i, flaw_total_misrun[i].depth);
//		printf("�� %d ȱ�ݵ�λ�� %d\t", i, flaw_total_misrun[i].position);
//		printf("�� %d ȱ�ݵ�ȱ������ %d\n", i, flaw_total_misrun[i].type);
//	}
//
//
//	clock_t end = clock();
//
//	cout << (end - start) / CLOCKS_PER_SEC;
//	return 0;
//}