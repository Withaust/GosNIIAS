#include <iostream>

#include "server.hpp"
#include "client.hpp"

int main(int p_argc, char* p_argv[])
{
    try
    {
        SetConsoleOutputCP(CP_UTF8);

        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode))
        {
            SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }

        // I assume I will have to explain why I am using system("pause") here...
        // std::cin.get acts as "is key currently being pressed"
        // while system("pause"); acts as "was key pressed right now".
        // Using something like std::cin.get leads to single prolongued button press
        // accidentally skipping finishing text while also closing the entire app

        if (p_argc < 4)
        {
            std::cerr << "Usage:\n"
                << "  Server: " << p_argv[0] << " server <bind_ip> <bind_port>\n"
                << "  Client: " << p_argv[0] << " client <bind_ip> <bind_port> <target_ip> <target_port>\n";
            system("pause");
            std::cout << "Press any key to exit.\n";
            return 1;
        }

        std::string mode = p_argv[1];
        asio::io_context io_context;

        if (mode == "server")
        {
            std::string bind_ip = p_argv[2];
            unsigned short bind_port = static_cast<unsigned short>(std::stoi(p_argv[3]));

            Server server(io_context, bind_ip, bind_port);
            std::cout << "Starting server on " << bind_ip << ":" << bind_port << "\n";
            io_context.run();
        }
        else if (mode == "client")
        {
            if (p_argc < 6)
            {
                std::cerr << "Client mode requires 6 arguments\n";
                system("pause");
                std::cout << "Press any key to exit.\n";
                return 1;
            }

            std::string bind_ip = p_argv[2];
            unsigned short bind_port = static_cast<unsigned short>(std::stoi(p_argv[3]));
            std::string target_ip = p_argv[4];
            unsigned short target_port = static_cast<unsigned short>(std::stoi(p_argv[5]));

            Client client(io_context, bind_ip, bind_port, target_ip, target_port);
            std::cout << "Starting client on " << bind_ip << ":" << bind_port << " targeting " << target_ip << ":" << target_port << "\n";
            io_context.run();
        }
        else
        {
            std::cerr << "Unknown mode: " << mode << "\n";
            system("pause");
            std::cout << "Press any key to exit.\n";
            return 1;
        }
    }
    catch (std::exception& p_exception)
    {
        std::cerr << "Exception: " << p_exception.what() << "\n";
        system("pause");
        std::cout << "Press any key to exit.\n";
        return 1;
    }

    system("pause");
    std::cout << "Press any key to exit.\n";

    return 0;
}
