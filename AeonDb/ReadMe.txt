AEON DATABASE

Copyright (c) 2011-2017 GridWhale Corporation. All Rights Reserved.

AeonDB is a NoSQL, log-structured merge-tree database which supports
key-value storage, blob file storage, and secondary indices. AeonDB
is designed to work with the Hexarc server architecture and is uses
HexeLisp compatible datatypes and function definitions.

TABLES

The core concept is a TABLE, which consists of an indexed set of
records generally storing a HexeLisp struct (key-value pairs) but also
able to hold any other datatype, including files.

A table can have 0 or more secondary views, which can have a subset
of the primary data and can be indexed by different fields.

KEY FEATURES

* Log-structured merge-tree, allowing for robust implementation resistant
  to corroption (since we never mutate a file).

* Native support for UTF-8, 64-bit integers, and arbitrary precision
  integers.

* Support for file storage.

* Support for stored functions in secondary views, which evaluate a
  field value at record storage time.

* Support for update-if-version-equals to implement optimistic 
  concurrency.

* Support for atomic field mutation commands (e.g., to atomically 
  increment a field value in a record).

* Replicated table backup and automatic fail-over.

FUTURE WORK

* Full-text search support.

* Native unread mark support.

* Support for arbitrary queries.

* Sharded tables (across drives and machines).
