#ifndef __NKN_SERIALIZE__
#define __NKN_SERIALIZE__

#include <iostream>
#include <string>
#include <iterator>

#include "include/byteslice.h"

namespace NKN {
using namespace std;

/*************
 * Serialize *
 *************/
template <typename T>
const string Serialize(const T& obj);
template <typename T>
inline const string Serialize(const T* obj)            { return Serialize<T>(*obj); }
template <typename T>
inline const string Serialize(const shared_ptr<T> obj) { return Serialize<T>(*obj); }
template<>
inline const string Serialize<bool>(const bool& v) {
    return v ? string(1, (char)1) : string(1, char(0));
}

/* *****
 * Common Serializer with host-endianness-independent (whatever LE/BE HOST)
 * Serializer '_IntT' data as little endian byte slice, put them into '_Container'
 *   '_IntT' could be any multiple of 8bits integers, both of signed/unsigned
 *   '_Container' could be any STL container(e.g. string,vector,list,deque etc) which element's size==1 (char or unsigned char)
 * ****/
template <typename _IntT, typename _Container=byteSlice>
inline typename enable_if<
                    sizeof(typename _Container::value_type) == 1,  // _Container<_CharT> must only one byte size
                    _Container
                >::type SerializeToLE(const _IntT& i) {
    typedef typename _Container::value_type _CharT;

    _Container out(sizeof(i), 0);    // preallocates enough

    _CharT* p = (_CharT*)&i;   // pointer to lowest byte of input
    for (auto it=out.begin(); it!=out.end(); it++, p++) { // p++ one by one
        *it = *p;
    }
    return out;
}

/* for compatible github.com/nknorg/nkn/common/serialization WriteVarUint */
template <typename _Container=byteSlice>
inline typename enable_if<
                    sizeof(typename _Container::value_type) == 1,  // _Container<_CharT> must only one byte size
                    _Container
                >::type SerializeVarUint(uint64_t i) {
    typedef typename _Container::value_type _CharT;

    _Container out;

    if (i < 0xFD) {
        out.push_back((_CharT)i);
    } else if(i < 0xFFFF) {
        out.push_back((_CharT)0xFD);
        out.append(SerializeToLE<uint16_t>((uint16_t)i));
    } else if(i < 0xFFFFFFFF) {
        out.push_back((_CharT)0xFE);
        out.append(SerializeToLE<uint32_t>((uint32_t)i));
    } else {
        out.push_back((_CharT)0xFF);
        out.append(SerializeToLE<uint64_t>((uint64_t)i));
    }
    return out;
}

template <template<typename...>class Container, class CharT, class Traits=char_traits<CharT>>
const string Serialize(const Container<CharT>& ctn) {
    ostringstream os;
    os << SerializeVarUint((uint64_t)ctn.size());
    std::copy(ctn.begin(), ctn.end(), ostream_iterator<char>(os));
    return os.str();
}
};  // namespace NKN
#endif  // __NKN_SERIALIZE__
