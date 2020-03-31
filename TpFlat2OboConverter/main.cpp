/* 
 * File:   main.cpp
 * Author: mueller
 *
 * Created on January 27, 2015, 3:24 PM
 */

#include <string>
#include <iostream>
#include <iomanip>

std::string OboHeader() {
    std::string ret;
    char buff[20];
    time_t t(time(0));
    strftime(buff, 20, "%d:%m:%Y %H:%M", localtime(&t));
    ret = "format-version: 1.2\n";
    ret += "data-version: " + std::string(buff) + "\n";
    ret += "date: " + std::string(buff) + "\n";
    ret += "saved-by: Textpresso\n";
    ret += "auto-generated-by: TpFlat2OboConverter\n";
    return ret;
}

std::string RootTerms(std::string categoryname, std::string idabbrevation) {
    std::string ret;
    ret += "\n[Term]\n";
    ret += "id: " + idabbrevation + ":0000000\n";
    ret += "name: " + categoryname + "\n";
    ret += "\n";
    return ret;
}

int main(int argc, char** argv) {
    if (argc < 3) {
            std::cerr << "Specify category name and id abbreviation!" << std::endl;
            return -1;
        }
    std::string categoryname = std::string(argv[1]);
    std::string idabbrevation = std::string(argv[2]);
    std::cout << OboHeader() << RootTerms(categoryname, idabbrevation);
    int count = 1;
    std::string in;
    while (getline(std::cin, in)) {
        std::cout << "[Term]" << std::endl;
        std::cout << "id: " << idabbrevation << ":";
        std::cout << std::setfill('0') << std::setw(7) << count++ << std::endl;
        std::cout << "name: " << in << std::endl;
        std::cout << "is_a: " << idabbrevation << ":0000000 ! " <<categoryname << std::endl;
        std::cout << std::endl;
    }
    return 0;
}
