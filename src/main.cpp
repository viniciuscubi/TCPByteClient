#include <string>
#include <iostream>
#include <pigpio.h>
#include <unistd.h>
#include <chrono>
#include <zmq.hpp>
#include <vector>
#include <iomanip>
#include <sstream>
#include <fstream>


int main()
{

    std::vector<int> mea_time{1,5,10,60};
    std::string wifi_freq = "ETHERNET_CABLE_";


    for(int j = 0; j < (int)mea_time.size(); j++){

        std::cout << wifi_freq << "Testing..." << std::endl;
        std::cout << "Packet size: "  << "001" << "Bytes" << std::endl;
        std::cout << "Testing time: " << std::setw(2) << std::setfill('0') << mea_time[j] << "s" << std::endl;

        std::stringstream ss;
        ss << "001" "B_" << wifi_freq << std::setw(2) << std::setfill('0') << mea_time[j] << "s.txt";
            
        std::ofstream file(ss.str());

        if (gpioInitialise() < 0) {
            std::cerr << "Erro ao inicializar a biblioteca pigpio." << std::endl;
            return 1;
        }

        // Defina o número do pino GPIO que você deseja usar (por exemplo, GPIO 18)
        int ledPin = 18;
        bool var = true;

        // Configure o pino GPIO como saída
        gpioSetMode(ledPin, PI_OUTPUT);


        // initialize the zmq context with a single IO thread
        zmq::context_t context{1};

        // construct a REQ (request) socket and connect to interface
        zmq::socket_t socket{context, zmq::socket_type::req};
        socket.connect("tcp://192.168.1.102:5555");

        // set up some static data to send
        std::string data ="1";
        //std::cout << std::fixed << std::setprecision(4);

        file << std::fixed << std::setprecision(4);

        auto start_test = std::chrono::steady_clock::now();
        while(((int)(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start_test).count())) < mea_time[j])//for (auto request_num = 0; request_num > -1; ++request_num) 
        {
            // send the request message
            //std::cout << "Sending Ping " << request_num << "..." << std::endl;
            auto start = std::chrono::steady_clock::now();
            socket.send(zmq::buffer(data), zmq::send_flags::none);
            
            // wait for reply from server
            zmq::message_t reply{};
            socket.recv(reply, zmq::recv_flags::none);
            auto end = std::chrono::steady_clock::now();
            
            //std::cout << reply.size() << " bytes " 
            //<< "time="  
            //<< ((double)(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()))/2000000 
            //<< " ms" << std::endl;

            file << reply.size() << " bytes " 
            << "time="  
            << ((double)(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()))/2000000 
            << " ms" << std::endl;            

            //std::cout << "Received " << reply.to_string(); 
            //std::cout << " (" << request_num << ")";
            //std::cout << std::endl;

            if(reply.to_string().compare("1")){
                data =  "1";
                var = true;
            }else{
                data = "0";
                var = false;
            }

            gpioWrite(ledPin, var);
        }

        file.close();
    }

    return 0;
}