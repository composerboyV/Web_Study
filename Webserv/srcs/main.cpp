#include <iostream>
#include <string>

#include "configParsing/MainManager.hpp"
#include "configParsing/ParsingManager.hpp"
#include "socket/ConnectionManager.hpp"

int main(int ac, char **av)
{
    std::string     fileName;
    MainManager     myMainManager;
    ParsingManager  myParsingManager(myMainManager);

    if (ac == 1) //기본 설정 파일 
        fileName = std::string("configFile/defaultServerConfig.conf");
    else if (ac == 2) // 인자로 받은 설정 파일 파싱 시작
        fileName = av[1];
    else
    {
        std::cout << "Wrong Argument\n";
        return (1);
    }   

    try 
    {
        myParsingManager.parseConfigFile(fileName);
        myMainManager.fillMissingConfigValues();
        
        //myMainManager.printParsedConfig();
    }
    catch (std::exception &expn)
    {
        std::cerr << expn.what();
        return (1);
    }

    try
    {           
        ConnectionManager   conn(myMainManager);

        conn.epollMainLoop();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return (1);
    }
    return (0);
}