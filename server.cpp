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
#include <fstream>
#include <vector>
#include <nlohmann/json.hpp>
#define MAX_LENGHT_LINE     1000;
#define DATABASE            "database.csv";

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using namespace std;
using tcp = boost::asio::ip::tcp;

enum description_field{country, name, min_wage, max_wage, skill};


class HelperJson {
    public:
        //HelperJson();
        //void write_csv_append(const std::string& filename, string content);
        //void DeserialiseJson(string body);

        void write_csv_append(const std::string& filename, string content) {
            std::ofstream file(filename, std::ios_base::app);
            file << content;
            file << "\n";
            file.close();
        }
        void DeserialiseJson(string body)
        {
            string str;
            int index = 0;

            auto j3 = nlohmann::json::parse(body);    
            std::string country = j3["Country"];
            std::string name = j3["Name"];
            std::string max_wage = j3["max_wage"];
            std::string min_wage = j3["min_wage"];
            std::vector<std::string> skills = j3["skills"].get<std::vector<std::string>>();
            str = country;
            str.append(",");
            str.append(name);
            str.append(",");
            str.append(max_wage);
            str.append(",");
            str.append(min_wage);
            str.append(",");
            for (const auto& col : skills) {
                if (index++ < skills.size()-1)
                {
                    str.append(col);
                    str.append(",");
                }
                else
                {
                    str.append(col);
                }
            }
            write_csv_append("database.csv", str);
        }

        std::string builJson(std::vector<std::vector<std::string>> dataRead)
        {
            int lineNumber = 0;
            int index = 0;
            string strJson;
            nlohmann::json j;
            nlohmann::json array = nlohmann::json::array();
            string strRet;

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
            }
            return strRet;
        }

        std::vector<std::vector<std::string>> read_csv(const std::string& filename, string filter) {
        std::ifstream file(filename);
        std::vector<std::vector<std::string>> data;
        std::string line;

        while (std::getline(file, line)) {
            std::stringstream lineStream(line);
            std::string cell;
            std::vector<std::string> row;

            auto position = line.find(filter);
            if (position > 1000) //MAX_LENGHT_LINE
            {
                continue;
            }
            while (std::getline(lineStream, cell, ',')) {
                row.push_back(cell);
            }
            data.push_back(row);
        }
        file.close();
        return data;
        }

        void write_csv(const std::string& filename, const std::vector<std::vector<std::string>>& data) 
        {
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
};



/*void DeserialiseJson(string body)
{
    string str;
    int index = 0;

    auto j3 = nlohmann::json::parse(body);    
    std::string country = j3["Country"];
    std::string name = j3["Name"];
    std::string max_wage = j3["max_wage"];
    std::string min_wage = j3["min_wage"];
    std::vector<std::string> skills = j3["skills"].get<std::vector<std::string>>();
    str = country;
    str.append(",");
    str.append(name);
    str.append(",");
    str.append(max_wage);
    str.append(",");
    str.append(min_wage);
    str.append(",");
    for (const auto& col : skills) {
        if (index++ < skills.size()-1)
        {
            str.append(col);
            str.append(",");
        }
        else
        {
            str.append(col);
        }
    }
    write_csv_append("database.csv", str);
}*/



// The HTTP request handler
class session : public std::enable_shared_from_this<session> {
public:
    session(tcp::socket socket) : socket_(std::move(socket)) {}

    void run() {
        // Read HTTP request
        http::async_read(socket_, buffer_, request_,
            [self = shared_from_this()](beast::error_code ec, std::size_t) {
                self->on_read(ec);
            });
    }

private:
    void on_read(beast::error_code ec) {
        if (ec) return fail(ec, "read");

        // Process the request
        http::response<http::string_body> res;
        if (request_.method() == http::verb::get) {
            handle_get(res);
        } else if (request_.method() == http::verb::post) {
            handle_post(res);
        } else {
            res.result(http::status::bad_request);
            res.body() = "Unsupported HTTP method";
        }
        //res.content_length(44);
        //res.prepare_payload();

        // Send the answer
        http::async_write(socket_, res,
            [self = shared_from_this()](beast::error_code ec, std::size_t) {
                self->on_write(ec);
            });
    }

    void handle_get(http::response<http::string_body>& res) {
        HelperJson hj;
        std::vector<std::vector<std::string>> dataRead;
        string strJson;
        auto const query = request_.target().to_string();
        auto const param_start = query.find('?');

        if (param_start != std::string::npos) {
            auto const params = query.substr(param_start + 1);
            cout << "GET request with parameters: " << params << endl;
            auto const index = params.find('=');         
            auto const var = params.substr(0, index);
            auto const filter = params.substr(index+1, params.length());
            dataRead = hj.read_csv("database.csv", filter);
            strJson = hj.builJson(dataRead);
            res.result(http::status::ok);
            res.body() = "GET request with parameters: " + strJson;
        } else {
            res.result(http::status::ok);
            res.body() = "GET request without parameters";
        }
    }

    void handle_post(http::response<http::string_body>& res) {
        HelperJson hj;
        auto const body = request_.body();
        cout << "POS body parameters = " << body << endl;
        hj.DeserialiseJson(body);
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

// The HTTP server
class server {
public:
    server(net::io_context& ioc, tcp::endpoint endpoint)
        : acceptor_(ioc) {
        boost::system::error_code ec;

        acceptor_.open(endpoint.protocol(), ec);
        if (ec) 
            fail(ec, "open");

        acceptor_.set_option(net::socket_base::reuse_address(true), ec);
        if (ec) 
            fail(ec, "set_option");

        acceptor_.bind(endpoint, ec);
        if (ec) 
            fail(ec, "bind");

        acceptor_.listen(net::socket_base::max_listen_connections, ec);
        if (ec) 
            fail(ec, "listen");

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
