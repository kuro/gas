Title: Gas
use_numbered_headers: true

Gas is an extensible binary container format.

| Link                | URL                                |
| ------------------- | ---------------------------------- |
| Home Page           | <http://gas.gekkoware.net>         |
| Official Repository | <http://github.com/kuro7/gas/tree> |

------------------------------------------------------------------------------

*Table of contents:*

* Table of contents
{:toc}

------------------------------------------------------------------------------

Uses
====

Just a few ideas.

- Network Communication Protocol
- Archive Format
- Multimedia Containers (images, video, audio, etc.)

Consider gas for a network protocol.  For a minimal increase in bandwidth, it becomes possible to specify a protocol that can adapt as necessary.  Need a new field?  Add a new attribute.  And of course, adding an attribute does not break preexisting implementations.

Design Goals
============

Extensibility
-------------

The design philosophy necessitates that any use of gas define and adhere to its own conventions.  In fact, gas has *no* knowledge of typical data types, such as floats, strings, or [halfs][OpenEXR].  Instead, everything from chunk IDs, to attribute keys and values, to payloads, are all treated as opaque binary blobs.  The advantage, however, is that agreeing upon a field's data type allows the application to quickly graft data into place.  Furthermore, as architectures evolve over time, gas should remain relevant as long as computing systems remain binary and byte based.

[OpenEXR]: http://openexr.com

Portability
-----------

Gas uses a given platform's long type as its unsigned number format.  This allows for the potential for overflow when a 64bit machine passes a large yet valid number to a peer where the largest data type is only 2 bytes.  However, this is were application engineering comes in.

Also, there is no notion of endian conventions.  Instead, number encoding and decoding is performed using bit masking and bit shifting.

Number Encoding
===============

The gas format has only one data type.  It uses a number encoding format that uses the minimum space required.  However, there is no upper limit to a number's size.  That is to say, any given implementation is limited only by the maximum size of its long data type.

Possible gas encoding lengths for a 64bit architecture:

    value 0 to  2^7-2 - 1xxx xxxx
    value 0 to 2^14-2 - 01xx xxxx  xxxx xxxx
    value 0 to 2^21-2 - 001x xxxx  xxxx xxxx  xxxx xxxx
    value 0 to 2^28-2 - 0001 xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx
    value 0 to 2^35-2 - 0000 1xxx  xxxx xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx
    value 0 to 2^42-2 - 0000 01xx  xxxx xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx
    value 0 to 2^49-2 - 0000 001x  xxxx xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx
    value 0 to 2^56-2 - 0000 0001  xxxx xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx
    value 0 to 2^63-2 - 0000 0000  1xxx xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx
    value 0 to 2^70-2 - 0000 0000  01xx xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx

Big Number Potential
--------------------

Consider [Ruby][].  The standard ruby interpreter seamlessly converts a Fixnum to a Bignum, both subclasses of Integer, as number size necessitates.  Since the pure ruby version of gas does not care whether it is working with a Fixnum or a Bignum, there is no limit to a gas encoded number when a big number implementation is involved.

[ruby]: http://ruby-lang.org

<!-- vim: set ft=mkd : -->
