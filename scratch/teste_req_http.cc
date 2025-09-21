// #include "httplib.h"
// #include <iostream>

// using namespace std;

// int main(){
    
//     httplib::Client cli("www.youtube.com");

//     if (auto res = cli.Get("/")) {
//         cout << "foi?" << endl;
//         cout << res->status << endl;
//         cout << res->get_header_value("Content-Type") << endl;
//         cout << res->body << endl;
//     } else {
//         cout << "error code: " << res.error() << std::endl;
//     }
//     return 0;
// }