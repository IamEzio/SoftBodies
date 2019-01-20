//
// Created by lirfu on 03.10.18..
//

#ifndef LAB_RACGRA_AOBJPARSER_H
#define LAB_RACGRA_AOBJPARSER_H


#include <cstdio>
#include <stdexcept>

template<typename T>
class AObjParser {
public:
    inline void update(double val, double &container, bool min) {
        if (min && val < container || !min && val > container)
            container = val;
    }

    virtual void parse(const char *cmd, char *args, ulong line)=0;

    virtual std::string to_obj() const =0;

    static T load(std::string &filepath) {
        FILE *f = fopen(filepath.data(), "r");
        if (!f)
            throw std::runtime_error("Cannot open file: " + filepath);
        T obj;
        uint line = 0;
        while (true) {
            ++line;
            char *cmd = new char[255];
            char *args = new char[255];
            int res = fscanf(f, "%s %[^\n]s", cmd, args);
            if (res == EOF)
                break;

            if (res < 2 || *cmd == '#') continue;
            obj.parse(cmd, args, line);
            if (!std::strcmp(cmd, "norm")) {
                double x, y, z;
                if (!sscanf(args, "%lf %lf %lf", &x, &y, &z))
                    throw std::runtime_error("Error reading norm on line " + std::to_string(line));
                obj.set_normal({x, y, z});
            } else if (!std::strcmp(cmd, "lookup")) {
                double x, y, z;
                if (!sscanf(args, "%lf %lf %lf", &x, &y, &z))
                    throw std::runtime_error("Error reading lookup on line " + std::to_string(line));
                obj.set_lookup({x, y, z});
            }
        }
        fclose(f);
        return obj;
    }

    static void write(std::string &filepath, AObjParser &obj) {
        FILE *f = fopen(filepath.data(), "w+");
        if (!f)
            throw std::runtime_error("Cannot open file: " + filepath);
        std::string data = obj.to_obj();
        int res = fprintf(f, data.data());
        if (res < 0)
            perror("Error writing object!");
        fclose(f);
    }
};


#endif //LAB_RACGRA_AOBJPARSER_H