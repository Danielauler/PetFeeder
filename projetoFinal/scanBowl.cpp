#include "scanBowl.h"

using namespace cv;
using namespace std;

bool verifyFood(char *filename){
	
	bool existe_racao;
	int pixeis_racao;
	int pixeis_tigela;
	
	Mat fonte = imread(filename, 1);
	Mat amostras(fonte.rows * fonte.cols, 3, CV_32F);
	for (int y = 0; y < fonte.rows; y++)
		for (int x = 0; x < fonte.cols; x++)
			for (int z = 0; z < 3; z++)
				amostras.at<float>(y + x*fonte.rows, z) = fonte.at<Vec3b>(y, x)[z];
	
	int clusterCount = 4;
	Mat labels;
	int tentativas = 5;
	Mat centros;
	kmeans(amostras, clusterCount, labels, TermCriteria(CV_TERMCRIT_ITER |
CV_TERMCRIT_EPS, 10000, 0.0001), tentativas, KMEANS_PP_CENTERS, centros);

    Mat new_image(fonte.size(), fonte.type());

	for (int y = 0; y < fonte.rows; y++)
		for (int x = 0; x < fonte.cols; x++){
			int cluster_idx = labels.at<int>(y + x*fonte.rows, 0);
			new_image.at<Vec3b>(y, x)[0] = centros.at<float>(cluster_idx, 0);
			new_image.at<Vec3b>(y, x)[1] = centros.at<float>(cluster_idx, 1);
			new_image.at<Vec3b>(y, x)[2] = centros.at<float>(cluster_idx, 2);
		}	
	
	// imshow("clustered image", new_image);
	imwrite("clustered_image.jpg", new_image);
	Mat img = imread("clustered_image.jpg", 0);
	// Contabilizar quantos pixeis estao associados ao label da racao,
	// e quantos correspondem ao label da tigela
	pixeis_racao = 0;
	pixeis_tigela = 0;
	for (int y = 0; y < fonte.rows; y++){
		for (int x = 0; x < fonte.cols; x++){
			int cluster_idx = labels.at<int>(y + x*fonte.rows, 0);
			if (cluster_idx == 1){ // 1 usar o label da racao
				pixeis_racao++;
			}
			if (cluster_idx == 0){ // 2 usar o label da tigela
				pixeis_tigela++;
			}
		}
	}
	cout<<pixeis_racao<<endl;
	cout<<pixeis_tigela<<endl;
	if ((float)((1.0 * pixeis_racao) / (1.0 * pixeis_tigela) > 0.2)){ // esta fazendo a conversao 
                                                          // para float
		existe_racao = true;
	}
	else
		existe_racao = false;
	
	Mat cimg;
	Mat thresh = Mat::zeros(img.size(), img.type());
	medianBlur(img, img, 5);
	cvtColor(img, cimg, COLOR_GRAY2BGR);
	std::vector<Vec3f> circles;
	
	HoughCircles(img, circles, HOUGH_GRADIENT, 1, 500, 100, 30, 200, 450); //
	//change the last two parameters
	//(min_radius & max_radius) to detect larger circles
	
	for (size_t i = 0; i < circles.size(); i++){
		Vec3i c = circles[i];
		ellipse(cimg, Point(c[0], c[1] * 3 / 4), Size(c[2], c[2] * 3 / 4), 0, 0,
		360, Scalar(0, 255, 0), 3, LINE_AA);
		ellipse(thresh, Point(c[0], c[1] * 3 / 4), Size(c[2], c[2] * 3 / 4), 0, 0,
		360, Scalar(255, 255, 255), -1, LINE_AA);
		circle(cimg, Point(c[0], c[1]), 2, Scalar(0, 255, 0), 3, LINE_AA);
	}
	
	//imshow("detected circles", cimg);
	//imshow("threshold img", thresh);
	Mat hist;
	int histSize = 256;
	float range[] = { 0, 256 };
	const float* histRange = { range };
	calcHist(&img, 1, 0, thresh, hist, 1, &histSize, &histRange, true, false);
	
	// Draw hist
	int hist_w = 512; int hist_h = 400;
	int bin_w = cvRound((double)hist_w / histSize);
	Mat histImage(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));
	
	/// Normalize the result to [ 0, histImage.rows ]
	normalize(hist, hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
	
	/// Draw for each channel
	for (int i = 1; i < histSize; i++){
		line(histImage, Point(bin_w*(i - 1), hist_h - cvRound(hist.at<float>(i -
		1))),
		Point(bin_w*(i), hist_h - cvRound(hist.at<float>(i))),
		Scalar(255, 0, 0), 2, 8, 0);
	}	
	
	/// Display
	//namedWindow("calcHist Demo", CV_WINDOW_AUTOSIZE);
	//imshow("calcHist Demo", histImage);
	std::cout << "Mean intensity is: " << mean(hist) << std::endl;
	waitKey();
	return existe_racao;
}
