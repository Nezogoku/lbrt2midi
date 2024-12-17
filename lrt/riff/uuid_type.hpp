#ifndef UUID_TYPE_HPP
#define UUID_TYPE_HPP

#include <cstdio>
#include <vector>

///128bit Universally Unique Identifier
struct uuid {
    unsigned g0;
    unsigned short g1, g2;
    unsigned char g3[8] {};

    ~uuid() = default;
    uuid(const unsigned t0 = 0, const unsigned short t1 = 0, const unsigned short t2 = 0) :
        g0(t0), g1(t1), g2(t2) {}
    uuid(const unsigned t0, const unsigned short t1, const unsigned short t2, const unsigned long long t3) :
        uuid{t0, t1, t2} { for (int g = 8; g > 0; --g) g3[g - 1] = (t3 >> ((8 - g) * 8)) & 0xFF; }
    uuid(const unsigned t0, const unsigned short t1, const unsigned short t2, const unsigned char *t3, const unsigned tZ) :
        uuid{t0, t1, t2} { for (int g = 0; g < (tZ > 8) ? 8 : tZ; ++g) g3[g] = t3[g]; }
    template <int tZ>
    uuid(const unsigned t0, const unsigned short t1, const unsigned short t2, const unsigned char (&t3)[tZ]) :
        uuid{t0, t1, t2, t3, tZ} {}
    uuid(const unsigned char *tarr) :
        uuid{} {
        for (int g = 0; g < 4; ++g) g0 |= (unsigned)*(tarr++) << (g * 8);
        for (int g = 0; g < 2; ++g) g1 |= (unsigned short)*(tarr++) << (g * 8);
        for (int g = 0; g < 2; ++g) g2 |= (unsigned short)*(tarr++) << (g * 8);
        for (auto &g : g3) g = *(tarr++);
    }
    uuid(const uuid &u) : uuid{u.g0, u.g1, u.g2, u.g3} {}
    uuid(uuid &&u) : uuid{u} {}

    uuid& operator=(const uuid &u) {
        g0 = u.g0;
        g1 = u.g1;
        g2 = u.g2;
        for (int g = 0; g < 8; ++g) g3[g] = u.g3[g];
        return *this;
    }
    uuid& operator=(uuid &&u) { return (*this = u); }

    //To make comparing easier
    auto operator<=>(const uuid &u) const {
        if (auto cmp = g0 <=> u.g0; cmp != 0) return cmp;
        if (auto cmp = g1 <=> u.g1; cmp != 0) return cmp;
        if (auto cmp = g2 <=> u.g2; cmp != 0) return cmp;
        for (int g = 0; g < 8; ++g) {
            if (auto cmp = g3[g] <=> u.g3[g]; cmp != 0) return cmp;
        }
        return std::strong_ordering::equal;
    }
    bool operator<(const uuid &u) const = default;
    bool operator>(const uuid &u) const = default;
    bool operator==(const uuid &u) const = default;
    bool operator<=(const uuid &u) const = default;
    bool operator!=(const uuid &u) const = default;
    bool operator>=(const uuid &u) const = default;

    //To make formatted printing easier (%s)
    const char* c_str() const {
        static char gs[47] {};
        snprintf(
            gs, 47,
            "0x%08X 0x%04X 0x%04X 0x%02X%02X 0x%02X%02X%02X%02X%02X%02X",
            g0, g1, g2, g3[0], g3[1], g3[2], g3[3], g3[4], g3[5], g3[6], g3[7]
        );
        return gs;
    }
};
#define MAKEUUID(defn, t0, t1, t2, ...) inline const uuid defn { t0, t1, t2 __VA_OPT__(,) __VA_ARGS__ }


#endif
