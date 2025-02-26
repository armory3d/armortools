/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

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

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_YY_MACHINEINDEPENDENT_GLSLANG_TAB_CPP_H_INCLUDED
# define YY_YY_MACHINEINDEPENDENT_GLSLANG_TAB_CPP_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    CONST = 258,
    BOOL = 259,
    INT = 260,
    UINT = 261,
    FLOAT = 262,
    BVEC2 = 263,
    BVEC3 = 264,
    BVEC4 = 265,
    IVEC2 = 266,
    IVEC3 = 267,
    IVEC4 = 268,
    UVEC2 = 269,
    UVEC3 = 270,
    UVEC4 = 271,
    VEC2 = 272,
    VEC3 = 273,
    VEC4 = 274,
    MAT2 = 275,
    MAT3 = 276,
    MAT4 = 277,
    MAT2X2 = 278,
    MAT2X3 = 279,
    MAT2X4 = 280,
    MAT3X2 = 281,
    MAT3X3 = 282,
    MAT3X4 = 283,
    MAT4X2 = 284,
    MAT4X3 = 285,
    MAT4X4 = 286,
    SAMPLER2D = 287,
    SAMPLER3D = 288,
    SAMPLERCUBE = 289,
    SAMPLER2DSHADOW = 290,
    SAMPLERCUBESHADOW = 291,
    SAMPLER2DARRAY = 292,
    SAMPLER2DARRAYSHADOW = 293,
    ISAMPLER2D = 294,
    ISAMPLER3D = 295,
    ISAMPLERCUBE = 296,
    ISAMPLER2DARRAY = 297,
    USAMPLER2D = 298,
    USAMPLER3D = 299,
    USAMPLERCUBE = 300,
    USAMPLER2DARRAY = 301,
    SAMPLER = 302,
    SAMPLERSHADOW = 303,
    TEXTURE2D = 304,
    TEXTURE3D = 305,
    TEXTURECUBE = 306,
    TEXTURE2DARRAY = 307,
    ITEXTURE2D = 308,
    ITEXTURE3D = 309,
    ITEXTURECUBE = 310,
    ITEXTURE2DARRAY = 311,
    UTEXTURE2D = 312,
    UTEXTURE3D = 313,
    UTEXTURECUBE = 314,
    UTEXTURE2DARRAY = 315,
    ATTRIBUTE = 316,
    VARYING = 317,
    FLOAT16_T = 318,
    FLOAT32_T = 319,
    DOUBLE = 320,
    FLOAT64_T = 321,
    INT64_T = 322,
    UINT64_T = 323,
    INT32_T = 324,
    UINT32_T = 325,
    INT16_T = 326,
    UINT16_T = 327,
    INT8_T = 328,
    UINT8_T = 329,
    I64VEC2 = 330,
    I64VEC3 = 331,
    I64VEC4 = 332,
    U64VEC2 = 333,
    U64VEC3 = 334,
    U64VEC4 = 335,
    I32VEC2 = 336,
    I32VEC3 = 337,
    I32VEC4 = 338,
    U32VEC2 = 339,
    U32VEC3 = 340,
    U32VEC4 = 341,
    I16VEC2 = 342,
    I16VEC3 = 343,
    I16VEC4 = 344,
    U16VEC2 = 345,
    U16VEC3 = 346,
    U16VEC4 = 347,
    I8VEC2 = 348,
    I8VEC3 = 349,
    I8VEC4 = 350,
    U8VEC2 = 351,
    U8VEC3 = 352,
    U8VEC4 = 353,
    DVEC2 = 354,
    DVEC3 = 355,
    DVEC4 = 356,
    DMAT2 = 357,
    DMAT3 = 358,
    DMAT4 = 359,
    F16VEC2 = 360,
    F16VEC3 = 361,
    F16VEC4 = 362,
    F16MAT2 = 363,
    F16MAT3 = 364,
    F16MAT4 = 365,
    F32VEC2 = 366,
    F32VEC3 = 367,
    F32VEC4 = 368,
    F32MAT2 = 369,
    F32MAT3 = 370,
    F32MAT4 = 371,
    F64VEC2 = 372,
    F64VEC3 = 373,
    F64VEC4 = 374,
    F64MAT2 = 375,
    F64MAT3 = 376,
    F64MAT4 = 377,
    DMAT2X2 = 378,
    DMAT2X3 = 379,
    DMAT2X4 = 380,
    DMAT3X2 = 381,
    DMAT3X3 = 382,
    DMAT3X4 = 383,
    DMAT4X2 = 384,
    DMAT4X3 = 385,
    DMAT4X4 = 386,
    F16MAT2X2 = 387,
    F16MAT2X3 = 388,
    F16MAT2X4 = 389,
    F16MAT3X2 = 390,
    F16MAT3X3 = 391,
    F16MAT3X4 = 392,
    F16MAT4X2 = 393,
    F16MAT4X3 = 394,
    F16MAT4X4 = 395,
    F32MAT2X2 = 396,
    F32MAT2X3 = 397,
    F32MAT2X4 = 398,
    F32MAT3X2 = 399,
    F32MAT3X3 = 400,
    F32MAT3X4 = 401,
    F32MAT4X2 = 402,
    F32MAT4X3 = 403,
    F32MAT4X4 = 404,
    F64MAT2X2 = 405,
    F64MAT2X3 = 406,
    F64MAT2X4 = 407,
    F64MAT3X2 = 408,
    F64MAT3X3 = 409,
    F64MAT3X4 = 410,
    F64MAT4X2 = 411,
    F64MAT4X3 = 412,
    F64MAT4X4 = 413,
    ATOMIC_UINT = 414,
    ACCSTRUCTNV = 415,
    ACCSTRUCTEXT = 416,
    RAYQUERYEXT = 417,
    FCOOPMATNV = 418,
    ICOOPMATNV = 419,
    UCOOPMATNV = 420,
    SAMPLERCUBEARRAY = 421,
    SAMPLERCUBEARRAYSHADOW = 422,
    ISAMPLERCUBEARRAY = 423,
    USAMPLERCUBEARRAY = 424,
    SAMPLER1D = 425,
    SAMPLER1DARRAY = 426,
    SAMPLER1DARRAYSHADOW = 427,
    ISAMPLER1D = 428,
    SAMPLER1DSHADOW = 429,
    SAMPLER2DRECT = 430,
    SAMPLER2DRECTSHADOW = 431,
    ISAMPLER2DRECT = 432,
    USAMPLER2DRECT = 433,
    SAMPLERBUFFER = 434,
    ISAMPLERBUFFER = 435,
    USAMPLERBUFFER = 436,
    SAMPLER2DMS = 437,
    ISAMPLER2DMS = 438,
    USAMPLER2DMS = 439,
    SAMPLER2DMSARRAY = 440,
    ISAMPLER2DMSARRAY = 441,
    USAMPLER2DMSARRAY = 442,
    SAMPLEREXTERNALOES = 443,
    SAMPLEREXTERNAL2DY2YEXT = 444,
    SAMPLERVIDEO = 445,
    ISAMPLER1DARRAY = 446,
    USAMPLER1D = 447,
    USAMPLER1DARRAY = 448,
    F16SAMPLER1D = 449,
    F16SAMPLER2D = 450,
    F16SAMPLER3D = 451,
    F16SAMPLER2DRECT = 452,
    F16SAMPLERCUBE = 453,
    F16SAMPLER1DARRAY = 454,
    F16SAMPLER2DARRAY = 455,
    F16SAMPLERCUBEARRAY = 456,
    F16SAMPLERBUFFER = 457,
    F16SAMPLER2DMS = 458,
    F16SAMPLER2DMSARRAY = 459,
    F16SAMPLER1DSHADOW = 460,
    F16SAMPLER2DSHADOW = 461,
    F16SAMPLER1DARRAYSHADOW = 462,
    F16SAMPLER2DARRAYSHADOW = 463,
    F16SAMPLER2DRECTSHADOW = 464,
    F16SAMPLERCUBESHADOW = 465,
    F16SAMPLERCUBEARRAYSHADOW = 466,
    IMAGE1D = 467,
    IIMAGE1D = 468,
    UIMAGE1D = 469,
    IMAGE2D = 470,
    IIMAGE2D = 471,
    UIMAGE2D = 472,
    IMAGE3D = 473,
    IIMAGE3D = 474,
    UIMAGE3D = 475,
    IMAGE2DRECT = 476,
    IIMAGE2DRECT = 477,
    UIMAGE2DRECT = 478,
    IMAGECUBE = 479,
    IIMAGECUBE = 480,
    UIMAGECUBE = 481,
    IMAGEBUFFER = 482,
    IIMAGEBUFFER = 483,
    UIMAGEBUFFER = 484,
    IMAGE1DARRAY = 485,
    IIMAGE1DARRAY = 486,
    UIMAGE1DARRAY = 487,
    IMAGE2DARRAY = 488,
    IIMAGE2DARRAY = 489,
    UIMAGE2DARRAY = 490,
    IMAGECUBEARRAY = 491,
    IIMAGECUBEARRAY = 492,
    UIMAGECUBEARRAY = 493,
    IMAGE2DMS = 494,
    IIMAGE2DMS = 495,
    UIMAGE2DMS = 496,
    IMAGE2DMSARRAY = 497,
    IIMAGE2DMSARRAY = 498,
    UIMAGE2DMSARRAY = 499,
    F16IMAGE1D = 500,
    F16IMAGE2D = 501,
    F16IMAGE3D = 502,
    F16IMAGE2DRECT = 503,
    F16IMAGECUBE = 504,
    F16IMAGE1DARRAY = 505,
    F16IMAGE2DARRAY = 506,
    F16IMAGECUBEARRAY = 507,
    F16IMAGEBUFFER = 508,
    F16IMAGE2DMS = 509,
    F16IMAGE2DMSARRAY = 510,
    TEXTURECUBEARRAY = 511,
    ITEXTURECUBEARRAY = 512,
    UTEXTURECUBEARRAY = 513,
    TEXTURE1D = 514,
    ITEXTURE1D = 515,
    UTEXTURE1D = 516,
    TEXTURE1DARRAY = 517,
    ITEXTURE1DARRAY = 518,
    UTEXTURE1DARRAY = 519,
    TEXTURE2DRECT = 520,
    ITEXTURE2DRECT = 521,
    UTEXTURE2DRECT = 522,
    TEXTUREBUFFER = 523,
    ITEXTUREBUFFER = 524,
    UTEXTUREBUFFER = 525,
    TEXTURE2DMS = 526,
    ITEXTURE2DMS = 527,
    UTEXTURE2DMS = 528,
    TEXTURE2DMSARRAY = 529,
    ITEXTURE2DMSARRAY = 530,
    UTEXTURE2DMSARRAY = 531,
    F16TEXTURE1D = 532,
    F16TEXTURE2D = 533,
    F16TEXTURE3D = 534,
    F16TEXTURE2DRECT = 535,
    F16TEXTURECUBE = 536,
    F16TEXTURE1DARRAY = 537,
    F16TEXTURE2DARRAY = 538,
    F16TEXTURECUBEARRAY = 539,
    F16TEXTUREBUFFER = 540,
    F16TEXTURE2DMS = 541,
    F16TEXTURE2DMSARRAY = 542,
    SUBPASSINPUT = 543,
    SUBPASSINPUTMS = 544,
    ISUBPASSINPUT = 545,
    ISUBPASSINPUTMS = 546,
    USUBPASSINPUT = 547,
    USUBPASSINPUTMS = 548,
    F16SUBPASSINPUT = 549,
    F16SUBPASSINPUTMS = 550,
    LEFT_OP = 551,
    RIGHT_OP = 552,
    INC_OP = 553,
    DEC_OP = 554,
    LE_OP = 555,
    GE_OP = 556,
    EQ_OP = 557,
    NE_OP = 558,
    AND_OP = 559,
    OR_OP = 560,
    XOR_OP = 561,
    MUL_ASSIGN = 562,
    DIV_ASSIGN = 563,
    ADD_ASSIGN = 564,
    MOD_ASSIGN = 565,
    LEFT_ASSIGN = 566,
    RIGHT_ASSIGN = 567,
    AND_ASSIGN = 568,
    XOR_ASSIGN = 569,
    OR_ASSIGN = 570,
    SUB_ASSIGN = 571,
    STRING_LITERAL = 572,
    LEFT_PAREN = 573,
    RIGHT_PAREN = 574,
    LEFT_BRACKET = 575,
    RIGHT_BRACKET = 576,
    LEFT_BRACE = 577,
    RIGHT_BRACE = 578,
    DOT = 579,
    COMMA = 580,
    COLON = 581,
    EQUAL = 582,
    SEMICOLON = 583,
    BANG = 584,
    DASH = 585,
    TILDE = 586,
    PLUS = 587,
    STAR = 588,
    SLASH = 589,
    PERCENT = 590,
    LEFT_ANGLE = 591,
    RIGHT_ANGLE = 592,
    VERTICAL_BAR = 593,
    CARET = 594,
    AMPERSAND = 595,
    QUESTION = 596,
    INVARIANT = 597,
    HIGH_PRECISION = 598,
    MEDIUM_PRECISION = 599,
    LOW_PRECISION = 600,
    PRECISION = 601,
    PACKED = 602,
    RESOURCE = 603,
    SUPERP = 604,
    FLOATCONSTANT = 605,
    INTCONSTANT = 606,
    UINTCONSTANT = 607,
    BOOLCONSTANT = 608,
    IDENTIFIER = 609,
    TYPE_NAME = 610,
    CENTROID = 611,
    IN = 612,
    OUT = 613,
    INOUT = 614,
    STRUCT = 615,
    VOID = 616,
    WHILE = 617,
    BREAK = 618,
    CONTINUE = 619,
    DO = 620,
    ELSE = 621,
    FOR = 622,
    IF = 623,
    DISCARD = 624,
    RETURN = 625,
    SWITCH = 626,
    CASE = 627,
    DEFAULT = 628,
    UNIFORM = 629,
    SHARED = 630,
    BUFFER = 631,
    FLAT = 632,
    SMOOTH = 633,
    LAYOUT = 634,
    DOUBLECONSTANT = 635,
    INT16CONSTANT = 636,
    UINT16CONSTANT = 637,
    FLOAT16CONSTANT = 638,
    INT32CONSTANT = 639,
    UINT32CONSTANT = 640,
    INT64CONSTANT = 641,
    UINT64CONSTANT = 642,
    SUBROUTINE = 643,
    DEMOTE = 644,
    PAYLOADNV = 645,
    PAYLOADINNV = 646,
    HITATTRNV = 647,
    CALLDATANV = 648,
    CALLDATAINNV = 649,
    PAYLOADEXT = 650,
    PAYLOADINEXT = 651,
    HITATTREXT = 652,
    CALLDATAEXT = 653,
    CALLDATAINEXT = 654,
    PATCH = 655,
    SAMPLE = 656,
    NONUNIFORM = 657,
    COHERENT = 658,
    VOLATILE = 659,
    RESTRICT = 660,
    READONLY = 661,
    WRITEONLY = 662,
    DEVICECOHERENT = 663,
    QUEUEFAMILYCOHERENT = 664,
    WORKGROUPCOHERENT = 665,
    SUBGROUPCOHERENT = 666,
    NONPRIVATE = 667,
    SHADERCALLCOHERENT = 668,
    NOPERSPECTIVE = 669,
    EXPLICITINTERPAMD = 670,
    PERVERTEXNV = 671,
    PERPRIMITIVENV = 672,
    PERVIEWNV = 673,
    PERTASKNV = 674,
    PRECISE = 675
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 97 "MachineIndependent/glslang.y"

    struct {
        glslang::TSourceLoc loc;
        union {
            glslang::TString *string;
            int i;
            unsigned int u;
            long long i64;
            unsigned long long u64;
            bool b;
            double d;
        };
        glslang::TSymbol* symbol;
    } lex;
    struct {
        glslang::TSourceLoc loc;
        glslang::TOperator op;
        union {
            TIntermNode* intermNode;
            glslang::TIntermNodePair nodePair;
            glslang::TIntermTyped* intermTypedNode;
            glslang::TAttributes* attributes;
        };
        union {
            glslang::TPublicType type;
            glslang::TFunction* function;
            glslang::TParameter param;
            glslang::TTypeLoc typeLine;
            glslang::TTypeList* typeList;
            glslang::TArraySizes* arraySizes;
            glslang::TIdentifierList* identifierList;
        };
        glslang::TArraySizes* typeParameters;
    } interm;

#line 514 "MachineIndependent/glslang_tab.cpp.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int yyparse (glslang::TParseContext* pParseContext);

#endif /* !YY_YY_MACHINEINDEPENDENT_GLSLANG_TAB_CPP_H_INCLUDED  */
