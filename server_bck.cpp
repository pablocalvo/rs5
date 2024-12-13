#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/beast/version.hpp>
#include <iostream>
#include <string>
#include <memory>
#include <cstdlib>
//-------------------------
#include <fstream>
#include <vector>
//-------------------------
#include <nlohmann/json.hpp>
#define MAX_LENGHT_LINE     1000;

//-------------------------

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using namespace std;
using tcp = boost::asio::ip::tcp;

enum description_field{country, name, min_wage, max_wage, skill};

std::string builJson(std::vector<std::vector<std::string>> dataRead)
{
int lineNumber = 0;
int index = 0;
string strJson;
nlohmann::json j;
nlohmann::json array = nlohmann::json::array();
string strRet;


std::cout << "Reading rows:" << endl;
for (const auto& row : dataRead) {
    lineNumber++;        
    index = 0;
    strJson = "{";
    for (const auto& cell : row) {

        switch (index)
        {
        case country:
            j["Country"] = cell;
            break;
        case name:
            j["Name"] = cell;
            break;
        case min_wage:
            j["min_wage"] = cell;
            break;
        case max_wage:
            j["max_wage"] = cell;
            break;
        default:    /*skill*/
            array.push_back(cell);
            break;
        }
        index++;
    }
    j["skills"] = array;
    std::string str = j.dump();
    strRet.append(str);
    // strJson.append("}");
    // string list = "Employee" + std::to_string(lineNumber);
    // j[list] = strJson;
    //std::string str = j.dump();
    //std::cout << "------->str = "<< str<< std::endl;
    //j["list"] = { "Cloud", "1001", { 1, 1, 1 } };
    }
        //std::string str = j.dump();
        //std::cout << "------->str = "<< str<< std::endl;
            // j["skills"] = array;
            //     std::string str = j.dump();
                 //std::cout << "<.....................>strRet = " << strRet << std::endl;
    return strRet;
}

std::vector<std::vector<std::string>> read_csv(const std::string& filename, string filter) {
    std::ifstream file(filename);
    std::vector<std::vector<std::string>> data;
    std::string line;

    std::cout << "inside read_csv" << std::endl;
    while (std::getline(file, line)) {
        std::stringstream lineStream(line);
        std::string cell;
        std::vector<std::string> row;

        //cout << "line = " << line << endl;
        auto position = line.find(filter);
        if (position > 1000) //MAX_LENGHT_LINE
        {
            cout << "no se tiene esta linea = " << line << endl;
            continue;
        }
        //cout << "position = " << position << endl;
        while (std::getline(lineStream, cell, ',')) {
            row.push_back(cell);
        }
        data.push_back(row);
    }
    file.close();
    return data;
}


// std::vector<std::vector<std::string>> read_csv(const std::string& filename) {
//     std::ifstream file(filename);
//     std::vector<std::vector<std::string>> data;
//     std::string line;
//     bool match_filter = false;

//     while (std::getline(file, line)) {
//         std::stringstream lineStream(line);
//         std::string cell;
//         std::vector<std::string> row;
// cout << "line = " << line << endl;
// auto position = line.find("Azure");

// if (position > 1000) //MAX_LENGHT_LINE
// {
//     cout << "no se tiene esta linea = " << line << endl;
//     continue;
    
// }
// cout << "position = " << position << endl;
//         while (std::getline(lineStream, cell, ',')) {
//             row.push_back(cell);
//         }

//         data.push_back(row);
//     }

//     file.close();
//     return data;
// }

void write_csv(const std::string& filename, const std::vector<std::vector<std::string>>& data) {
    std::ofstream file(filename);

    for (const auto& row : data) {
        for (size_t i = 0; i < row.size(); ++i) {
            file << row[i];
            if (i < row.size() - 1) {
                file << ",";
            }
        }
        file << "\n";
    }

    file.close();
}

// El manejador de peticiones HTTP
class session : public std::enable_shared_from_this<session> {
public:
    session(tcp::socket socket) : socket_(std::move(socket)) {}

    void run() {
        // Leer la petición HTTP
        http::async_read(socket_, buffer_, request_,
            [self = shared_from_this()](beast::error_code ec, std::size_t) {
                self->on_read(ec);
            });
    }

private:
    void on_read(beast::error_code ec) {
        if (ec) return fail(ec, "read");

        // Procesar la petición
        http::response<http::string_body> res;
        if (request_.method() == http::verb::get) {
            handle_get(res);
        } else if (request_.method() == http::verb::post) {
            handle_post(res);
        } else {
            res.result(http::status::bad_request);
            res.body() = "Unsupported HTTP method";
        }
        cout << "--->h1....." << endl;
        //res.content_length(44);
        cout << "--->h2" << endl;
        //res.prepare_payload();
        cout << "--->h3" << endl;

        // Enviar la respuesta
        http::async_write(socket_, res,
            [self = shared_from_this()](beast::error_code ec, std::size_t) {
                self->on_write(ec);
            });
    }

