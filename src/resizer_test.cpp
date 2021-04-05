#include <iostream>
#include <stdio.h>

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int resize_image(Mat ori_image, Mat& new_image, int new_width, int new_height)
{
    int ori_width = ori_image.cols;
    int ori_height = ori_image.rows;
    printf("ori_width : %d , ori_height : %d \n", ori_width, ori_height);
    printf("new_width : %d , new_height : %d \n", new_width, new_height);
    if (new_width <= 0 || new_height <= 0) {
        return 0;
    } else {
        resize(ori_image, new_image, Size(new_width,new_height));
        return 1;        
    }
    
}

int main(int argc, char** argv )
{
    if ( argc != 4 )
    {
        printf("usage: ./nodefluxcv <Image_Path> <New_Width> <New_Height>\n");
        return -1;
    } else {
        Mat image;
        image = imread( argv[1], 1 );
        if ( !image.data )
        {
            printf("No image data \n");
            return -1;
        }

        Mat resized_image;

        resize_image(image, resized_image, atoi(argv[2]), atoi(argv[3]));
        printf("output width : %d , output height : %d \n", resized_image.cols, resized_image.rows);

        namedWindow("Display Image", WINDOW_AUTOSIZE );
        imshow("Display Image", resized_image);
        waitKey(0);
    }
    
    

    

    return 0;
}