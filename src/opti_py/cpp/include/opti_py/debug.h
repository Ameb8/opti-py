namespace debug {

constexpr bool enabled =
#ifndef NDEBUG
    true;
#else
    false;
#endif

template<typename... Args>
inline void log(Args&&... args) {
    if constexpr (enabled) {
        (std::cerr << ... << args) << '\n';
    }
}

} // namespace debug
