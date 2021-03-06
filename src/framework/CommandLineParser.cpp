#include <CommandLineParser.h>
#include <iostream>
#include <stdexcept>
#include <string.h>

namespace Acidrain {
    vector<string> split(const string& str, char delimiter) {
        vector<string> result;

        istringstream f(str);
        string value;
        while (getline(f, value, delimiter)) {
            result.push_back(value);
        }

        return result;
    }

    void ParseCommandLineFlags(int argc, char** argv) {
        CommandLineParser::instance().parse(argc, argv);
    }

    const string get(const CommandLineOptionValue& value, const CommandLineOptionType& type) {
        stringstream ss;
        switch (type) {
            case CommandLineOptionType::STRING:
                ss << value.stringValue;
                break;
            case CommandLineOptionType::INTEGER:
                ss << value.intValue;
                break;
            case CommandLineOptionType::BOOLEAN:
                ss << std::boolalpha << value.boolValue;
                break;
            default:
                ss << "[unknown command line option type " << (int) type << "]";
        }
        return ss.str();
    }

    ostream& operator<<(ostream& out, const CommandLineOption& option) {
        out << "Option: {";
        out << "shortOption: " << option.shortOption;
        out << ", longOption: " << option.longOption;
        out << ", description: " << option.description;
        out << ", type: " << option.type;
        out << ", specified: " << option.isSpecified;
        out << ", defaultValue: " << get(option.defaultValue, option.type);
        out << ", value: " << get(option.value, option.type);
        out << "}";
        return out;
    }


    CommandLineParser& CommandLineParser::addString(string argumentNames, string description, string defaultValue, void* valueHolder) {
        CommandLineOption option;
        option.description = description;
        option.defaultValue = CommandLineOptionValue(defaultValue);
        option.value = CommandLineOptionValue(defaultValue);
        option.type = CommandLineOptionType::STRING;
        option.valueHolder = valueHolder;

        vector<string> components = split(argumentNames, ',');
        option.longOption = components.at(0);
        if (components.size() > 1)
            option.shortOption = components.at(1);

        options.push_back(option);
        return *this;
    }

    CommandLineParser& CommandLineParser::addInteger(string argumentNames, string description, int defaultValue, void* valueHolder) {
        CommandLineOption option;
        option.description = description;
        option.defaultValue = CommandLineOptionValue(defaultValue);
        option.value = CommandLineOptionValue(defaultValue);
        option.type = CommandLineOptionType::INTEGER;
        option.valueHolder = valueHolder;

        vector<string> components = split(argumentNames, ',');
        option.longOption = components.at(0);
        if (components.size() > 1)
            option.shortOption = components.at(1);

        options.push_back(option);
        return *this;
    }

    CommandLineParser& CommandLineParser::addBool(string argumentNames, string description, bool defaultValue, void* valueHolder) {
        CommandLineOption option;
        option.description = description;
        option.defaultValue = CommandLineOptionValue(defaultValue);
        option.value = CommandLineOptionValue(defaultValue);
        option.type = CommandLineOptionType::BOOLEAN;
        option.valueHolder = valueHolder;

        vector<string> components = split(argumentNames, ',');
        option.longOption = components.at(0);
        if (components.size() > 1)
            option.shortOption = components.at(1);

        options.push_back(option);
        return *this;
    }

