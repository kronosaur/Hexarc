
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     IDENT = 258,
     UIDENT = 259,
     FCONST = 260,
     SCONST = 261,
     USCONST = 262,
     BCONST = 263,
     XCONST = 264,
     Op = 265,
     ICONST = 266,
     PARAM = 267,
     TYPECAST = 268,
     DOT_DOT = 269,
     COLON_EQUALS = 270,
     EQUALS_GREATER = 271,
     LESS_EQUALS = 272,
     GREATER_EQUALS = 273,
     NOT_EQUALS = 274,
     ABORT_P = 275,
     ABSOLUTE_P = 276,
     ACCESS = 277,
     ACTION = 278,
     ADD_P = 279,
     ADMIN = 280,
     AFTER = 281,
     AGGREGATE = 282,
     ALL = 283,
     ALSO = 284,
     ALTER = 285,
     ALWAYS = 286,
     ANALYSE = 287,
     ANALYZE = 288,
     AND = 289,
     ANY = 290,
     ARRAY = 291,
     AS = 292,
     ASC = 293,
     ASENSITIVE = 294,
     ASSERTION = 295,
     ASSIGNMENT = 296,
     ASYMMETRIC = 297,
     ATOMIC = 298,
     AT = 299,
     ATTACH = 300,
     ATTRIBUTE = 301,
     AUTHORIZATION = 302,
     BACKWARD = 303,
     BEFORE = 304,
     BEGIN_P = 305,
     BETWEEN = 306,
     BIGINT = 307,
     BINARY = 308,
     BIT = 309,
     BOOLEAN_P = 310,
     BOTH = 311,
     BREADTH = 312,
     BY = 313,
     CACHE = 314,
     CALL = 315,
     CALLED = 316,
     CASCADE = 317,
     CASCADED = 318,
     CASE = 319,
     CAST = 320,
     CATALOG_P = 321,
     CHAIN = 322,
     CHAR_P = 323,
     CHARACTER = 324,
     CHARACTERISTICS = 325,
     CHECK = 326,
     CHECKPOINT = 327,
     CLASS = 328,
     CLOSE = 329,
     CLUSTER = 330,
     COALESCE = 331,
     COLLATE = 332,
     COLLATION = 333,
     COLUMN = 334,
     COLUMNS = 335,
     COMMENT = 336,
     COMMENTS = 337,
     COMMIT = 338,
     COMMITTED = 339,
     COMPRESSION = 340,
     CONCURRENTLY = 341,
     CONFIGURATION = 342,
     CONFLICT = 343,
     CONNECTION = 344,
     CONSTRAINT = 345,
     CONSTRAINTS = 346,
     CONTENT_P = 347,
     CONTINUE_P = 348,
     CONVERSION_P = 349,
     COPY = 350,
     COST = 351,
     CREATE = 352,
     CROSS = 353,
     CSV = 354,
     CUBE = 355,
     CURRENT_P = 356,
     CURRENT_CATALOG = 357,
     CURRENT_DATE = 358,
     CURRENT_ROLE = 359,
     CURRENT_SCHEMA = 360,
     CURRENT_TIME = 361,
     CURRENT_TIMESTAMP = 362,
     CURRENT_USER = 363,
     CURSOR = 364,
     CYCLE = 365,
     DATA_P = 366,
     DATABASE = 367,
     DAY_P = 368,
     DEALLOCATE = 369,
     DEC = 370,
     DECIMAL_P = 371,
     DECLARE = 372,
     DEFAULT = 373,
     DEFAULTS = 374,
     DEFERRABLE = 375,
     DEFERRED = 376,
     DEFINER = 377,
     DELETE_P = 378,
     DELIMITER = 379,
     DELIMITERS = 380,
     DEPENDS = 381,
     DEPTH = 382,
     DESC = 383,
     DETACH = 384,
     DICTIONARY = 385,
     DISABLE_P = 386,
     DISCARD = 387,
     DISTINCT = 388,
     DO = 389,
     DOCUMENT_P = 390,
     DOMAIN_P = 391,
     DOUBLE_P = 392,
     DROP = 393,
     EACH = 394,
     ELSE = 395,
     ENABLE_P = 396,
     ENCODING = 397,
     ENCRYPTED = 398,
     END_P = 399,
     ENUM_P = 400,
     ESCAPE = 401,
     EVENT = 402,
     EXCEPT = 403,
     EXCLUDE = 404,
     EXCLUDING = 405,
     EXCLUSIVE = 406,
     EXECUTE = 407,
     EXISTS = 408,
     EXPLAIN = 409,
     EXPRESSION = 410,
     EXTENSION = 411,
     EXTERNAL = 412,
     EXTRACT = 413,
     FALSE_P = 414,
     FAMILY = 415,
     FETCH = 416,
     FILTER = 417,
     FINALIZE = 418,
     FIRST_P = 419,
     FLOAT_P = 420,
     FOLLOWING = 421,
     FOR = 422,
     FORCE = 423,
     FOREIGN = 424,
     FORWARD = 425,
     FREEZE = 426,
     FROM = 427,
     FULL = 428,
     FUNCTION = 429,
     FUNCTIONS = 430,
     GENERATED = 431,
     GLOBAL = 432,
     GRANT = 433,
     GRANTED = 434,
     GREATEST = 435,
     GROUP_P = 436,
     GROUPING = 437,
     GROUPS = 438,
     HANDLER = 439,
     HAVING = 440,
     HEADER_P = 441,
     HOLD = 442,
     HOUR_P = 443,
     IDENTITY_P = 444,
     IF_P = 445,
     ILIKE = 446,
     IMMEDIATE = 447,
     IMMUTABLE = 448,
     IMPLICIT_P = 449,
     IMPORT_P = 450,
     IN_P = 451,
     INCLUDE = 452,
     INCLUDING = 453,
     INCREMENT = 454,
     INDEX = 455,
     INDEXES = 456,
     INHERIT = 457,
     INHERITS = 458,
     INITIALLY = 459,
     INLINE_P = 460,
     INNER_P = 461,
     INOUT = 462,
     INPUT_P = 463,
     INSENSITIVE = 464,
     INSERT = 465,
     INSTEAD = 466,
     INT_P = 467,
     INTEGER = 468,
     INTERSECT = 469,
     INTERVAL = 470,
     INTO = 471,
     INVOKER = 472,
     IS = 473,
     ISNULL = 474,
     ISOLATION = 475,
     JOIN = 476,
     KEY = 477,
     LABEL = 478,
     LANGUAGE = 479,
     LARGE_P = 480,
     LAST_P = 481,
     LATERAL_P = 482,
     LEADING = 483,
     LEAKPROOF = 484,
     LEAST = 485,
     LEFT = 486,
     LEVEL = 487,
     LIKE = 488,
     LIMIT = 489,
     LISTEN = 490,
     LOAD = 491,
     LOCAL = 492,
     LOCALTIME = 493,
     LOCALTIMESTAMP = 494,
     LOCATION = 495,
     LOCK_P = 496,
     LOCKED = 497,
     LOGGED = 498,
     MAPPING = 499,
     MATCH = 500,
     MATCHED = 501,
     MATERIALIZED = 502,
     MAXVALUE = 503,
     MERGE = 504,
     METHOD = 505,
     MINUTE_P = 506,
     MINVALUE = 507,
     MODE = 508,
     MONTH_P = 509,
     MOVE = 510,
     NAME_P = 511,
     NAMES = 512,
     NATIONAL = 513,
     NATURAL = 514,
     NCHAR = 515,
     NEW = 516,
     NEXT = 517,
     NFC = 518,
     NFD = 519,
     NFKC = 520,
     NFKD = 521,
     NO = 522,
     NONE = 523,
     NORMALIZE = 524,
     NORMALIZED = 525,
     NOT = 526,
     NOTHING = 527,
     NOTIFY = 528,
     NOTNULL = 529,
     NOWAIT = 530,
     NULL_P = 531,
     NULLIF = 532,
     NULLS_P = 533,
     NUMERIC = 534,
     OBJECT_P = 535,
     OF = 536,
     OFF = 537,
     OFFSET = 538,
     OIDS = 539,
     OLD = 540,
     ON = 541,
     ONLY = 542,
     OPERATOR = 543,
     OPTION = 544,
     OPTIONS = 545,
     OR = 546,
     ORDER = 547,
     ORDINALITY = 548,
     OTHERS = 549,
     OUT_P = 550,
     OUTER_P = 551,
     OVER = 552,
     OVERLAPS = 553,
     OVERLAY = 554,
     OVERRIDING = 555,
     OWNED = 556,
     OWNER = 557,
     PARALLEL = 558,
     PARAMETER = 559,
     PARSER = 560,
     PARTIAL = 561,
     PARTITION = 562,
     PASSING = 563,
     PASSWORD = 564,
     PLACING = 565,
     PLANS = 566,
     POLICY = 567,
     POSITION = 568,
     PRECEDING = 569,
     PRECISION = 570,
     PRESERVE = 571,
     PREPARE = 572,
     PREPARED = 573,
     PRIMARY = 574,
     PRIOR = 575,
     PRIVILEGES = 576,
     PROCEDURAL = 577,
     PROCEDURE = 578,
     PROCEDURES = 579,
     PROGRAM = 580,
     PUBLICATION = 581,
     QUOTE = 582,
     RANGE = 583,
     READ = 584,
     REAL = 585,
     REASSIGN = 586,
     RECHECK = 587,
     RECURSIVE = 588,
     REF_P = 589,
     REFERENCES = 590,
     REFERENCING = 591,
     REFRESH = 592,
     REINDEX = 593,
     RELATIVE_P = 594,
     RELEASE = 595,
     RENAME = 596,
     REPEATABLE = 597,
     REPLACE = 598,
     REPLICA = 599,
     RESET = 600,
     RESTART = 601,
     RESTRICT = 602,
     RETURN = 603,
     RETURNING = 604,
     RETURNS = 605,
     REVOKE = 606,
     RIGHT = 607,
     ROLE = 608,
     ROLLBACK = 609,
     ROLLUP = 610,
     ROUTINE = 611,
     ROUTINES = 612,
     ROW = 613,
     ROWS = 614,
     RULE = 615,
     SAVEPOINT = 616,
     SCHEMA = 617,
     SCHEMAS = 618,
     SCROLL = 619,
     SEARCH = 620,
     SECOND_P = 621,
     SECURITY = 622,
     SELECT = 623,
     SEQUENCE = 624,
     SEQUENCES = 625,
     SERIALIZABLE = 626,
     SERVER = 627,
     SESSION = 628,
     SESSION_USER = 629,
     SET = 630,
     SETS = 631,
     SETOF = 632,
     SHARE = 633,
     SHOW = 634,
     SIMILAR = 635,
     SIMPLE = 636,
     SKIP = 637,
     SMALLINT = 638,
     SNAPSHOT = 639,
     SOME = 640,
     SQL_P = 641,
     STABLE = 642,
     STANDALONE_P = 643,
     START = 644,
     STATEMENT = 645,
     STATISTICS = 646,
     STDIN = 647,
     STDOUT = 648,
     STORAGE = 649,
     STORED = 650,
     STRICT_P = 651,
     STRIP_P = 652,
     SUBSCRIPTION = 653,
     SUBSTRING = 654,
     SUPPORT = 655,
     SYMMETRIC = 656,
     SYSID = 657,
     SYSTEM_P = 658,
     TABLE = 659,
     TABLES = 660,
     TABLESAMPLE = 661,
     TABLESPACE = 662,
     TEMP = 663,
     TEMPLATE = 664,
     TEMPORARY = 665,
     TEXT_P = 666,
     THEN = 667,
     TIES = 668,
     TIME = 669,
     TIMESTAMP = 670,
     TO = 671,
     TRAILING = 672,
     TRANSACTION = 673,
     TRANSFORM = 674,
     TREAT = 675,
     TRIGGER = 676,
     TRIM = 677,
     TRUE_P = 678,
     TRUNCATE = 679,
     TRUSTED = 680,
     TYPE_P = 681,
     TYPES_P = 682,
     UESCAPE = 683,
     UNBOUNDED = 684,
     UNCOMMITTED = 685,
     UNENCRYPTED = 686,
     UNION = 687,
     UNIQUE = 688,
     UNKNOWN = 689,
     UNLISTEN = 690,
     UNLOGGED = 691,
     UNTIL = 692,
     UPDATE = 693,
     USER = 694,
     USING = 695,
     VACUUM = 696,
     VALID = 697,
     VALIDATE = 698,
     VALIDATOR = 699,
     VALUE_P = 700,
     VALUES = 701,
     VARCHAR = 702,
     VARIADIC = 703,
     VARYING = 704,
     VERBOSE = 705,
     VERSION_P = 706,
     VIEW = 707,
     VIEWS = 708,
     VOLATILE = 709,
     WHEN = 710,
     WHERE = 711,
     WHITESPACE_P = 712,
     WINDOW = 713,
     WITH = 714,
     WITHIN = 715,
     WITHOUT = 716,
     WORK = 717,
     WRAPPER = 718,
     WRITE = 719,
     XML_P = 720,
     XMLATTRIBUTES = 721,
     XMLCONCAT = 722,
     XMLELEMENT = 723,
     XMLEXISTS = 724,
     XMLFOREST = 725,
     XMLNAMESPACES = 726,
     XMLPARSE = 727,
     XMLPI = 728,
     XMLROOT = 729,
     XMLSERIALIZE = 730,
     XMLTABLE = 731,
     YEAR_P = 732,
     YES_P = 733,
     ZONE = 734,
     NOT_LA = 735,
     NULLS_LA = 736,
     WITH_LA = 737,
     MODE_TYPE_NAME = 738,
     MODE_PLPGSQL_EXPR = 739,
     MODE_PLPGSQL_ASSIGN1 = 740,
     MODE_PLPGSQL_ASSIGN2 = 741,
     MODE_PLPGSQL_ASSIGN3 = 742,
     UMINUS = 743
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1676 of yacc.c  */
#line 235 "src/backend/parser/gram.y"

	core_YYSTYPE core_yystype;
	/* these fields must match core_YYSTYPE: */
	int			ival;
	char	   *str;
	const char *keyword;

	char		chr;
	bool		boolean;
	JoinType	jtype;
	DropBehavior dbehavior;
	OnCommitAction oncommit;
	List	   *list;
	Node	   *node;
	ObjectType	objtype;
	TypeName   *typnam;
	FunctionParameter *fun_param;
	FunctionParameterMode fun_param_mode;
	ObjectWithArgs *objwithargs;
	DefElem	   *defelt;
	SortBy	   *sortby;
	WindowDef  *windef;
	JoinExpr   *jexpr;
	IndexElem  *ielem;
	StatsElem  *selem;
	Alias	   *alias;
	RangeVar   *range;
	IntoClause *into;
	WithClause *with;
	InferClause	*infer;
	OnConflictClause *onconflict;
	A_Indices  *aind;
	ResTarget  *target;
	struct PrivTarget *privtarget;
	AccessPriv *accesspriv;
	struct ImportQual *importqual;
	InsertStmt *istmt;
	VariableSetStmt *vsetstmt;
	PartitionElem *partelem;
	PartitionSpec *partspec;
	PartitionBoundSpec *partboundspec;
	RoleSpec   *rolespec;
	PublicationObjSpec *publicationobjectspec;
	struct SelectLimit *selectlimit;
	SetQuantifier setquantifier;
	struct GroupClause *groupclause;
	MergeWhenClause *mergewhen;
	struct KeyActions *keyactions;
	struct KeyAction *keyaction;



/* Line 1676 of yacc.c  */
#line 593 "src/backend/parser/gram.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif



#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



