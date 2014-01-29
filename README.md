mcchunktools
============

A C library to interface directly with chunks in minecraft region files. And a small number of applications that make use of this library. Besides region files this also gives you an easy interface to the level.dat files. Documentation can be found over at [http://schoentoon.github.io/mcchunktools](http://schoentoon.github.io/mcchunktools).

cleanup_regionfile
==================

A small little tool used to zero out unneeded data from your regionfiles. Why would you want this you ask? To have more predictable data to have better compression over your backups. As an example, on a 1.2GB world this were the results when creating a .tar.gz.

```
With this tool, 687M (703068 bytes)
Without this tool, 732M (748636 bytes)
```

This generally won't provide much advantage when running it on a live system, it is just to make your backups slightly smaller. The zero'd out worlds will load and run just fine, although there may be some issues with dimensions from modded servers, use it with caution on those.