    CommandLineParser& CommandLineParser::parse(int argc, char** argv) {
        CommandLineOption* currentOption = nullptr;
        int i = 1;
        while (i < argc) {
            if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h"))
                usage();

            string argument = string(argv[i]);
            vector<string> argumentParts = split(argument, '=');


            currentOption = optionByArgument(argumentParts.at(0).c_str());
            if (currentOption == nullptr) {
                // see if we have positional attributes
            } else {
                bool* boolValueHolder = (bool*) currentOption->valueHolder;
                string* stringValueHolder = (string*) currentOption->valueHolder;
                int* intValueHolder = (int*) currentOption->valueHolder;
                switch (currentOption->type) {
                    case BOOLEAN:
                        if (argumentParts.size() > 1) {
                            currentOption->value.boolValue = argumentParts.at(1) == "true" || argumentParts.at(1) == "1";
                        } else {
                            currentOption->value.boolValue = true;
                        }
                        currentOption->isSpecified = true;
                        *boolValueHolder = currentOption->value.boolValue;
                        break;
                    case STRING:
                        if (argumentParts.size() > 1) {
                            currentOption->value.stringValue = argumentParts.at(1);
                        } else {
                            if ((i + 1) >= argc)
                                throw new runtime_error("Missing value for argument " + currentOption->longOption);

                            currentOption->value.stringValue = argv[++i];
                        }
                        currentOption->isSpecified = true;
                        *stringValueHolder = currentOption->value.stringValue;
                        break;
                    case INTEGER:
                        if (argumentParts.size() > 1) {
                            currentOption->value.intValue = atoi(argumentParts.at(1).c_str());
                        } else {
                            if ((i + 1) >= argc)
                                throw new runtime_error("Missing value for argument " + currentOption->longOption);

                            currentOption->value.intValue = atoi(argv[++i]);
                        }
                        currentOption->isSpecified = true;
                        *intValueHolder = currentOption->value.intValue;
                        break;
                }
                currentOption = nullptr;
            }
            i++;
        }
        return *this;
    }

    CommandLineOption* CommandLineParser::optionByArgument(char const* name) {
        if (name[0] == '-') {
            if (name[1] == '-') {
                return optionByLongName(&name[2]);
            } else {
                return optionByShortName(&name[1]);
            }
        }

        return nullptr;
    }

    CommandLineOption* CommandLineParser::optionByLongName(char const* name) {
        for (auto& o : options)
            if (strcmp(o.longOption.c_str(), name) == 0)
                return &o;

        return nullptr;
    }

    CommandLineOption* CommandLineParser::optionByShortName(char const* name) {
        for (auto& o : options)
            if (strcmp(o.shortOption.c_str(), name) == 0)
                return &o;

        return nullptr;
    }

    CommandLineParser& CommandLineParser::dump() {
        cout << "Options: " << endl;
        for (auto& o : options)
            cout << "\t" << o << endl;
        return *this;
    }

    string CommandLineParser::paramAsString(string name) {
        CommandLineOption* option = optionByName(name);
        if (option == nullptr)
            throw new runtime_error("Unknown parameter " + name);

        return option->value.stringValue;
    }

    int CommandLineParser::paramAsInt(string name) {
        CommandLineOption* option = optionByName(name);
        if (option == nullptr)
            throw new runtime_error("Unknown parameter " + name);

        return option->value.intValue;
    }

    bool CommandLineParser::paramAsBool(string name) {
        CommandLineOption* option = optionByName(name);
        if (option == nullptr)
            throw new runtime_error("Unknown parameter " + name);

        return option->value.boolValue;
    }

    CommandLineOption* CommandLineParser::optionByName(string name) {
        CommandLineOption* option = nullptr;
        option = optionByLongName(name.c_str());
        if (option == nullptr)
            option = optionByShortName(name.c_str());
        return option;
    }

    CommandLineParser& CommandLineParser::instance() {
        static CommandLineParser instance;
        return instance;
    }

    void CommandLineParser::usage() {
        std::cout << "Usage: stardust [OPTION]...\n" << std::endl;

        unsigned long maxLongOptionLength = 0;
        for (auto& o : options)
            if (o.longOption.length() > maxLongOptionLength)
                maxLongOptionLength = o.longOption.length();

        for (auto& o : options) {
            std::cout << "  -" << o.shortOption << ", --" << o.longOption;
            std::cout << std::string(maxLongOptionLength - o.longOption.length(), ' ');
            std::cout << "   " << o.description << ". Defaults to " << get(o.defaultValue, o.type) <<  std::endl;
        }
        std::cout << std::endl;
        exit(1);
    }

    bool CommandLineParser::isArgumentSpecified(string argument) {
        CommandLineOption* option = optionByLongName(argument.c_str());
        return option != nullptr && option->isSpecified;
    }
}
