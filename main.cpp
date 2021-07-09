#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, const char** argv)
{
    Rect roi(40, 100, 270, 270);
    string text1, text2, text3;

    VideoCapture cam(0);
    if (!cam.isOpened()) {
        cout << "ERROR not opened " << endl;
        return -1;
    }

    Mat img;
    Mat img_threshold;
    Mat img_gray;
    Mat img_roi;


    Mat kernel = getStructuringElement(MORPH_ELLIPSE, cv::Size(5, 5), cv::Point(1, 1));


    namedWindow("Original_image");
    namedWindow("Gray_image");
    namedWindow("Thresholded_image");
    namedWindow("ROI");

    char a[40];
    int count = 0;

    while (1) {
        bool b = cam.read(img);
        if (!b) {
            cout << "ERROR : cannot read" << endl;
            return -1;
        }
        flip(img, img, 1);


        img_roi = img(roi);
        rectangle(img, roi.tl(), roi.br(), Scalar(0, 255, 255), 2, 8, 0);

        cvtColor(img_roi, img_gray, COLOR_BGR2HSV);
        inRange(img_gray, Scalar(0, 30, 0), Scalar(15, 255, 255), img_threshold);
        morphologyEx(img_threshold, img_threshold, cv::MORPH_CLOSE, kernel);
        GaussianBlur(img_threshold, img_threshold, Size(19, 19), 0.0, 0);




        vector<vector<Point> >contours;
        vector<Vec4i>hierarchy;
        findContours(img_threshold, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point());

        /*
        // Eliminate too short or too long contours
        int cmin = 100; // minimum contour length
        int cmax = 1000; // maximum contour length
        std::vector<std::vector<cv::Point>>::const_iterator itc = contours.begin();
        while (itc != contours.end() - 1) {
            if (itc->size() - 1 < cmin || itc->size() - 1 > cmax)
                itc = contours.erase(itc);
            else
                ++itc;
        }
        */
        if (contours.size() > 0) {
            size_t indexOfBiggestContour = -1;
            size_t sizeOfBiggestContour = 0;

            for (size_t i = 0; i < contours.size(); i++) {
                if (contours[i].size() > sizeOfBiggestContour) {
                    sizeOfBiggestContour = contours[i].size();
                    indexOfBiggestContour = i;
                }
            }
            vector<vector<int> >hull(contours.size());
            vector<vector<Point> >hullPoint(contours.size());
            vector<vector<Vec4i> >defects(contours.size());
            vector<vector<Point> >defectPoint(contours.size());
            vector<vector<Point> >contours_poly(contours.size());
            Point2f rect_point[4];
            vector<RotatedRect>minRect(contours.size());
            vector<Rect> boundRect(contours.size());
            for (size_t i = 0; i < contours.size(); i++) {
                if (contourArea(contours[i]) > 5000) {
                    convexHull(contours[i], hull[i], true);
                    convexityDefects(contours[i], hull[i], defects[i]);
                    if (indexOfBiggestContour == i) {
                        minRect[i] = minAreaRect(contours[i]);
                        for (size_t k = 0; k < hull[i].size(); k++) {
                            int ind = hull[i][k];
                            hullPoint[i].push_back(contours[i][ind]);
                        }
                        count = 0;

                        for (size_t k = 0; k < defects[i].size(); k++) {
                            if (defects[i][k][3] > 13 * 256) {
                                /*   int p_start=defects[i][k][0];   */
                                int p_end = defects[i][k][1];
                                int p_far = defects[i][k][2];
                                defectPoint[i].push_back(contours[i][p_far]);
                                circle(img_roi, contours[i][p_end], 3, Scalar(0, 255, 0), 3);
                                count++;
                            }

                        }

                        if (count == 1) {
                            text1 = "Number of finger = 1";
                        }
                        else if (count == 2)
                            text1 = "Number of finger = 2";
                        else if (count == 3)
                            text1 = "Number of finger = 3";
                        else if (count == 4)
                            text1 = "Number of finger = 4";
                        else if (count == 5)
                            text1 = "Number of finger = 5";
                        else
                            text1 = "NO FINGER";

                        Size textSize = getTextSize(      //Size는 텍스트의 width, heigh 이 멤버 변수
                            text1,
                            1,      //폰트 종류
                            2,      //확대 축소 비율
                            3, 0);

                        Point textOrg((img.cols - textSize.width) / 2, (img.rows) * 3 / 4);
                        putText(img, text1, textOrg, 1, 2, Scalar(0, 255, 255), 3, 8);

                        drawContours(img_threshold, contours, i, Scalar(255, 255, 0), 2, 8, vector<Vec4i>(), 0, Point());
                        drawContours(img_threshold, hullPoint, i, Scalar(255, 255, 0), 1, 8, vector<Vec4i>(), 0, Point());
                        drawContours(img_roi, hullPoint, i, Scalar(0, 0, 255), 2, 8, vector<Vec4i>(), 0, Point());
                        approxPolyDP(contours[i], contours_poly[i], 3, false);
                        boundRect[i] = boundingRect(contours_poly[i]);
                        rectangle(img_roi, boundRect[i].tl(), boundRect[i].br(), Scalar(255, 0, 0), 2, 8, 0);
                        minRect[i].points(rect_point);
                        for (size_t k = 0; k < 4; k++) {
                            line(img_roi, rect_point[k], rect_point[(k + 1) % 4], Scalar(0, 255, 0), 2, 8);
                        }

                    }
                }

            }

            text2 = "PRESS r:right hand  l: left hand.  esc: to exit.";
            Size textSize2 = getTextSize(      //Size는 텍스트의 width, heigh 이 멤버 변수
                text2,
                1,      //폰트 종류
                1,      //확대 축소 비율
                1, 0);
            Point textOrg2((img.cols - textSize2.width) / 10, 40);
            putText(img, text2, textOrg2, 1, 1, Scalar(0, 255, 255), 2, 8);
            text3 = "Gesture recognition is performed in a YELLOW box.";
            Size textSize3 = getTextSize(      //Size는 텍스트의 width, heigh 이 멤버 변수
                text3,
                1,      //폰트 종류
                1,      //확대 축소 비율
                1, 0);
            Point textOrg3((img.cols - textSize3.width) / 10, 60);
            putText(img, text3, textOrg3, 1, 1, Scalar(0, 255, 255), 2, 8);


            imshow("Original_image", img);
            imshow("Gray_image", img_gray);
            //imshow("Before_morph", img_morph);
            imshow("Thresholded_image", img_threshold);
            imshow("ROI", img_roi);

            //오른손잡이, 왼손잡이 구분
            char ch = waitKey(30);
            switch (ch)
            {
            case 27:
                return -1;
            case 'r':
                roi.x = 340;
                break;

            case 'R':
                roi.x = 340;
                break;

            case 'l':
                roi.x = 40;
                break;

            case 'L':
                roi.x = 40;
                break;

            default:
                break;
            }

        }

    }

    return 0;
}