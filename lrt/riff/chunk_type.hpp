#ifndef CHUNK_TYPE_HPP
#define CHUNK_TYPE_HPP

#include <string>
#include <vector>
#include "fourcc_type.hpp"

///Endian Type
enum EndianType : bool { ENDIAN_LITTLE, ENDIAN_BIG };

///Tagged Chunk
struct chunk {
    ~chunk() = default;
    chunk() = default;
    chunk(const EndianType t_e, const bool t_r) :
        is_rev(t_r), endian(t_e) {}
    chunk(const chunk &r) = default;
    chunk(chunk &&r) = default;

    chunk& operator=(const chunk &r) = default;
    chunk& operator=(chunk &&r) = default;

    //To make updating easier
    chunk& operator+=(const chunk &r) {
        setChk(r);
        return *this;
    }

    //Get size of fourcc + length + data
    int size(const bool has_size = true) const {
        return 4 + (4 * has_size) + data.size();
    }
    //Check is empty
    bool empty() const { return !endian && !is_rev && frcc == 0 && data.empty(); }
    //Clear chunk
    void clear() { frcc = 0; data.clear(); }
    void clrArr() { data.clear(); }
    //Set chunk stuff
    void setRev(const bool &is_r) { is_rev = is_r; }
    void setEnd(const EndianType &end) { endian = end; }
    void setFcc(const fourcc &fcc) { frcc = fcc; }
    void setFcc(const unsigned &fcc) { setFcc(fourcc{fcc}); }
    void setInt(unsigned int in, unsigned length) { set_int(in, length, data); }
    void setPad(const unsigned length = 1) {
        if (!length) return;
        unsigned char tmp[length] {};
        setArr(tmp, length);
    }
    void setArr(const unsigned char *in, const unsigned &length) {
        setArr(std::vector<unsigned char> (in, in + length));
    }
    void setArr(const std::vector<unsigned char> &in) {
        data.insert(data.end(), in.begin(), in.end());
    }
    void setStr(const std::string &in, const unsigned &length = 0) {
        setArr((unsigned char*)in.c_str(), in.length());
        if (length > in.length()) setPad(length - in.length());
        else data.resize(data.size() - (in.length() - length));
    }
    void setZtr(const std::string &in) {
        setStr(in, in.length() + (in.length() % 2 ? 1 : 0));
        if (data.back() != 0) setPad();
    }
    void setChk(const chunk &in, const bool &has_size = true) {
        set_fcc(in.getFcc(), data);
        setInt(in.size() - 8, (4 * has_size));
        setArr(in.getArr());
    }
    //Get chunk stuff
    bool getRev() const { return is_rev; }
    EndianType getEnd() const { return endian; }
    fourcc getFcc() const { return frcc; }
    std::vector<unsigned char> getArr() const { return data; }
    std::string getZtr() const { return std::string(data.begin(), data.end()); }
    std::vector<unsigned char> getAll(const bool &has_size = true) const {
        std::vector<unsigned char> out;
        set_fcc(frcc, out);
        set_int(data.size(), (4 * has_size), out);
        out.insert(out.end(), data.begin(), data.end());
        return out;
    }

    private:
        EndianType endian;
        bool is_rev;
        fourcc frcc;
        std::vector<unsigned char> data;

        void set_fcc(const fourcc &fcc, std::vector<unsigned char> &out) const {
            unsigned tmp = fcc.getInt(1);
            for (int f = 0; f < 4; ++f) { out.push_back(tmp & 0xFF); tmp >>= 8; }
        }
        void set_int(unsigned int in, unsigned length, std::vector<unsigned char> &out) const {
            while (length--) {
                switch (endian) {
                    case ENDIAN_BIG:
                        out.push_back((in >> (8 * length)) & 0xFF);
                        continue;
                    case ENDIAN_LITTLE:
                        out.push_back(in & 0xFF); in >>= 8;
                        continue;
                }
            }
        }
};


#endif
