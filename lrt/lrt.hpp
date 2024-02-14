#ifndef LRT_HPP
#define LRT_HPP

#include <string>
#include "playmidi.hpp"
#include "mid_shared.hpp"
#include "lrt_types.hpp"


class lbrt : public midi, public playmidi {
    public:
        lbrt() { lrt_path = ""; }
        int setLRT(std::string lrt_file);
        int writeMidi();

    private:
        std::string lrt_path;
};

#endif
