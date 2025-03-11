// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
// https://github.com/mlabbe/nativefiledialog

#pragma once

#include <stddef.h>
#include <stdint.h>

typedef char nfdchar_t;

typedef struct {
    nfdchar_t *buf;
    size_t *indices; /* byte offsets into buf */
    size_t count;    /* number of indices into buf */
}nfdpathset_t;

typedef enum {
    NFD_ERROR,       /* programmatic error */
    NFD_OKAY,        /* user pressed okay, or successful return */
    NFD_CANCEL       /* user pressed cancel */
}nfdresult_t;

nfdresult_t NFD_OpenDialog( const nfdchar_t *filterList,
                            const nfdchar_t *defaultPath,
                            nfdchar_t **outPath );

nfdresult_t NFD_OpenDialogMultiple( const nfdchar_t *filterList,
                                    const nfdchar_t *defaultPath,
                                    nfdpathset_t *outPaths );

nfdresult_t NFD_SaveDialog( const nfdchar_t *filterList,
                            const nfdchar_t *defaultPath,
                            nfdchar_t **outPath );

nfdresult_t NFD_PickFolder( const nfdchar_t *defaultPath,
                            nfdchar_t **outPath);

const char *NFD_GetError( void );
size_t      NFD_PathSet_GetCount( const nfdpathset_t *pathSet );
nfdchar_t  *NFD_PathSet_GetPath( const nfdpathset_t *pathSet, size_t index );
void        NFD_PathSet_Free( nfdpathset_t *pathSet );

#define NFD_MAX_STRLEN 256
#define _NFD_UNUSED(x) ((void)x)
#define NFD_UTF8_BOM "\xEF\xBB\xBF"

void  *NFDi_Malloc( size_t bytes );
void   NFDi_Free( void *ptr );
void   NFDi_SetError( const char *msg );
int    NFDi_SafeStrncpy( char *dst, const char *src, size_t maxCopy );
int32_t NFDi_UTF8_Strlen( const nfdchar_t *str );
int    NFDi_IsFilterSegmentChar( char ch );
