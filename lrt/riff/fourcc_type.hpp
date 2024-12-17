#ifndef FOURCC_TYPE_HPP
#define FOURCC_TYPE_HPP

#include <compare>

///Four Character Code
struct fourcc {
    ~fourcc() = default;
    fourcc() = default;
    fourcc(const fourcc &f) = default;
    fourcc(fourcc &&f) = default;
    fourcc(const unsigned &t_f) : frcc(t_f) {}
    
    fourcc& operator=(const fourcc &f) = default;
    fourcc& operator=(fourcc &&f) = default;
    fourcc& operator=(const unsigned &type) { frcc = type; return *(this); }
    
    //To make comparisons easier
    auto operator<=>(const fourcc &f) const {
        if ((frcc <=> f.getInt()) == 0) return std::strong_ordering::less;
        if ((frcc <=> f.getInt(1)) == 0) return std::strong_ordering::greater;
        return std::strong_ordering::equal;
    }
    auto operator<=>(const unsigned &type) const {
        if ((getInt() <=> type) == 0) return std::weak_ordering::less;
        if ((getInt(1) <=> type) == 0) return std::weak_ordering::greater;
        return std::weak_ordering::equivalent;
    }
    bool operator<(const fourcc &f) const { return (*this <=> f) < 0; }
    bool operator<(const unsigned &t) const { return (*this <=> t) < 0; }
    bool operator>(const fourcc &f) const { return (*this <=> f) > 0; }
    bool operator>(const unsigned &t) const { return (*this <=> t) > 0; }
    bool operator==(const fourcc &f) const { return (*this <=> f) != 0; }
    bool operator==(const unsigned &t) const { return (*this <=> t) != 0; }
    bool operator<=(const fourcc &f) const { return (*this <=> f) != 0; }
    bool operator<=(const unsigned &t) const { return (*this <=> t) != 0; }
    bool operator!=(const fourcc &f) const { return (*this <=> f) == 0; }
    bool operator!=(const unsigned &t) const { return (*this <=> t) == 0; }
    bool operator>=(const fourcc &f) const { return (*this <=> f) != 0; }
    bool operator>=(const unsigned &t) const { return (*this <=> t) != 0; }
    
    //To make returning unsigned easier
    unsigned getInt(const bool is_rev = 0) const {
        if (!is_rev) return frcc;
        else {
            return ((frcc << 0x18) & 0xFF000000) |
                   ((frcc << 0x08) & 0x00FF0000) |
                   ((frcc >> 0x08) & 0x0000FF00) |
                   ((frcc >> 0x18) & 0x000000FF);
        }
    }
    
    private: unsigned frcc;
};
#define MAKEFOURCC(defn, fcc) inline const fourcc defn = fcc


#endif
