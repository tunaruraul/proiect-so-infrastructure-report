# This is a file where I write down the questions I have or where I'm not fully understanding things

## Permissions
| File / Directory     | Permissions | Meaning in this simulation                                      |
|---------------------|------------|-----------------------------------------------------------------|
| District directory  | rwxr-x--- (750) | Managers have full access; inspectors read and execute only     |
| reports.dat         | rw-rw-r-- (664) | Both roles may read; both may write (append)                    |
| district.cfg        | rw-r----- (640) | Managers may read and write; inspectors may read                |
| logged_district     | rw-r--r-- (644) | Anyone may read; only the manager role may write                |

This is a bit unclear to me how the permissions are working, we have 3 types user group and other \
The problem has 2 users manager and inspector, I believe that the user is manager which has more permissions, then would the inspector user fall in the others section or is it a group? \
Solved the problem in permissions.c categorizing it as a group for now
