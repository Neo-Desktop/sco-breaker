#ifndef SB_H_H
#define SB_H_H

/* sb.c */
void usage(bool fail);
_Noreturn void die(const char *msgfmt, ...);

/* md_common.c */
char	*BSCanon(const char *s);
void	extmd(char md[static 7], unsigned int nstr, ...);

/* reg.c */
int	gen_regcode(int argc, char *argv[]);

/* snak.c */
int	gen_snak(int argc, char *argv[]);

/* snakext.c */
void	mdsnakext(const char *serno, const char *actkey, const char *snakext,
		char snakextmd[static 7]);


#endif

