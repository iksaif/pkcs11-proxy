#ifndef CONFIG_H
# define CONFIG_H

#ifdef __MINGW32__

# include <stdint.h>
# include <stdlib.h>
# include <limits.h>

typedef uint32_t __uid32_t;
typedef uint32_t __gid32_t;
typedef uint32_t uid_t;
typedef size_t socklen_t;

struct sockaddr_un {
	uint16_t sun_family;
	char sun_path[PATH_MAX];
};

enum  {
	SHUT_RD = 0, /* No more receptions.  */
	SHUT_WR, /* No more transmissions.  */
	SHUT_RDWR /* No more receptions or transmissions.  */
};

#endif

#endif	/* CONFIG_H */
