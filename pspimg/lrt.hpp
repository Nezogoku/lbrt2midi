#ifndef LRT_HPP
#define LRT_HPP

#include <string>
#include "sequence/mid_types.hpp"
#include "sequence/mid_shared.hpp"
#include "lrt_types.hpp"


class lbrt : public midi {
    public:
        lbrt(bool debug = false) { this->debug = debug; }
        int setLRT(std::string lrt_file);
        int writeMidi();

    private:
        unsigned int getLeInt(unsigned char *&in, int length);
        
        bool debug;
        std::string basepath;
};

#endif
