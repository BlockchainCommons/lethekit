
This set of git hooks maintains the state of `seedtool/gitrevision.h`.
In order to enable this feature one must execute the following command
anywhere in the seedtool directory:

    ./enable-gitrevision-hooks.sh

A default `seedtool/gitrevision.h` is checked in so folks who use a
tarball or other means will be able to compile.  It has placeholders
for the defined constants.

The `post-checkout`, `post-commit`, and `post-merge` hooks write the
following current information (samples below) to the
`seedtool/gitrevision.h` file:

```
#define GIT_BRANCH "revinfo"
#define GIT_SHORT_HASH "cdb54bd"
#define GIT_REV_COUNT "29"
#define GIT_LATEST_TAG "v0.1.0"
```

The `pre-commit` hook overwrites any generated `seedtool/gitrevision.h`
with the placeholders prior to commit so the placeholders version is
always checked in.

It is, of couurse, impossible to check in the correct current values
because those values depend on the hash of those value ...
