#ifndef CHRNOLOG_UNO_Q_H
#define CHRNOLOG_UNO_Q_H

#if defined(CHRONOLOG_PLATFORM_ARDUINO) && defined(CHRONOLOG_UNO_Q)
    struct ThreadMapEntry {
        k_tid_t id;
        const char* name;
    };

    extern      ThreadMapEntry threadsMap[CHRONOLOG_MAX_THREADS];
    extern int  threadsMapCount;

    static inline char *chronolog_strncat(char *dst, const char *src, size_t n) {
        char *d = dst;
        while (*d) d++;

        while (n-- && *src) {
            *d++ = *src++;
        }

        *d = '\0';
        return dst;
    }

    static inline int chronolog_vsnprintf(char *str, size_t size, const char *format, va_list ap) {
        if (!str || size == 0) return 0;
        size_t written = 0;
        char *out = str;
        size_t remaining = size - 1;

        while (*format && remaining > 0) {
            if (*format == '%') {
                format++;
                char buf[32];                                                                           // Temp buffer for numbers
                const char* str_val = buf;
                
                if (*format == 's') {
                    str_val = va_arg(ap, const char *);
                    if (!str_val) str_val = "(null)";
                } else if (*format == 'd' || *format == 'i') {
                    snprintf(buf, sizeof(buf), "%d", va_arg(ap, int));
                } else if (*format == 'u') {
                    snprintf(buf, sizeof(buf), "%u", va_arg(ap, unsigned int));
                } else if (*format == 'x' || *format == 'X') {
                    snprintf(buf, sizeof(buf), "%x", va_arg(ap, unsigned int));
                } else if (*format == 'c') {
                    buf[0] = (char)va_arg(ap, int);
                    buf[1] = '\0';
                } else if (*format == '%') {
                    buf[0] = '%';
                    buf[1] = '\0';
                } else {
                    buf[0] = *format;                                                                   // Unsupported specifier, just print the character
                    buf[1] = '\0';
                }
                
                while (*str_val && remaining > 0) {                                                     // Copy string to output
                    *out++ = *str_val++;
                    remaining--;
                    written++;
                }
            } else {
                *out++ = *format;
                remaining--;
                written++;
            }
            format++;
        }
        *out = '\0';
        return written;
    }

    #define strncat     chronolog_strncat
    #define vsnprintf   chronolog_vsnprintf

#endif // CHRONOLOG_PLATFORM_ARDUINO && CHRONOLOG_UNO_Q

#endif // CHRONOLOG_UNO_Q_H