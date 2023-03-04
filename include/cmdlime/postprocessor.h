#ifndef CMDLIME_POSTPROCESSOR_H
#define CMDLIME_POSTPROCESSOR_H

namespace cmdlime {

template<typename T>
struct PostProcessor {
    void operator()(T&){};
};

} //namespace cmdlime

#endif //CMDLIME_POSTPROCESSOR_H
