char ** generator_statenames = (char *[]) {
[0] = "INITIAL",
[1] = "ws",
[2] = "multilinecomment",
[3] = "emptyarg",
[4] = "slash_norefs",
[5] = "slash_dorefs",
[6] = "comma_norefs",
[7] = "comma_dorefs",
[8] = "dot_norefs",
[9] = "dot_dorefs",
[10] = "colon_norefs",
[11] = "colon_dorefs",
[12] = "semicolon_norefs",
[13] = "semicolon_dorefs",
[14] = "braces",
[15] = "braces_norefs",
[16] = "braces_dorefs",
[17] = "brackets",
[18] = "brackets_norefs",
[19] = "brackets_dorefs",
[20] = "parens",
[21] = "parens_norefs",
[22] = "parens_dorefs",
[23] = "angles",
[24] = "angles_norefs",
[25] = "angles_dorefs",
};
int * generator_statenumbers = (int []) {
0,
1,
2,
3,
4,
5,
6,
7,
8,
9,
10,
11,
12,
13,
14,
15,
16,
17,
18,
19,
20,
21,
22,
23,
24,
25,
};
int generator_numstates = sizeof((int []) {
0,
1,
2,
3,
4,
5,
6,
7,
8,
9,
10,
11,
12,
13,
14,
15,
16,
17,
18,
19,
20,
21,
22,
23,
24,
25,
}) / sizeof(int);
