#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

class CSVRow {
public:
    string const &operator[](size_t index) const {
        return row[index];
    }


    void readNextRow(istream &str) {
        string line;
        getline(str, line);

        stringstream lineStream(line);
        string cell;

        row.clear();
        while (getline(lineStream, cell, ',')) {
            row.push_back(cell);
        }
        if (!lineStream && cell.empty()) {
            row.push_back("");
        }
    }

private:
    vector<string> row;
};

istream &operator>>(istream &str, CSVRow &data) {
    data.readNextRow(str);
    return str;
}

struct Pixel {
    long double color[4];
};

long double distance(int x1, int x2, int y1, int y2) {
    return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

int main() {
    int width = 0, height = 0;
    ifstream file1("general.txt");
    string s;
    getline(file1, s, '=');
    getline(file1, s, '=');
    width = stoi(s);
    getline(file1, s, '=');
    height = stoi(s);
    file1.close();

    Mat output(height, width, CV_8UC4);
    vector<vector<long double>> total_weight;
    for (int i = 0; i < height; i++) {
        total_weight.push_back(vector<long double>((unsigned long) width, 0));
    }

    vector<int> left, top, center_x, center_y;
    vector<string> names;
    vector<Mat> images;

    ifstream file2("images.csv");
    CSVRow row;
    file2 >> row;
    while (file2 >> row) {
        names.push_back(row[0] + ".png");
        left.push_back(stoi(row[1]));
        top.push_back(stoi(row[2]));
        center_x.push_back(stoi(row[3]));
        center_y.push_back(stoi(row[4]));
    }
    file2.close();

    for (int i = 0; i < names.size(); i++) {
        String name = names[i];
        images.push_back(imread(name, IMREAD_UNCHANGED));
        Mat &img = images[i];
        for (int y = top[i]; y < top[i] + img.rows; y++) {
            for (int x = left[i]; x < left[i] + img.cols; x++) {
                Vec4b color = img.at<Vec4b>(Point(x - left[i], y - top[i]));
                if (color[3]) {
                    long double dist = distance(x, center_x[i], y, center_y[i]);
                    total_weight[y][x] += 1 / (dist * dist * dist);
                }
            }
        }
    }

    vector<vector<Pixel>> newImage;
    for (int i = 0; i < height; i++) {
        newImage.push_back(vector<Pixel>((unsigned long) width, Pixel()));
    }

    for (int i = 0; i < names.size(); i++) {
        String name = names[i];
        Mat &img = images[i];
        for (int y = top[i]; y < top[i] + img.rows; y++) {
            for (int x = left[i]; x < left[i] + img.cols; x++) {
                Vec4b color = img.at<Vec4b>(Point(x - left[i], y - top[i]));
                if (color[3]) {
                    newImage[y][x].color[3] = color[3];
                    long double dist = distance(x, center_x[i], y, center_y[i]);
                    for (int c = 0; c < 3; c++) {
                        newImage[y][x].color[c] +=
                                (1 / (dist * dist * dist)) / total_weight[y][x] * color[c];
                    }
                }
            }
        }
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Vec4b color;
            for (int c = 0; c < 4; c++)
                color[c] = (uchar) newImage[y][x].color[c];
            output.at<Vec4b>(Point(x, y)) = color;
        }
    }

    imwrite("result.png", output);
    return 0;
}