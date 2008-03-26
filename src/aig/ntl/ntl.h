/**CFile****************************************************************

  FileName    [ntl.h]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [Netlist representation.]

  Synopsis    [External declarations.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - June 20, 2005.]

  Revision    [$Id: .h,v 1.00 2005/06/20 00:00:00 alanmi Exp $]

***********************************************************************/
 
#ifndef __NTL_H__
#define __NTL_H__

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////
///                          INCLUDES                                ///
////////////////////////////////////////////////////////////////////////

#include "aig.h"
#include "tim.h"

////////////////////////////////////////////////////////////////////////
///                         PARAMETERS                               ///
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
///                         BASIC TYPES                              ///
////////////////////////////////////////////////////////////////////////

typedef struct Ntl_Man_t_    Ntl_Man_t;
typedef struct Ntl_Mod_t_    Ntl_Mod_t;
typedef struct Ntl_Obj_t_    Ntl_Obj_t;
typedef struct Ntl_Net_t_    Ntl_Net_t;
typedef struct Ntl_Lut_t_    Ntl_Lut_t;

// object types
typedef enum { 
    NTL_OBJ_NONE,                      // 0: non-existent object
    NTL_OBJ_PI,                        // 1: primary input
    NTL_OBJ_PO,                        // 2: primary output
    NTL_OBJ_LATCH,                     // 3: latch node
    NTL_OBJ_NODE,                      // 4: logic node
    NTL_OBJ_BOX,                       // 5: white box or black box
    NTL_OBJ_VOID                       // 6: unused object
} Ntl_Type_t;

struct Ntl_Man_t_
{
    // models of this design
    char *             pName;          // the name of this design
    char *             pSpec;          // the name of input file
    Vec_Ptr_t *        vModels;        // the array of all models used to represent boxes
    // memory managers
    Aig_MmFlex_t *     pMemObjs;       // memory for objects
    Aig_MmFlex_t *     pMemSops;       // memory for SOPs
    // extracted representation
    Vec_Ptr_t *        vCis;           // the primary inputs of the extracted part
    Vec_Ptr_t *        vCos;           // the primary outputs of the extracted part 
    Vec_Ptr_t *        vNodes;         // the nodes of the abstracted part
    Vec_Int_t *        vBox1Cos;       // the first COs of the boxes
    Aig_Man_t *        pAig;           // the extracted AIG
    Tim_Man_t *        pManTime;       // the timing manager
};

struct Ntl_Mod_t_
{
    // model description
    Ntl_Man_t *        pMan;           // the model manager 
    char *             pName;          // the model name
    Vec_Ptr_t *        vObjs;          // the array of all objects
    Vec_Ptr_t *        vPis;           // the array of PI objects
    Vec_Ptr_t *        vPos;           // the array of PO objects
    int                nObjs[NTL_OBJ_VOID]; // counter of objects of each type
    // hashing names into nets
    Ntl_Net_t **       pTable;         // the hash table of names into nets
    int                nTableSize;     // the allocated table size
    int                nEntries;       // the number of entries in the hash table
    // delay information
    Vec_Int_t *        vDelays;
    Vec_Int_t *        vArrivals;
    Vec_Int_t *        vRequireds;
    float *            pDelayTable;   
}; 

struct Ntl_Obj_t_
{
    Ntl_Mod_t *        pModel;         // the model  
    unsigned           Type   :  3;    // object type
    unsigned           Id     : 27;    // object ID
    unsigned           MarkA  :  1;    // temporary mark  
    unsigned           MarkB  :  1;    // temporary mark
    int                nFanins;        // the number of fanins
    int                nFanouts;       // the number of fanouts
    union {                            // functionality
        Ntl_Mod_t *    pImplem;        // model (for boxes)
        char *         pSop;           // SOP (for logic nodes)
        unsigned       LatchId;        // init state + register class (for latches) 
    };
    Ntl_Net_t *        pFanio[0];      // fanins/fanouts
};

struct Ntl_Net_t_
{
    Ntl_Obj_t *        pDriver;        // driver of the net
    Ntl_Net_t *        pNext;          // next net in the hash table
    Aig_Obj_t *        pFunc;          // the AIG representation
    char               nVisits;        // the number of times the net is visited
    char               fMark;          // temporary mark
    char               pName[0];       // the name of this net
};

