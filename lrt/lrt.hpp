#ifndef LRT_HPP
#define LRT_HPP

#include <string>
#include "mid/mid_shared.hpp"
#include "lrt_types.hpp"
#include "playmidi.hpp"


class lbrt : public midi, public playmidi {
    public:
        lbrt() { lrt_path = lrt_name = ""; }
        int setLRT(std::string lrt_file);
        int writeMidi();

    private:
        std::string lrt_path;
};

#endif
