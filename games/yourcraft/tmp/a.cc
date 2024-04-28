#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

const char *file = "G:/workspace/souland/ActionRPGStarterGame/Source/ActionRPGGame/Quest/QuestEvent.h";

int main()
{
    std::ifstream            f(file);
    std::vector<std::string> contents;
    bool                     ok = false;

    if (f.fail()) {
        std::cerr << "Failed to open file." << std::endl;
        exit(-1);
    }

    std::string line;
    while (std::getline(f, line))
    {
        if (line.find("UENUM(BlueprintType)") != std::string::npos)
        {
            ok = true;
        }

        if (ok && line.find("{") != std::string::npos)
        {
            continue;
        }

        if (ok)
        {
            if (line.find("};") != std::string::npos)
            {
                break;
            }
            if (!line.empty())
            {
                contents.push_back(line);
            }
        }
    }

    for (auto content : contents)
    {
        // Extract enum value and comment
        size_t equalsPos = content.find('=');
        size_t umetaPos  = content.find("UMETA");
        if (equalsPos != std::string::npos && umetaPos != std::string::npos)
        {
            std::string enumValue = content.substr(equalsPos + 1, umetaPos - equalsPos - 2);
            std::string comment   = content.substr(umetaPos + 18, content.size() - umetaPos - 20);
            std::cout << comment << "=" << enumValue << std::endl;
        }
    }

    return 0;
}