struct Ntl_Lut_t_
{
    int                Id;             // the ID of the root AIG node
    int                nFanins;        // the number of fanins
    int *              pFanins;        // the array of fanins
    unsigned *         pTruth;         // the truth table
};

////////////////////////////////////////////////////////////////////////
///                      MACRO DEFINITIONS                           ///
////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
///                      INLINED FUNCTIONS                           ///
////////////////////////////////////////////////////////////////////////

static inline int         Ntl_ModelPiNum( Ntl_Mod_t * p )         { return p->nObjs[NTL_OBJ_PI];                } 
static inline int         Ntl_ModelPoNum( Ntl_Mod_t * p )         { return p->nObjs[NTL_OBJ_PO];                } 
static inline int         Ntl_ModelNodeNum( Ntl_Mod_t * p )       { return p->nObjs[NTL_OBJ_NODE];              } 
static inline int         Ntl_ModelLatchNum( Ntl_Mod_t * p )      { return p->nObjs[NTL_OBJ_LATCH];             } 
static inline int         Ntl_ModelBoxNum( Ntl_Mod_t * p )        { return p->nObjs[NTL_OBJ_BOX];               } 
static inline int         Ntl_ModelCiNum( Ntl_Mod_t * p )         { return p->nObjs[NTL_OBJ_PI] + p->nObjs[NTL_OBJ_LATCH]; } 
static inline int         Ntl_ModelCoNum( Ntl_Mod_t * p )         { return p->nObjs[NTL_OBJ_PO] + p->nObjs[NTL_OBJ_LATCH]; } 

static inline Ntl_Obj_t * Ntl_ModelPi( Ntl_Mod_t * p, int i )     { return Vec_PtrEntry(p->vPis, i);            } 
static inline Ntl_Obj_t * Ntl_ModelPo( Ntl_Mod_t * p, int i )     { return Vec_PtrEntry(p->vPos, i);            } 

static inline char *      Ntl_ModelPiName( Ntl_Mod_t * p, int i ) { return Ntl_ModelPi(p, i)->pFanio[0]->pName; } 
static inline char *      Ntl_ModelPoName( Ntl_Mod_t * p, int i ) { return Ntl_ModelPo(p, i)->pFanio[0]->pName; } 

static inline int         Ntl_ModelIsBlackBox( Ntl_Mod_t * p )    { return Ntl_ModelPiNum(p) + Ntl_ModelPoNum(p) == Vec_PtrSize(p->vObjs); } 

static inline int         Ntl_ObjFaninNum( Ntl_Obj_t * p )        { return p->nFanins;                          } 
static inline int         Ntl_ObjFanoutNum( Ntl_Obj_t * p )       { return p->nFanouts;                         } 

static inline int         Ntl_ObjIsPi( Ntl_Obj_t * p )            { return p->Type == NTL_OBJ_PI;               } 
static inline int         Ntl_ObjIsPo( Ntl_Obj_t * p )            { return p->Type == NTL_OBJ_PO;               } 
static inline int         Ntl_ObjIsNode( Ntl_Obj_t * p )          { return p->Type == NTL_OBJ_NODE;             } 
static inline int         Ntl_ObjIsLatch( Ntl_Obj_t * p )         { return p->Type == NTL_OBJ_LATCH;            } 
static inline int         Ntl_ObjIsBox( Ntl_Obj_t * p )           { return p->Type == NTL_OBJ_BOX;              } 

static inline Ntl_Net_t * Ntl_ObjFanin0( Ntl_Obj_t * p )          { return p->pFanio[0];                        } 
static inline Ntl_Net_t * Ntl_ObjFanout0( Ntl_Obj_t * p )         { return p->pFanio[p->nFanins];               } 

static inline Ntl_Net_t * Ntl_ObjFanin( Ntl_Obj_t * p, int i )    { return p->pFanio[i];                        } 
static inline Ntl_Net_t * Ntl_ObjFanout( Ntl_Obj_t * p, int i )   { return p->pFanio[p->nFanins+1];             } 

