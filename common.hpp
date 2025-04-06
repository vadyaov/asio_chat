#pragma once

/// TODO:
/// create logger
/// use mb google logger as in impala
/// log important things to some logging files
#ifdef DEBUG_BUILD
#define LOG_DEBUG(x) std::cerr << x << std::endl;
#else
#define LOG_DEBUG(x) do {} while(0)
#endif

static const char* port = "5555";