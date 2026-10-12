# OS Assignment 02 Report

## Feature 2: Long Listing Format (v1.1.0)

### Q1: What is the difference between stat() and lstat()? When is lstat() more appropriate in ls?

stat() follows symbolic links and returns the metadata of the file the link points to. lstat() does not follow the link; it returns the metadata of the link itself (its type is S_IFLNK and st_size is the length of the target path). In ls, lstat() is the right choice because ls -l must show a symlink as a link ("l" type, "link -> target"), not as its target. With stat(), a symlink would look like its target, and a broken symlink would make the call fail with ENOENT. lstat() is also needed later for recursive listing so that symlinked directories are not followed into infinite loops.

### Q2: How can bitwise operators and macros extract file type and permissions from st_mode?

st_mode packs the file type in its upper bits and the special bits (setuid, setgid, sticky) plus nine permission bits (rwx for owner, group, others) in its lower 12 bits. Permissions are tested with bitwise AND: (mode & S_IRUSR) is non-zero if the owner has read permission, and the same applies to S_IWUSR, S_IXGRP, S_IROTH, etc. The file type must first be isolated with the mask S_IFMT and then compared: (mode & S_IFMT) == S_IFDIR. Using only (mode & S_IFDIR) is wrong because type values are not single bits (for example S_IFBLK & S_IFDIR is non-zero). The macros S_ISDIR(), S_ISREG(), S_ISLNK() do this masked comparison internally. My mode_to_string() function uses these macros for the type character and the & operator with S_IRUSR... S_IXOTH for the nine permission characters, and S_ISUID, S_ISGID, S_ISVTX for the s/S and t/T special cases.