static inline void        Ntl_ObjSetFanin( Ntl_Obj_t * p, Ntl_Net_t * pNet, int i )    { p->pFanio[i] = pNet;   } 
static inline void        Ntl_ObjSetFanout( Ntl_Obj_t * p, Ntl_Net_t * pNet, int i )   { p->pFanio[p->nFanins+i] = pNet; pNet->pDriver = p; } 

////////////////////////////////////////////////////////////////////////
///                         ITERATORS                                ///
////////////////////////////////////////////////////////////////////////

#define Ntl_ManForEachModel( p, pNtl, i )                                       \
    Vec_PtrForEachEntry( p->vModels, pNtl, i )
#define Ntl_ManForEachCiNet( p, pNtl, i )                                       \
    Vec_PtrForEachEntry( p->vCis, pNtl, i )
#define Ntl_ManForEachCoNet( p, pNtl, i )                                       \
    Vec_PtrForEachEntry( p->vCos, pNtl, i )
#define Ntl_ManForEachNode( p, pObj, i )                                   \
    for ( i = 0; (i < Vec_PtrSize(p->vNodes)) && (((pObj) = Vec_PtrEntry(p->vNodes, i)), 1); i++ ) \
        if ( !Ntl_ObjIsNode(pObj) ) {} else
#define Ntl_ManForEachBox( p, pObj, i )                                    \
    for ( i = 0; (i < Vec_PtrSize(p->vNodes)) && (((pObj) = Vec_PtrEntry(p->vNodes, i)), 1); i++ ) \
        if ( !Ntl_ObjIsBox(pObj) ) {} else

#define Ntl_ModelForEachPi( pNtl, pObj, i )                                     \
    Vec_PtrForEachEntry( pNtl->vPis, pObj, i )
#define Ntl_ModelForEachPo( pNtl, pObj, i )                                     \
    Vec_PtrForEachEntry( pNtl->vPos, pObj, i )
#define Ntl_ModelForEachObj( pNtl, pObj, i )                                    \
    for ( i = 0; (i < Vec_PtrSize(pNtl->vObjs)) && (((pObj) = Vec_PtrEntry(pNtl->vObjs, i)), 1); i++ ) \
        if ( pObj == NULL ) {} else
#define Ntl_ModelForEachLatch( pNtl, pObj, i )                                  \
    for ( i = 0; (i < Vec_PtrSize(pNtl->vObjs)) && (((pObj) = Vec_PtrEntry(pNtl->vObjs, i)), 1); i++ ) \
        if ( !Ntl_ObjIsLatch(pObj) ) {} else
#define Ntl_ModelForEachNode( pNtl, pObj, i )                                   \
    for ( i = 0; (i < Vec_PtrSize(pNtl->vObjs)) && (((pObj) = Vec_PtrEntry(pNtl->vObjs, i)), 1); i++ ) \
        if ( !Ntl_ObjIsNode(pObj) ) {} else
#define Ntl_ModelForEachBox( pNtl, pObj, i )                                    \
    for ( i = 0; (i < Vec_PtrSize(pNtl->vObjs)) && (((pObj) = Vec_PtrEntry(pNtl->vObjs, i)), 1); i++ ) \
        if ( !Ntl_ObjIsBox(pObj) ) {} else
#define Ntl_ModelForEachNet( pNtl, pNet, i )                                    \
    for ( i = 0; i < pNtl->nTableSize; i++ )                                    \
        for ( pNet = pNtl->pTable[i]; pNet; pNet = pNet->pNext ) 

#define Ntl_ObjForEachFanin( pObj, pFanin, i )                                  \
    for ( i = 0; (i < (pObj)->nFanins) && ((pFanin) = (pObj)->pFanio[i]); i++ )
#define Ntl_ObjForEachFanout( pObj, pFanout, i )                                \
    for ( i = 0; (i < (pObj)->nFanouts) && ((pFanout) = (pObj)->pFanio[(pObj)->nFanins+i]); i++ )

////////////////////////////////////////////////////////////////////////
///                    FUNCTION DECLARATIONS                         ///
////////////////////////////////////////////////////////////////////////

