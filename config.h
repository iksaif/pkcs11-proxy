#ifndef CONFIG_H
# define CONFIG_H

#ifdef __MINGW32__

# include <stdint.h>
# include <stdlib.h>
# include <limits.h>
# include <winsock2.h>

typedef uint32_t __uid32_t;
typedef uint32_t __gid32_t;
typedef uint32_t uid_t;
typedef int socklen_t;

struct sockaddr_un {
	uint16_t sun_family;
	char sun_path[PATH_MAX];
};

enum  {
	SHUT_RD = 0, /* No more receptions.  */
	SHUT_WR, /* No more transmissions.  */
	SHUT_RDWR /* No more receptions or transmissions.  */
};

#ifdef  __MINGW32__
static inline int inet_aton(const char * cp, struct in_addr *pin)
{
        int rc = inet_addr(cp);
        if (rc == -1 && strcmp(cp, "255.255.255.255"))
                return 0;

        pin->s_addr = rc;
        return 1;
}
#endif

#endif

#endif	/* CONFIG_H */