    void handle_get(http::response<http::string_body>& res) {
        std::vector<std::vector<std::string>> dataRead;
        auto const query = request_.target().to_string();
        auto const param_start = query.find('?');

//auto const first = query.substr(0, param_start);
//auto const second = query.substr(param_start+1, query.length());
//cout << "first = " << first << endl;
//cout << "second = " << second << endl;

//---------
//cout << "query = " << query << endl;
//cout << "param_start = " << param_start << endl;
//---------

        if (param_start != std::string::npos) {
            auto const params = query.substr(param_start + 1);
cout << "params = " << params << endl;   //------
auto const index = params.find('=');         
auto const var = params.substr(0, index);
auto const filter = params.substr(index+1, params.length());
cout << "var = " << var << endl;
cout << "filter = " << filter << endl;

//std::vector<std::vector<std::string>> dataRead = read_csv("database.csv", "England");
 dataRead = read_csv("database.csv", filter);
 string ss = builJson(dataRead);
            res.result(http::status::ok);
            //res.body() = "GET request with parameters: " + params;
            res.body() = "GET request with parameters: " + ss;
        } else {
            res.result(http::status::ok);
            res.body() = "GET request without parameters";
        }
    }

    void handle_post(http::response<http::string_body>& res) {
        auto const body = request_.body();
        cout << "parametros del body = " << body << endl;
        res.result(http::status::ok);
        res.body() = "POST request with body: " + body;
    }

    void on_write(beast::error_code ec) {
        if (ec) return fail(ec, "write");

        // Cerrar el socket
        socket_.shutdown(tcp::socket::shutdown_send, ec);
    }

    static void fail(beast::error_code ec, char const* what) {
        std::cerr << what << ": " << ec.message() << "\n";
    }

    tcp::socket socket_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> request_;
};

// El servidor HTTP
class server {
public:
    server(net::io_context& ioc, tcp::endpoint endpoint)
        : acceptor_(ioc) {
        boost::system::error_code ec;

        acceptor_.open(endpoint.protocol(), ec);
        if (ec) fail(ec, "open");
        //if (ec) return fail(ec, "open");

        acceptor_.set_option(net::socket_base::reuse_address(true), ec);
        if (ec) fail(ec, "set_option");
        //if (ec) return fail(ec, "set_option");

        acceptor_.bind(endpoint, ec);
        if (ec) fail(ec, "bind");
        //if (ec) return fail(ec, "bind");

        acceptor_.listen(net::socket_base::max_listen_connections, ec);
        if (ec) fail(ec, "listen");
        //if (ec) return fail(ec, "listen");

        accept();
    }

private:
    void accept() {
        acceptor_.async_accept(
            [this](beast::error_code ec, tcp::socket socket) {
                if (!ec) {
                    std::make_shared<session>(std::move(socket))->run();
                } else {
                    fail(ec, "accept");
                }
                accept();
            });
    }

    static void fail(beast::error_code ec, char const* what) {
        std::cerr << what << ": " << ec.message() << "\n";
    }

    tcp::acceptor acceptor_;
};

int main(int argc, char* argv[]) {
    try {
        if (argc != 3) {
            std::cerr << "Usage: <port> <address>\n";
            return EXIT_FAILURE;
        }
//-------------------------------------------------------------
    std::vector<std::vector<std::string>> data = {
        {"Name", "Age", "City"},
        {"Alice", "30", "New York"},
        {"Bob", "25", "Los Angeles"},
        {"Charlie", "35", "Chicago"}
    };
    write_csv("output.csv", data);
    std::cout << "CSV file written successfully!" << std::endl;

    // Now we can read the cvs file
    //std::vector<std::vector<std::string>> dataRead = read_csv("database.csv");
    std::vector<std::vector<std::string>> dataRead = read_csv("database.csv", "England");
    std::cout << "Reading rows:" << endl;
    for (const auto& row : dataRead) {
        for (const auto& cell : row) {
            std::cout << cell << " ";
        }
        std::cout << std::endl;
    }


string ss = builJson(dataRead);
std::cout << "ss = " << ss << endl;

//--------------------------------------------------------------
                                        // Crear un objeto JSON
                                        // nlohmann::json jsonObject;
                                        // Añadir datos al objeto JSON
                                        // jsonObject["name"] = "Juan";
                                        // jsonObject["age"] = 30;
                                        // jsonObject["city"] = "Buenos Aires";
                                        // std::string jsonString = jsonObject.dump();
                                        // std::cout << jsonString << std::endl;


        auto const address = net::ip::make_address(argv[2]);
        unsigned short port = static_cast<unsigned short>(std::stoi(argv[1]));

        net::io_context ioc{1};
        server srv{ioc, {address, port}};
        ioc.run();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