/*=== ntlCore.c ==========================================================*/
extern int             Ntl_ManInsertTest( Ntl_Man_t * p, Aig_Man_t * pAig );
extern int             Ntl_ManInsertTestIf( Ntl_Man_t * p, Aig_Man_t * pAig );
/*=== ntlExtract.c ==========================================================*/
extern Aig_Man_t *     Ntl_ManExtract( Ntl_Man_t * p );
extern char *          Ntl_SopFromTruth( Ntl_Man_t * p, unsigned * pTruth, int nVars, Vec_Int_t * vCover );
/*=== ntlInsert.c ==========================================================*/
extern int             Ntl_ManInsert( Ntl_Man_t * p, Vec_Ptr_t * vMapping, Aig_Man_t * pAig );
/*=== ntlCheck.c ==========================================================*/
extern int             Ntl_ManCheck( Ntl_Man_t * pMan );
extern int             Ntl_ModelCheck( Ntl_Mod_t * pModel );
extern void            Ntl_ModelFixNonDrivenNets( Ntl_Mod_t * pModel );
/*=== ntlMan.c ============================================================*/
extern Ntl_Man_t *     Ntl_ManAlloc( char * pFileName );
extern void            Ntl_ManFree( Ntl_Man_t * p );
extern Ntl_Mod_t *     Ntl_ManFindModel( Ntl_Man_t * p, char * pName );
extern void            Ntl_ManPrintStats( Ntl_Man_t * p );
extern Ntl_Mod_t *     Ntl_ModelAlloc( Ntl_Man_t * pMan, char * pName );
extern void            Ntl_ModelFree( Ntl_Mod_t * p );
/*=== ntlMap.c ============================================================*/
extern Vec_Ptr_t *     Ntl_MappingAlloc( int nLuts, int nVars );
extern Vec_Ptr_t *     Ntl_MappingFromAig( Aig_Man_t * p );
extern Vec_Ptr_t *     Ntl_MappingFpga( Aig_Man_t * p );
extern Vec_Ptr_t *     Ntl_MappingIf( Ntl_Man_t * pMan, Aig_Man_t * p );
/*=== ntlObj.c ============================================================*/
extern Ntl_Obj_t *     Ntl_ModelCreatePi( Ntl_Mod_t * pModel );
extern Ntl_Obj_t *     Ntl_ModelCreatePo( Ntl_Mod_t * pModel, Ntl_Net_t * pNet );
extern Ntl_Obj_t *     Ntl_ModelCreateLatch( Ntl_Mod_t * pModel );
extern Ntl_Obj_t *     Ntl_ModelCreateNode( Ntl_Mod_t * pModel, int nFanins );
extern Ntl_Obj_t *     Ntl_ModelCreateBox( Ntl_Mod_t * pModel, int nFanins, int nFanouts );
extern char *          Ntl_ManStoreName( Ntl_Man_t * p, char * pName );
extern char *          Ntl_ManStoreSop( Ntl_Man_t * p, char * pSop );
extern char *          Ntl_ManStoreFileName( Ntl_Man_t * p, char * pFileName );
/*=== ntlTable.c ==========================================================*/
extern Ntl_Net_t *     Ntl_ModelFindNet( Ntl_Mod_t * p, char * pName );
extern Ntl_Net_t *     Ntl_ModelFindOrCreateNet( Ntl_Mod_t * p, char * pName );
extern int             Ntl_ModelSetNetDriver( Ntl_Obj_t * pObj, Ntl_Net_t * pNet );
extern int             Ntl_ModelFindPioNumber( Ntl_Mod_t * p, char * pName, int * pNumber );
/*=== ntlTime.c ==========================================================*/
extern Tim_Man_t *     Ntl_ManCreateTiming( Ntl_Man_t * p );
/*=== ntlReadBlif.c ==========================================================*/
extern Ntl_Man_t *     Ioa_ReadBlif( char * pFileName, int fCheck );
/*=== ntlWriteBlif.c ==========================================================*/
extern void            Ioa_WriteBlif( Ntl_Man_t * p, char * pFileName );

#ifdef __cplusplus
}
#endif

#endif

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////

