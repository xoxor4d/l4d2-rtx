#pragma once
#include "utils/vector.hpp"

namespace components
{
	enum LightType_t : int
	{
		MATERIAL_LIGHT_DISABLE = 0x0,
		MATERIAL_LIGHT_POINT = 0x1,
		MATERIAL_LIGHT_DIRECTIONAL = 0x2,
		MATERIAL_LIGHT_SPOT = 0x3,
	};

	enum VertexCompressionType_t : int
	{
		VERTEX_COMPRESSION_INVALID = 0xFFFFFFFF,
		VERTEX_COMPRESSION_NONE = 0x0,
		VERTEX_COMPRESSION_ON = 0x1,
	};

	enum ShadowType_t : int
	{
		SHADOWS_NONE = 0x0,
		SHADOWS_SIMPLE = 0x1,
		SHADOWS_RENDER_TO_TEXTURE = 0x2,
		SHADOWS_RENDER_TO_TEXTURE_DYNAMIC = 0x3,
		SHADOWS_RENDER_TO_DEPTH_TEXTURE = 0x4,
		SHADOWS_RENDER_TO_TEXTURE_DYNAMIC_CUSTOM = 0x5,
	};

	enum MaterialVarFlags_t : int
	{
		MATERIAL_VAR_DEBUG = 0x1,
		MATERIAL_VAR_NO_DEBUG_OVERRIDE = 0x2,
		MATERIAL_VAR_NO_DRAW = 0x4,
		MATERIAL_VAR_USE_IN_FILLRATE_MODE = 0x8,
		MATERIAL_VAR_VERTEXCOLOR = 0x10,
		MATERIAL_VAR_VERTEXALPHA = 0x20,
		MATERIAL_VAR_SELFILLUM = 0x40,
		MATERIAL_VAR_ADDITIVE = 0x80,
		MATERIAL_VAR_ALPHATEST = 0x100,
		MATERIAL_VAR_PSEUDO_TRANSLUCENT = 0x200,
		MATERIAL_VAR_ZNEARER = 0x400,
		MATERIAL_VAR_MODEL = 0x800,
		MATERIAL_VAR_FLAT = 0x1000,
		MATERIAL_VAR_NOCULL = 0x2000,
		MATERIAL_VAR_NOFOG = 0x4000,
		MATERIAL_VAR_IGNOREZ = 0x8000,
		MATERIAL_VAR_DECAL = 0x10000,
		MATERIAL_VAR_ENVMAPSPHERE = 0x20000,
		MATERIAL_VAR_ENVMAPCAMERASPACE = 0x80000,
		MATERIAL_VAR_BASEALPHAENVMAPMASK = 0x100000,
		MATERIAL_VAR_TRANSLUCENT = 0x200000,
		MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK = 0x400000,
		MATERIAL_VAR_NEEDS_SOFTWARE_SKINNING = 0x800000,
		MATERIAL_VAR_OPAQUETEXTURE = 0x1000000,
		MATERIAL_VAR_ENVMAPMODE = 0x2000000,
		MATERIAL_VAR_SUPPRESS_DECALS = 0x4000000,
		MATERIAL_VAR_HALFLAMBERT = 0x8000000,
		MATERIAL_VAR_WIREFRAME = 0x10000000,
		MATERIAL_VAR_ALLOWALPHATOCOVERAGE = 0x20000000,
		MATERIAL_VAR_ALPHA_MODIFIED_BY_PROXY = 0x40000000,
		MATERIAL_VAR_VERTEXFOG = 0x80000000,
	};

	enum DrawBrushModelMode_t : __int32
	{
		DBM_DRAW_ALL = 0x0,
		DBM_DRAW_OPAQUE_ONLY = 0x1,
		DBM_DRAW_TRANSLUCENT_ONLY = 0x2,
	};

	enum MaterialPropertyTypes_t : int
	{
		MATERIAL_PROPERTY_NEEDS_LIGHTMAP = 0x0,
		MATERIAL_PROPERTY_OPACITY = 0x1,
		MATERIAL_PROPERTY_REFLECTIVITY = 0x2,
		MATERIAL_PROPERTY_NEEDS_BUMPED_LIGHTMAPS = 0x3,
	};

	enum PreviewImageRetVal_t : int
	{
		MATERIAL_PREVIEW_IMAGE_BAD = 0x0,
		MATERIAL_PREVIEW_IMAGE_OK = 0x1,
		MATERIAL_NO_PREVIEW_IMAGE = 0x2,
	};

	enum MaterialPrimitiveType_t : __int32
	{
		MATERIAL_POINTS = 0x0,
		MATERIAL_LINES = 0x1,
		MATERIAL_TRIANGLES = 0x2,
		MATERIAL_TRIANGLE_STRIP = 0x3,
		MATERIAL_LINE_STRIP = 0x4,
		MATERIAL_LINE_LOOP = 0x5,
		MATERIAL_POLYGON = 0x6,
		MATERIAL_QUADS = 0x7,
		MATERIAL_SUBD_QUADS_EXTRA = 0x8,
		MATERIAL_SUBD_QUADS_REG = 0x9,
		MATERIAL_INSTANCED_QUADS = 0xA,
		MATERIAL_HETEROGENOUS = 0xB,
	};

	enum ImageFormat : int
	{
		IMAGE_FORMAT_DEFAULT = 0xFFFFFFFE,
		IMAGE_FORMAT_UNKNOWN = 0xFFFFFFFF,
		IMAGE_FORMAT_RGBA8888 = 0x0,
		IMAGE_FORMAT_ABGR8888 = 0x1,
		IMAGE_FORMAT_RGB888 = 0x2,
		IMAGE_FORMAT_BGR888 = 0x3,
		IMAGE_FORMAT_RGB565 = 0x4,
		IMAGE_FORMAT_I8 = 0x5,
		IMAGE_FORMAT_IA88 = 0x6,
		IMAGE_FORMAT_P8 = 0x7,
		IMAGE_FORMAT_A8 = 0x8,
		IMAGE_FORMAT_RGB888_BLUESCREEN = 0x9,
		IMAGE_FORMAT_BGR888_BLUESCREEN = 0xA,
		IMAGE_FORMAT_ARGB8888 = 0xB,
		IMAGE_FORMAT_BGRA8888 = 0xC,
		IMAGE_FORMAT_DXT1 = 0xD,
		IMAGE_FORMAT_DXT3 = 0xE,
		IMAGE_FORMAT_DXT5 = 0xF,
		IMAGE_FORMAT_BGRX8888 = 0x10,
		IMAGE_FORMAT_BGR565 = 0x11,
		IMAGE_FORMAT_BGRX5551 = 0x12,
		IMAGE_FORMAT_BGRA4444 = 0x13,
		IMAGE_FORMAT_DXT1_ONEBITALPHA = 0x14,
		IMAGE_FORMAT_BGRA5551 = 0x15,
		IMAGE_FORMAT_UV88 = 0x16,
		IMAGE_FORMAT_UVWQ8888 = 0x17,
		IMAGE_FORMAT_RGBA16161616F = 0x18,
		IMAGE_FORMAT_RGBA16161616 = 0x19,
		IMAGE_FORMAT_UVLX8888 = 0x1A,
		IMAGE_FORMAT_R32F = 0x1B,
		IMAGE_FORMAT_RGB323232F = 0x1C,
		IMAGE_FORMAT_RGBA32323232F = 0x1D,
		IMAGE_FORMAT_RG1616F = 0x1E,
		IMAGE_FORMAT_RG3232F = 0x1F,
		IMAGE_FORMAT_RGBX8888 = 0x20,
		IMAGE_FORMAT_NULL = 0x21,
		IMAGE_FORMAT_ATI2N = 0x22,
		IMAGE_FORMAT_ATI1N = 0x23,
		IMAGE_FORMAT_RGBA1010102 = 0x24,
		IMAGE_FORMAT_BGRA1010102 = 0x25,
		IMAGE_FORMAT_R16F = 0x26,
		IMAGE_FORMAT_D16 = 0x27,
		IMAGE_FORMAT_D15S1 = 0x28,
		IMAGE_FORMAT_D32 = 0x29,
		IMAGE_FORMAT_D24S8 = 0x2A,
		IMAGE_FORMAT_LINEAR_D24S8 = 0x2B,
		IMAGE_FORMAT_D24X8 = 0x2C,
		IMAGE_FORMAT_D24X4S4 = 0x2D,
		IMAGE_FORMAT_D24FS8 = 0x2E,
		IMAGE_FORMAT_D16_SHADOW = 0x2F,
		IMAGE_FORMAT_D24X8_SHADOW = 0x30,
		IMAGE_FORMAT_LINEAR_BGRX8888 = 0x31,
		IMAGE_FORMAT_LINEAR_RGBA8888 = 0x32,
		IMAGE_FORMAT_LINEAR_ABGR8888 = 0x33,
		IMAGE_FORMAT_LINEAR_ARGB8888 = 0x34,
		IMAGE_FORMAT_LINEAR_BGRA8888 = 0x35,
		IMAGE_FORMAT_LINEAR_RGB888 = 0x36,
		IMAGE_FORMAT_LINEAR_BGR888 = 0x37,
		IMAGE_FORMAT_LINEAR_BGRX5551 = 0x38,
		IMAGE_FORMAT_LINEAR_I8 = 0x39,
		IMAGE_FORMAT_LINEAR_RGBA16161616 = 0x3A,
		IMAGE_FORMAT_LE_BGRX8888 = 0x3B,
		IMAGE_FORMAT_LE_BGRA8888 = 0x3C,
		NUM_IMAGE_FORMATS = 0x3D,
	};

	enum MaterialIndexFormat_t : int
	{
		MATERIAL_INDEX_FORMAT_UNKNOWN = 0xFFFFFFFF,
		MATERIAL_INDEX_FORMAT_16BIT = 0x0,
		MATERIAL_INDEX_FORMAT_32BIT = 0x1,
	};

	enum modtype_t : std::int32_t
	{
		mod_bad = 0x0,
		mod_brush = 0x1,
		mod_sprite = 0x2,
		mod_studio = 0x3,
	};

	enum OverrideType_t : int
	{
		OVERRIDE_NORMAL = 0x0,
		OVERRIDE_BUILD_SHADOWS = 0x1,
		OVERRIDE_DEPTH_WRITE = 0x2,
	};

	enum $D6791CFE666A571E1681338A952F9E69 : int
	{
		ADDDECAL_TO_ALL_LODS = 0xFFFFFFFF,
	};

	enum
	{
		FRUSTUM_RIGHT = 0,
		FRUSTUM_LEFT = 1,
		FRUSTUM_TOP = 2,
		FRUSTUM_BOTTOM = 3,
		FRUSTUM_NEARZ = 4,
		FRUSTUM_FARZ = 5,
		FRUSTUM_NUMPLANES = 6
	};

	enum view_id : __int32
	{
		VIEW_ILLEGAL = 0xFFFFFFFE,
		VIEW_NONE = 0xFFFFFFFF,
		VIEW_MAIN = 0x0,
		VIEW_3DSKY = 0x1,
		VIEW_MONITOR = 0x2,
		VIEW_REFLECTION = 0x3,
		VIEW_REFRACTION = 0x4,
		VIEW_INTRO_PLAYER = 0x5,
		VIEW_INTRO_CAMERA = 0x6,
		VIEW_SHADOW_DEPTH_TEXTURE = 0x7,
		VIEW_ID_COUNT = 0x8,
	};

	struct matrix3x4_t
	{
		float m_flMatVal[3][4];
	};

	struct VMatrix
	{
		float m[4][4];
	};

	struct IntVector4D
	{
		int x;
		int y;
		int z;
		int w;
	};

	struct colorVec
	{
		unsigned int r;
		unsigned int g;
		unsigned int b;
		unsigned int a;
	};

	/*struct VPlane
	{
		Vector m_Normal;
		float m_Dist;
	};*/

	class VPlane
	{
	public:
		VPlane();
		VPlane(const Vector& vNormal, vec_t dist);

		void		Init(const Vector& vNormal, vec_t dist);
		vec_t		DistTo(const Vector& vVec) const;
		VPlane& operator=(const VPlane& thePlane);
		
	public:
		Vector		m_Normal;
		vec_t		m_Dist;
	};

	inline VPlane::VPlane()
	{
	}

	inline VPlane::VPlane(const Vector& vNormal, vec_t dist)
	{
		m_Normal = vNormal;
		m_Dist = dist;
	}

	inline void	VPlane::Init(const Vector& vNormal, vec_t dist)
	{
		m_Normal = vNormal;
		m_Dist = dist;
	}

	inline vec_t VPlane::DistTo(const Vector& vVec) const
	{
		return vVec.Dot(m_Normal) - m_Dist;
	}

	inline VPlane& VPlane::operator=(const VPlane& thePlane)
	{
		m_Normal = thePlane.m_Normal;
		m_Dist = thePlane.m_Dist;
		return *this;
	}

	// ------------------------------
	// ------------------------------

	struct QAngle
	{
		float x;
		float y;
		float z;
	};

	struct Color
	{
		unsigned __int8 _color[4];
	};


	struct CUtlSymbol
	{
		unsigned __int16 m_Id;
	};

	struct memhandle_t__
	{
		int unused;
	};

	/*struct __declspec(align(2)) LightingQuery_t
	{
		Vector m_LightingOrigin;
		unsigned __int16 m_InstanceHandle;
		bool m_bAmbientBoost;
	};*/

	struct __declspec(align(16)) Ray_t
	{
		VectorAligned m_Start;
		VectorAligned m_Delta;
		VectorAligned m_StartOffset;
		VectorAligned m_Extents;
		const matrix3x4_t* m_pWorldAxisTransform;
		bool m_IsRay;
		bool m_IsSwept;
	};

	union $123B8C34E442A509E2700B23F73AA3E7
	{
		int m_iValue;
		float m_flValue;
		void* m_pValue;
		unsigned __int8 m_Color[4];
	};

	struct KeyValues
	{
		unsigned __int32 m_iKeyName : 24;
		unsigned __int32 m_iKeyNameCaseSensitive1 : 8;
		char* m_sValue;
		wchar_t* m_wsValue;
		$123B8C34E442A509E2700B23F73AA3E7 u4;
		char m_iDataType;
		char m_bHasEscapeSequences;
		unsigned __int16 m_iKeyNameCaseSensitive2;
		KeyValues* m_pPeer;
		KeyValues* m_pSub;
		KeyValues* m_pChain;
		bool(__cdecl* m_pExpressionGetSymbolProc)(const char*);
	};

	struct cplane_t
	{
		Vector normal;
		float dist;
		unsigned __int8 type;
		unsigned __int8 signbits;
		unsigned __int8 pad[2];
	};

	struct mnode_t
	{
		int contents;
		int visframe;
		mnode_t* parent;
		__int16 area;
		__int16 flags;
		VectorAligned m_vecCenter;
		VectorAligned m_vecHalfDiagonal;
		cplane_t* plane;
		mnode_t* children[2];
		unsigned __int16 firstsurface;
		unsigned __int16 numsurfaces;
	};

	struct mleaf_t
	{
		int contents;
		int visframe;
		mnode_t* parent; // mnode_t
		__int16 area;
		__int16 flags;
		VectorAligned m_vecCenter;
		VectorAligned m_vecHalfDiagonal;
		__int16 cluster;
		__int16 leafWaterDataID;
		unsigned __int16 firstmarksurface;
		unsigned __int16 nummarksurfaces;
		__int16 nummarknodesurfaces;
		__int16 unused;
		unsigned __int16 dispListStart;
		unsigned __int16 dispCount;
	};

	struct mleafwaterdata_t
	{
		float surfaceZ;
		float minZ;
		__int16 surfaceTexInfoID;
		__int16 firstLeafIndex;
	};

	struct mvertex_t
	{
		Vector position;
	};

	struct doccluderdata_t
	{
		int flags;
		int firstpoly;
		int polycount;
		Vector mins;
		Vector maxs;
		int area;
	};

	struct doccluderpolydata_t
	{
		int firstvertexindex;
		int vertexcount;
		int planenum;
	};

	struct mtexinfo_t
	{
		Vector4D textureVecsTexelsPerWorldUnits[2];
		Vector4D lightmapVecsLuxelsPerWorldUnits[2];
		float luxelsPerWorldUnit;
		float worldUnitsPerLuxel;
		unsigned __int16 flags;
		unsigned __int16 texinfoFlags;
		void* material; // IMaterial
	};

	struct csurface_t
	{
		const char* name;
		__int16 surfaceProps;
		unsigned __int16 flags;
	};

	struct msurfacenormal_t
	{
		unsigned int firstvertnormal;
	};

	struct dfacebrushlist_t
	{
		unsigned __int16 m_nFaceBrushCount;
		unsigned __int16 m_nFaceBrushStart;
	};

	struct mprimitive_t
	{
		int type;
		unsigned __int16 firstIndex;
		unsigned __int16 indexCount;
		unsigned __int16 firstVert;
		unsigned __int16 vertCount;
	};

	struct mprimvert_t
	{
		Vector pos;
		float texCoord[2];
		float lightCoord[2];
	};

	struct spritedata_t
	{
		int numframes;
		int width;
		int height;
		void* sprite; // CEngineSprite
	};

	struct worldbrushdata_t
	{
		int numsubmodels;
		//int nWorldFaceCount;
		int numplanes;
		cplane_t* planes;
		int numleafs;
		mleaf_t* leafs;
		int numleafwaterdata;
		mleafwaterdata_t* leafwaterdata;
		int numvertexes;
		mvertex_t* vertexes;
		int numoccluders;
		doccluderdata_t* occluders;
		int numoccluderpolys;
		doccluderpolydata_t* occluderpolys;
		int numoccludervertindices;
		int* occludervertindices;
		int numvertnormalindices;
		unsigned __int16* vertnormalindices;
		int numvertnormals;
		Vector* vertnormals;
		int numnodes;
		mnode_t* nodes;
		//unsigned __int16* m_LeafMinDistToWater;
		//int numtexinfo;
		//mtexinfo_t* texinfo;
		//int numtexdata;
		//csurface_t* texdata;
		//int numDispInfos;
		//void* hDispInfos;
		//int numsurfaces;
		//void* surfaces1; // msurface1_t
		//void* surfaces2; // msurface2_t
		//void* surfacelighting; // msurfacelighting_t
		//msurfacenormal_t* surfacenormals;
		//unsigned __int16* m_pSurfaceBrushes;
		//dfacebrushlist_t* m_pSurfaceBrushList;
		//int numvertindices;
		//unsigned __int16* vertindices;
		//int nummarksurfaces;
		//void** marksurfaces; // msurface2_t
		//void* lightdata; // ColorRGBExp32
		//int m_nLightingDataSize;
		//int numworldlights;
		//void* worldlights; // dworldlight_t
		//void* shadowzbuffers;
		//int numprimitives;
		//mprimitive_t* primitives;
		//int numprimverts;
		//mprimvert_t* primverts;
		//int numprimindices;
		//unsigned __int16* primindices;
		//int m_nAreas;
		//void* m_pAreas; // darea_t
		//int m_nAreaPortals;
		//void* m_pAreaPortals; // dareaportal_t
		//int m_nClipPortalVerts;
		//Vector* m_pClipPortalVerts;
		//void* m_pCubemapSamples; // mcubemapsample_t
		//int m_nCubemapSamples;
		//int m_nDispInfoReferences;
		//unsigned __int16* m_pDispInfoReferences;
		//void* m_pLeafAmbient; // dleafambientindex_t
		//void* m_pAmbientSamples; // dleafambientlighting_t
		//bool m_bUnloadedAllLightmaps;
		//void* m_pLightingDataStack; // CMemoryStack
		//int m_nBSPFileSize;
	};
	STATIC_ASSERT_OFFSET(worldbrushdata_t, planes, 0x8);
	STATIC_ASSERT_OFFSET(worldbrushdata_t, numleafs, 0xC);
	STATIC_ASSERT_OFFSET(worldbrushdata_t, nodes, 0x50);

	struct brushdata_t
	{
		worldbrushdata_t* pShared;
		int firstmodelsurface;
		int nummodelsurfaces;
		int nLightstyleLastComputedFrame;
		unsigned __int16 nLightstyleIndex;
		unsigned __int16 nLightstyleCount;
		unsigned __int16 renderHandle;
		unsigned __int16 firstnode;
	};

	union $827FC955A655E715E2ACE31D316F483B
	{
		brushdata_t brush;
		unsigned __int16 studio;
		spritedata_t sprite;
	};

	struct model_t
	{
		void* fnHandle;
		char szPathName[260];
		int nLoadFlags;
		int nServerCount;
		modtype_t type;
		int flags;
		Vector mins;
		Vector maxs;
		float radius;
		KeyValues* m_pKeyValues;
		$827FC955A655E715E2ACE31D316F483B ___u10;
	};

	struct CCommonHostState
	{
		model_t* worldmodel;
		worldbrushdata_t* worldbrush;
		float interval_per_tick;
		int max_splitscreen_players;
		int max_splitscreen_players_clientdll;
	};


	enum ShaderStencilOp_t : __int32
	{
		SHADER_STENCILOP_KEEP = 0x1,
		SHADER_STENCILOP_ZERO = 0x2,
		SHADER_STENCILOP_SET_TO_REFERENCE = 0x3,
		SHADER_STENCILOP_INCREMENT_CLAMP = 0x4,
		SHADER_STENCILOP_DECREMENT_CLAMP = 0x5,
		SHADER_STENCILOP_INVERT = 0x6,
		SHADER_STENCILOP_INCREMENT_WRAP = 0x7,
		SHADER_STENCILOP_DECREMENT_WRAP = 0x8,
		SHADER_STENCILOP_FORCE_DWORD = 0x7FFFFFFF,
	};

	enum ShaderStencilFunc_t : __int32
	{
		SHADER_STENCILFUNC_NEVER = 0x1,
		SHADER_STENCILFUNC_LESS = 0x2,
		SHADER_STENCILFUNC_EQUAL = 0x3,
		SHADER_STENCILFUNC_LEQUAL = 0x4,
		SHADER_STENCILFUNC_GREATER = 0x5,
		SHADER_STENCILFUNC_NOTEQUAL = 0x6,
		SHADER_STENCILFUNC_GEQUAL = 0x7,
		SHADER_STENCILFUNC_ALWAYS = 0x8,
		SHADER_STENCILFUNC_FORCE_DWORD = 0x7FFFFFFF,
	};

	struct ShaderStencilState_t
	{
		bool m_bEnable;
		ShaderStencilOp_t m_FailOp;
		ShaderStencilOp_t m_ZFailOp;
		ShaderStencilOp_t m_PassOp;
		ShaderStencilFunc_t m_CompareFunc;
		int m_nReferenceValue;
		unsigned int m_nTestMask;
		unsigned int m_nWriteMask;
	};

	struct  BrushArrayInstanceData_t
	{
		matrix3x4_t* m_pBrushToWorld;
		const model_t* m_pBrushModel;
		Vector4D m_DiffuseModulation;
		ShaderStencilState_t* m_pStencilState;
	};

	struct studiohdr_t
	{
		int id;
		int version;
		int checksum;
		char name[64];
		int length;
		Vector eyeposition;
		Vector illumposition;
		Vector hull_min;
		Vector hull_max;
		Vector view_bbmin;
		Vector view_bbmax;
		int flags;
		int numbones;
		int boneindex;
		int numbonecontrollers;
		int bonecontrollerindex;
		int numhitboxsets;
		int hitboxsetindex;
		int numlocalanim;
		int localanimindex;
		int numlocalseq;
		int localseqindex;
		int activitylistversion;
		int eventsindexed;
		int numtextures;
		int textureindex;
		int numcdtextures;
		int cdtextureindex;
		int numskinref;
		int numskinfamilies;
		int skinindex;
		int numbodyparts;
		int bodypartindex;
		int numlocalattachments;
		int localattachmentindex;
		int numlocalnodes;
		int localnodeindex;
		int localnodenameindex;
		int numflexdesc;
		int flexdescindex;
		int numflexcontrollers;
		int flexcontrollerindex;
		int numflexrules;
		int flexruleindex;
		int numikchains;
		int ikchainindex;
		int nummouths;
		int mouthindex;
		int numlocalposeparameters;
		int localposeparamindex;
		int surfacepropindex;
		int keyvalueindex;
		int keyvaluesize;
		int numlocalikautoplaylocks;
		int localikautoplaylockindex;
		float mass;
		int contents;
		int numincludemodels;
		int includemodelindex;
		void* virtualModel;
		int szanimblocknameindex;
		int numanimblocks;
		int animblockindex;
		void* animblockModel;
		int bonetablebynameindex;
		void* pVertexBase;
		void* pIndexBase;
		unsigned __int8 constdirectionallightdot;
		unsigned __int8 rootLOD;
		unsigned __int8 numAllowedRootLODs;
		unsigned __int8 unused[1];
		int unused4;
		int numflexcontrollerui;
		int flexcontrolleruiindex;
		float flVertAnimFixedPointScale;
		int surfacepropLookup;
		int studiohdr2index;
		int unused2[1];
	};

	struct IndexDesc_t
	{
		unsigned __int16* m_pIndices;
		unsigned int m_nOffset;
		unsigned int m_nFirstIndex;
		unsigned int m_nIndexSize;
	};

	struct VertexDesc_t
	{
		int m_VertexSize_Position;
		int m_VertexSize_BoneWeight;
		int m_VertexSize_BoneMatrixIndex;
		int m_VertexSize_Normal;
		int m_VertexSize_Color;
		int m_VertexSize_Specular;
		int m_VertexSize_TexCoord[8];
		int m_VertexSize_TangentS;
		int m_VertexSize_TangentT;
		int m_VertexSize_Wrinkle;
		int m_VertexSize_UserData;
		int m_ActualVertexSize;
		VertexCompressionType_t m_CompressionType;
		int m_NumBoneWeights;
		float* m_pPosition;
		float* m_pBoneWeight;
		unsigned __int8* m_pBoneMatrixIndex;
		float* m_pNormal;
		unsigned __int8* m_pColor;
		unsigned __int8* m_pSpecular;
		float* m_pTexCoord[8];
		float* m_pTangentS;
		float* m_pTangentT;
		float* m_pWrinkle;
		float* m_pUserData;
		int m_nFirstVertex;
		unsigned int m_nOffset;
	};

	struct IMesh;
	struct IIndexBuffer_vtbl;
	struct IVertexBuffer_vtbl;
	struct IMaterial;
	struct IMaterial_vtbl;
	struct IMaterialVar_vtbl;

	struct IVertexBuffer
	{
		IVertexBuffer_vtbl* vftable;
	};

	struct IVertexBuffer_vtbl
	{
		int(__thiscall* VertexCount)(IVertexBuffer*);
		unsigned __int64(__thiscall* GetVertexFormat)(IVertexBuffer*);
		bool(__thiscall* IsDynamic)(IVertexBuffer*);
		void(__thiscall* BeginCastBuffer)(IVertexBuffer*, unsigned __int64);
		void(__thiscall* EndCastBuffer)(IVertexBuffer*);
		int(__thiscall* GetRoomRemaining)(IVertexBuffer*);
		bool(__thiscall* Lock)(IVertexBuffer*, int, bool, VertexDesc_t*);
		void(__thiscall* Unlock)(IVertexBuffer*, int, VertexDesc_t*);
		void(__thiscall* Spew)(IVertexBuffer*, int, const VertexDesc_t*);
		void(__thiscall* ValidateData)(IVertexBuffer*, int, const VertexDesc_t*);
	};

	struct IIndexBuffer
	{
		IIndexBuffer_vtbl* vftable;
	};

	struct IIndexBuffer_vtbl
	{
		int(__thiscall* IndexCount)(IIndexBuffer*);
		MaterialIndexFormat_t(__thiscall* IndexFormat)(IIndexBuffer*);
		bool(__thiscall* IsDynamic)(IIndexBuffer*);
		void(__thiscall* BeginCastBuffer)(IIndexBuffer*, MaterialIndexFormat_t);
		void(__thiscall* EndCastBuffer)(IIndexBuffer*);
		int(__thiscall* GetRoomRemaining)(IIndexBuffer*);
		bool(__thiscall* Lock)(IIndexBuffer*, int, bool, IndexDesc_t*);
		void(__thiscall* Unlock)(IIndexBuffer*, int, IndexDesc_t*);
		void(__thiscall* ModifyBegin)(IIndexBuffer*, bool, int, int, IndexDesc_t*);
		void(__thiscall* ModifyEnd)(IIndexBuffer*, IndexDesc_t*);
		void(__thiscall* Spew)(IIndexBuffer*, int, const IndexDesc_t*);
		void(__thiscall* ValidateData)(IIndexBuffer*, int, const IndexDesc_t*);
		IMesh* (__thiscall* GetMesh)(IIndexBuffer*);
	};

	struct IMaterialVar
	{
		IMaterialVar_vtbl* vftable;
		char* m_pStringVal;
		int m_intVal;
		Vector4D m_VecVal;
		unsigned __int8 m_Type : 4;
		unsigned __int8 m_nNumVectorComps : 3;
		unsigned __int8 m_bFakeMaterialVar : 1;
		unsigned __int8 m_nTempIndex;
		CUtlSymbol m_Name;
	};

	struct ITexture
	{
		void* vtbl;
	};

	struct IMaterialVar_vtbl
	{
		ITexture* (__thiscall* GetTextureValue)(IMaterialVar*);
		//bool(__thiscall* IsTextureValueInternalEnvCubemap)(IMaterialVar*); // nope
		const char* (__fastcall* GetName)(IMaterialVar*);
		unsigned __int16(__thiscall* GetNameAsSymbol)(IMaterialVar*);
		void(__thiscall* SetFloatValue)(IMaterialVar*, float);
		void(__thiscall* SetIntValue)(IMaterialVar*, int);
		void(__thiscall* SetStringValue)(IMaterialVar*, const char*);
		const char* (__thiscall* GetStringValue)(IMaterialVar*);
		void(__thiscall* SetFourCCValue)(IMaterialVar*, unsigned int, void*);
		void(__thiscall* GetFourCCValue)(IMaterialVar*, unsigned int*, void**);
		void(__thiscall* SetVecValue0)(IMaterialVar*, float, float, float, float);
		void(__thiscall* SetVecValue1)(IMaterialVar*, float, float, float);
		void(__thiscall* SetVecValue2)(IMaterialVar*, float, float);
		void(__thiscall* SetVecValue3)(IMaterialVar*, const float*, int);
		void(__thiscall* GetLinearVecValue)(IMaterialVar*, float*, int);
		void(__thiscall* SetTextureValue)(IMaterialVar*, ITexture*);
		IMaterial* (__thiscall* GetMaterialValue)(IMaterialVar*);
		void(__thiscall* SetMaterialValue)(IMaterialVar*, IMaterial*);
		bool(__thiscall* IsDefined)(IMaterialVar*);
		void(__thiscall* SetUndefined)(IMaterialVar*);
		void(__thiscall* SetMatrixValue)(IMaterialVar*, const VMatrix*);
		const VMatrix* (__thiscall* GetMatrixValue)(IMaterialVar*);
		bool(__thiscall* MatrixIsIdentity)(IMaterialVar*);
		void(__thiscall* CopyFrom)(IMaterialVar*, IMaterialVar*);
		void(__thiscall* SetValueAutodetectType)(IMaterialVar*, const char*);
		IMaterial* (__thiscall* GetOwningMaterial)(IMaterialVar*);
		void(__thiscall* SetVecComponentValue)(IMaterialVar*, float, int);
		int(__thiscall* GetIntValueInternal)(IMaterialVar*);
		float(__thiscall* GetFloatValueInternal)(IMaterialVar*);
		void(__thiscall* GetVecValueInternal0)(IMaterialVar*, float*, int);
		const float* (__thiscall* GetVecValueInternal1)(IMaterialVar*);
		int(__thiscall* VectorSizeInternal)(IMaterialVar*);

	};

	struct IMaterial
	{
		IMaterial_vtbl* vftable;
	};

	struct IMaterial_vtbl
	{
		//const char* (__thiscall* GetName)(IMaterial*);
		const char* (__fastcall* GetName)(IMaterial*);
		const char* (__thiscall* GetTextureGroupName)(IMaterial*);
		PreviewImageRetVal_t(__thiscall* GetPreviewImageProperties)(IMaterial*, int*, int*, ImageFormat*, bool*);
		PreviewImageRetVal_t(__thiscall* GetPreviewImage)(IMaterial*, unsigned __int8*, int, int, ImageFormat);
		int(__thiscall* GetMappingWidth)(IMaterial*);
		int(__thiscall* GetMappingHeight)(IMaterial*);
		int(__thiscall* GetNumAnimationFrames)(IMaterial*);
		bool(__thiscall* InMaterialPage)(IMaterial*);
		void(__thiscall* GetMaterialOffset)(IMaterial*, float*);
		void(__thiscall* GetMaterialScale)(IMaterial*, float*);
		IMaterial* (__thiscall* GetMaterialPage)(IMaterial*);
		IMaterialVar* (__thiscall* FindVar)(IMaterial*, const char*, bool*, bool);
		void(__thiscall* IncrementReferenceCount)(IMaterial*);
		void(__thiscall* DecrementReferenceCount)(IMaterial*);
		int(__thiscall* GetEnumerationID)(IMaterial*);
		void(__thiscall* GetLowResColorSample)(IMaterial*, float, float, float*);
		void(__thiscall* RecomputeStateSnapshots)(IMaterial*);
		bool(__thiscall* IsTranslucent)(IMaterial*);
		bool(__thiscall* IsAlphaTested)(IMaterial*);
		bool(__thiscall* IsVertexLit)(IMaterial*);
		unsigned __int64(__thiscall* GetVertexFormat)(IMaterial*);
		bool(__thiscall* HasProxy)(IMaterial*);
		bool(__thiscall* UsesEnvCubemap)(IMaterial*);
		bool(__thiscall* NeedsTangentSpace)(IMaterial*);
		bool(__thiscall* NeedsPowerOfTwoFrameBufferTexture)(IMaterial*, bool);
		bool(__thiscall* NeedsFullFrameBufferTexture)(IMaterial*, bool);
		bool(__thiscall* NeedsSoftwareSkinning)(IMaterial*);
		void(__thiscall* AlphaModulate)(IMaterial*, float);
		void(__thiscall* ColorModulate)(IMaterial*, float, float, float);
		void(__thiscall* SetMaterialVarFlag)(IMaterial*, MaterialVarFlags_t, bool);
		bool(__thiscall* GetMaterialVarFlag)(IMaterial*, MaterialVarFlags_t);
		void(__thiscall* GetReflectivity)(IMaterial*, Vector*);
		bool(__thiscall* GetPropertyFlag)(IMaterial*, MaterialPropertyTypes_t);
		bool(__thiscall* IsTwoSided)(IMaterial*);
		void(__thiscall* SetShader)(IMaterial*, const char*);
		int(__thiscall* GetNumPasses)(IMaterial*);
		int(__thiscall* GetTextureMemoryBytes)(IMaterial*);
		void(__thiscall* Refresh)(IMaterial*);
		bool(__thiscall* NeedsLightmapBlendAlpha)(IMaterial*);
		bool(__thiscall* NeedsSoftwareLighting)(IMaterial*);
		int(__thiscall* ShaderParamCount)(IMaterial*);
		IMaterialVar** (__thiscall* GetShaderParams)(IMaterial*);
		bool(__thiscall* IsErrorMaterial)(IMaterial*);
		void(__thiscall* Unused)(IMaterial*);
		float(__thiscall* GetAlphaModulation)(IMaterial*);
		void(__thiscall* GetColorModulation)(IMaterial*, float*, float*, float*);
		bool(__thiscall* IsTranslucentUnderModulation)(IMaterial*, float);
		IMaterialVar* (__thiscall* FindVarFast)(IMaterial*, const char*, unsigned int*);
		void(__thiscall* SetShaderAndParams)(IMaterial*, KeyValues*);
		const char* (__fastcall* GetShaderName)(IMaterial*);
		void(__thiscall* DeleteIfUnreferenced)(IMaterial*);
		bool(__thiscall* IsSpriteCard)(IMaterial*);
		void(__thiscall* CallBindProxy)(IMaterial*, void*, void*); // ICallQueue
		void(__thiscall* RefreshPreservingMaterialVars)(IMaterial*);
		bool(__thiscall* WasReloadedFromWhitelist)(IMaterial*);
	};

	struct IMaterialInternal_vtbl;
	struct IMaterialInternal /*: IMaterial*/
	{
		IMaterialInternal_vtbl* vftable;
	};

	struct IMaterialInternal_vtbl
	{
		const char* (__fastcall* GetName)(IMaterialInternal*);
		const char* (__thiscall* GetTextureGroupName)(IMaterialInternal*);
		PreviewImageRetVal_t(__thiscall* GetPreviewImageProperties)(IMaterialInternal*, int*, int*, ImageFormat*, bool*);
		PreviewImageRetVal_t(__thiscall* GetPreviewImage)(IMaterialInternal*, unsigned __int8*, int, int, ImageFormat);
		int(__thiscall* GetMappingWidth)(IMaterialInternal*);
		int(__thiscall* GetMappingHeight)(IMaterialInternal*);
		int(__thiscall* GetNumAnimationFrames)(IMaterialInternal*);
		bool(__thiscall* InMaterialPage)(IMaterialInternal*);
		void(__thiscall* GetMaterialOffset)(IMaterialInternal*, float*);
		void(__thiscall* GetMaterialScale)(IMaterialInternal*, float*);
		IMaterialInternal* (__thiscall* GetMaterialPage)(IMaterialInternal*);
		IMaterialVar* (__fastcall* FindVar)(IMaterialInternal*, void* null, const char*, bool*, bool);
		void(__thiscall* IncrementReferenceCount)(IMaterialInternal*);
		void(__thiscall* DecrementReferenceCount)(IMaterialInternal*);
		int(__thiscall* GetEnumerationID)(IMaterialInternal*);
		void(__thiscall* GetLowResColorSample)(IMaterialInternal*, float, float, float*);
		void(__thiscall* RecomputeStateSnapshots)(IMaterialInternal*);
		bool(__thiscall* IsTranslucent)(IMaterialInternal*);
		bool(__thiscall* IsAlphaTested)(IMaterialInternal*);
		bool(__thiscall* IsVertexLit)(IMaterialInternal*);
		unsigned __int64(__fastcall* GetVertexFormat)(IMaterialInternal*, void* null);
		bool(__thiscall* HasProxy)(IMaterialInternal*);
		bool(__thiscall* UsesEnvCubemap)(IMaterialInternal*);
		bool(__thiscall* NeedsTangentSpace)(IMaterialInternal*);
		bool(__thiscall* NeedsPowerOfTwoFrameBufferTexture)(IMaterialInternal*, bool);
		bool(__thiscall* NeedsFullFrameBufferTexture)(IMaterialInternal*, bool);
		bool(__thiscall* NeedsSoftwareSkinning)(IMaterialInternal*);
		void(__thiscall* AlphaModulate)(IMaterialInternal*, float);
		void(__thiscall* ColorModulate)(IMaterialInternal*, float, float, float);
		void(__thiscall* SetMaterialVarFlag)(IMaterialInternal*, MaterialVarFlags_t, bool);
		bool(__fastcall* GetMaterialVarFlag)(IMaterialInternal*, void* null, MaterialVarFlags_t);
		void(__thiscall* GetReflectivity)(IMaterialInternal*, Vector*);
		bool(__thiscall* GetPropertyFlag)(IMaterialInternal*, MaterialPropertyTypes_t);
		bool(__thiscall* IsTwoSided)(IMaterialInternal*);
		void(__thiscall* SetShader)(IMaterialInternal*, const char*);
		int(__thiscall* GetNumPasses)(IMaterialInternal*);
		int(__thiscall* GetTextureMemoryBytes)(IMaterialInternal*);
		void(__thiscall* Refresh)(IMaterialInternal*);
		bool(__thiscall* NeedsLightmapBlendAlpha)(IMaterialInternal*);
		bool(__thiscall* NeedsSoftwareLighting)(IMaterialInternal*);
		int(__thiscall* ShaderParamCount)(IMaterialInternal*);
		IMaterialVar** (__thiscall* GetShaderParams)(IMaterialInternal*);
		bool(__thiscall* IsErrorMaterial)(IMaterialInternal*);
		void(__thiscall* Unused)(IMaterialInternal*);
		float(__thiscall* GetAlphaModulation)(IMaterialInternal*);
		void(__thiscall* GetColorModulation)(IMaterialInternal*, float*, float*, float*);
		bool(__thiscall* IsTranslucentUnderModulation)(IMaterialInternal*, float);
		IMaterialVar* (__fastcall* FindVarFast)(IMaterialInternal*, void* null, const char*, unsigned int*);
		void(__thiscall* SetShaderAndParams)(IMaterialInternal*, KeyValues*);
		const char* (__fastcall* GetShaderName)(IMaterialInternal*);
		void(__thiscall* DeleteIfUnreferenced)(IMaterialInternal*);
		bool(__thiscall* IsSpriteCard)(IMaterialInternal*);
		void(__thiscall* CallBindProxy)(IMaterialInternal*, void*, void*); // ICallQueue
		void(__thiscall* RefreshPreservingMaterialVars)(IMaterialInternal*);
		bool(__thiscall* WasReloadedFromWhitelist)(IMaterialInternal*);
		int(__thiscall* GetReferenceCount)(IMaterialInternal*);
		void(__thiscall* SetEnumerationID)(IMaterialInternal*, int);
		void(__thiscall* SetNeedsWhiteLightmap)(IMaterialInternal*, bool);
		bool(__thiscall* GetNeedsWhiteLightmap)(IMaterialInternal*);
		void(__thiscall* Uncache)(IMaterialInternal*, bool);
		void(__thiscall* Precache)(IMaterialInternal*);
		bool(__thiscall* PrecacheVars)(IMaterialInternal*, KeyValues*, KeyValues*, void*, void*, int); // CUtlVector CUtlMemory
		void(__thiscall* ReloadTextures)(IMaterialInternal*);
		void(__thiscall* SetMinLightmapPageID)(IMaterialInternal*, int);
		void(__thiscall* SetMaxLightmapPageID)(IMaterialInternal*, int);
		int(__thiscall* GetMinLightmapPageID)(IMaterialInternal*);
		int(__thiscall* GetMaxLightmapPageID)(IMaterialInternal*);
		void* (__thiscall* GetShader)(IMaterialInternal*); // IShader
		bool(__thiscall* IsPrecached)(IMaterialInternal*);
		bool(__thiscall* IsPrecachedVars)(IMaterialInternal*);
		void(__thiscall* DrawMesh)(IMaterialInternal*, VertexCompressionType_t, bool, bool);
		unsigned __int64(__thiscall* GetVertexUsage)(IMaterialInternal*);
		bool(__thiscall* PerformDebugTrace)(IMaterialInternal*);
		bool(__thiscall* NoDebugOverride)(IMaterialInternal*);
		void(__thiscall* ToggleSuppression)(IMaterialInternal*);
		bool(__thiscall* IsSuppressed)(IMaterialInternal*);
		void(__thiscall* ToggleDebugTrace)(IMaterialInternal*);
		bool(__thiscall* UseFog)(IMaterialInternal*);
		void(__fastcall* AddMaterialVar)(IMaterialInternal*, void* null, IMaterialVar*);
		struct ShaderRenderState_t* (__thiscall* GetRenderState)(IMaterialInternal*);
		bool(__thiscall* IsManuallyCreated)(IMaterialInternal*);
		bool(__thiscall* NeedsFixedFunctionFlashlight)(IMaterialInternal*);
		bool(__thiscall* IsUsingVertexID)(IMaterialInternal*);
		void(__thiscall* MarkAsPreloaded)(IMaterialInternal*, bool);
		bool(__thiscall* IsPreloaded)(IMaterialInternal*);
		void(__thiscall* ArtificialAddRef)(IMaterialInternal*);
		void(__thiscall* ArtificialRelease)(IMaterialInternal*);
		void(__thiscall* ReportVarChanged)(IMaterialInternal*, struct IMaterialVar*);
		unsigned int(__thiscall* GetChangeID)(IMaterialInternal*);
		bool(__thiscall* IsTranslucentInternal)(IMaterialInternal*, float);
		bool(__thiscall* IsRealTimeVersion)(IMaterialInternal*);
		void(__thiscall* ClearContextData)(IMaterialInternal*);
		IMaterialInternal* (__thiscall* GetRealTimeVersion)(IMaterialInternal*);
		IMaterialInternal* (__thiscall* GetQueueFriendlyVersion)(IMaterialInternal*);
		void(__thiscall* PrecacheMappingDimensions)(IMaterialInternal*);
		void(__thiscall* FindRepresentativeTexture)(IMaterialInternal*);
		void(__thiscall* DecideShouldReloadFromWhitelist)(IMaterialInternal*, struct IFileList*);
		void(__thiscall* ReloadFromWhitelistIfMarked)(IMaterialInternal*);
		void(__thiscall* CompactMaterialVars)(IMaterialInternal*);
	};



	struct VisibleFogVolumeInfo_t
	{
		int m_nVisibleFogVolume;
		int m_nVisibleFogVolumeLeaf;
		bool m_bEyeInFogVolume;
		float m_flDistanceToWater;
		float m_flWaterHeight;
		IMaterial* m_pFogVolumeMaterial;
	};

	struct IMesh : IVertexBuffer, IIndexBuffer
	{
	};

	/*struct __declspec(align(1)) OptimizedModel_StripHeader_t
	{
	  int numIndices;
	  int indexOffset;
	  int numVerts;
	  int vertOffset;
	  __int16 numBones;
	  unsigned __int8 flags;
	  int numBoneStateChanges;
	  int boneStateChangeOffset;
	  int numTopologyIndices;
	  int topologyOffset;
	};*/

	struct studiomeshgroup_t
	{
		IMesh* m_pMesh;
		int m_NumStrips;
		int m_Flags;
		void* m_pStripData; // OptimizedModel_StripHeader_t
		unsigned __int16* m_pGroupIndexToMeshIndex;
		int m_NumVertices;
		int* m_pUniqueFaces;
		unsigned __int16* m_pIndices;
		unsigned __int16* m_pTopologyIndices;
		bool m_MeshNeedsRestore;
		__int16 m_ColorMeshID;
		struct IMorph* m_pMorph;
	};

	struct studiomeshdata_t
	{
		int m_NumGroup;
		studiomeshgroup_t* m_pMeshGroup;
	};

	struct studioloddata_t
	{
		studiomeshdata_t* m_pMeshData;
		float m_SwitchPoint;
		int numMaterials;
		IMaterial** ppMaterials;
		int* pMaterialFlags;
		int m_NumFaces;
		int* m_pHWMorphDecalBoneRemap;
		int m_nDecalBoneCount;
	};

	struct studiohwdata_t
	{
		int m_RootLOD;
		int m_NumLODs;
		studioloddata_t* m_pLODs;
		int m_NumStudioMeshes;
		int m_NumFacesRenderedThisFrame;
		int m_NumTimesRenderedThisFrame;
		studiohdr_t* m_pStudioHdr;
	};

	struct StudioDecalHandle_t__
	{
		int unused;
	};

	struct IClientRenderable_vtbl;
	struct IClientRenderable
	{
		IClientRenderable_vtbl* vftable;
	};

	const struct RenderableInstance_t
	{
		unsigned __int8 m_nAlpha;
	};

	struct IClientRenderable_vtbl
	{
		void* (__thiscall* GetIClientUnknown)(IClientRenderable*); // IClientUnknown
		const Vector* (__thiscall* GetRenderOrigin)(IClientRenderable*);
		const QAngle* (__thiscall* GetRenderAngles)(IClientRenderable*);
		bool(__thiscall* ShouldDraw)(IClientRenderable*);
		int(__thiscall* GetRenderFlags)(IClientRenderable*);
		void(__thiscall* Unused)(IClientRenderable*);
		unsigned __int16(__thiscall* GetShadowHandle)(IClientRenderable*);
		unsigned __int16* (__thiscall* RenderHandle)(IClientRenderable*);
		const model_t* (__thiscall* GetModel)(IClientRenderable*);
		int(__thiscall* DrawModel)(IClientRenderable*, int, const RenderableInstance_t*);
		int(__thiscall* GetBody)(IClientRenderable*);
		void(__thiscall* GetColorModulation)(IClientRenderable*, float*);
		bool(__thiscall* LODTest)(IClientRenderable*);
		bool(__thiscall* SetupBones)(IClientRenderable*, matrix3x4_t*, int, int, float);
		void(__thiscall* SetupWeights)(IClientRenderable*, const matrix3x4_t*, int, float*, float*);
		void(__thiscall* DoAnimationEvents)(IClientRenderable*);
		void* (__thiscall* GetPVSNotifyInterface)(IClientRenderable*); // IPVSNotify
		void(__thiscall* GetRenderBounds)(IClientRenderable*, Vector*, Vector*);
		void(__thiscall* GetRenderBoundsWorldspace)(IClientRenderable*, Vector*, Vector*);
		void(__thiscall* GetShadowRenderBounds)(IClientRenderable*, Vector*, Vector*, ShadowType_t);
		bool(__thiscall* ShouldReceiveProjectedTextures)(IClientRenderable*, int);
		bool(__thiscall* GetShadowCastDistance)(IClientRenderable*, float*, ShadowType_t);
		bool(__thiscall* GetShadowCastDirection)(IClientRenderable*, Vector*, ShadowType_t);
		bool(__thiscall* IsShadowDirty)(IClientRenderable*);
		void(__thiscall* MarkShadowDirty)(IClientRenderable*, bool);
		IClientRenderable* (__thiscall* GetShadowParent)(IClientRenderable*);
		IClientRenderable* (__thiscall* FirstShadowChild)(IClientRenderable*);
		IClientRenderable* (__thiscall* NextShadowPeer)(IClientRenderable*);
		ShadowType_t(__thiscall* ShadowCastType)(IClientRenderable*);
		void(__thiscall* Unused2)(IClientRenderable*);
		void(__thiscall* CreateModelInstance)(IClientRenderable*);
		unsigned __int16(__thiscall* GetModelInstance)(IClientRenderable*);
		const matrix3x4_t* (__thiscall* RenderableToWorldTransform)(IClientRenderable*);
		int(__thiscall* LookupAttachment)(IClientRenderable*, const char*);
		bool(__thiscall* GetAttachment0)(IClientRenderable*, int, matrix3x4_t*);
		bool(__thiscall* GetAttachment1)(IClientRenderable*, int, Vector*, QAngle*);
		float* (__thiscall* GetRenderClipPlane)(IClientRenderable*);
		int(__thiscall* GetSkin)(IClientRenderable*);
		void(__thiscall* OnThreadedDrawSetup)(IClientRenderable*);
		bool(__thiscall* UsesFlexDelayedWeights)(IClientRenderable*);
		void(__thiscall* RecordToolMessage)(IClientRenderable*);
		bool(__thiscall* ShouldDrawForSplitScreenUser)(IClientRenderable*, int);
		unsigned __int8(__thiscall* OverrideAlphaModulation)(IClientRenderable*, unsigned __int8);
		unsigned __int8(__thiscall* OverrideShadowAlphaModulation)(IClientRenderable*, unsigned __int8);
		void* (__thiscall* GetClientModelRenderable)(IClientRenderable*); // IClientModelRenderable
	};

	struct ColorRGBExp32
	{
		unsigned __int8 r;
		unsigned __int8 g;
		unsigned __int8 b;
		char exponent;
	};

	struct dlight_t
	{
		int flags;
		Vector origin;
		float radius;
		ColorRGBExp32 color;
		float die;
		float decay;
		float minlight;
		int key;
		int style;
		Vector m_Direction;
		float m_InnerAngle;
		float m_OuterAngle;
		const IClientRenderable* m_pExclusiveLightReceiver;
	};

	struct __declspec(align(4)) ModelRenderInfo_t
	{
		Vector origin;
		QAngle angles;
		IClientRenderable* pRenderable;
		const model_t* pModel;
		const matrix3x4_t* pModelToWorld;
		const matrix3x4_t* pLightingOffset;
		const Vector* pLightingOrigin;
		int flags;
		int entity_index;
		int skin;
		int body;
		int hitboxset;
		unsigned __int16 instance;
	};

	struct ColorMeshInfo_t
	{
		IMesh* m_pMesh;
		void* m_pPooledVBAllocator; // IPooledVBAllocator
		int m_nVertOffsetInBytes;
		int m_nNumVerts;
	};

	struct LightDesc_t
	{
		LightType_t m_Type;
		Vector m_Color;
		Vector m_Position;
		Vector m_Direction;
		float m_Range;
		float m_Falloff;
		float m_Attenuation0;
		float m_Attenuation1;
		float m_Attenuation2;
		float m_Theta;
		float m_Phi;
		float m_ThetaDot;
		float m_PhiDot;
		float m_OneOverThetaDotMinusPhiDot;
		unsigned int m_Flags;
		float m_RangeSquared;
	};

	struct MaterialLightingState_t
	{
		Vector m_vecAmbientCube[6];
		Vector m_vecLightingOrigin;
		int m_nLocalLightCount;
		LightDesc_t m_pLocalLightDesc[4];
	};

	struct DrawModelInfo_t
	{
		studiohdr_t* m_pStudioHdr;
		studiohwdata_t* m_pHardwareData;
		StudioDecalHandle_t__* m_Decals;
		int m_Skin;
		int m_Body;
		int m_HitboxSet;
		void* m_pClientEntity;
		int m_Lod;
		ColorMeshInfo_t* m_pColorMeshes;
		bool m_bStaticLighting;
		MaterialLightingState_t m_LightingState;
	};

	struct DrawModelState_t
	{
		studiohdr_t* m_pStudioHdr;
		studiohwdata_t* m_pStudioHWData;
		IClientRenderable* m_pRenderable;
		const matrix3x4_t* m_pModelToWorld;
		StudioDecalHandle_t__* m_decals;
		int m_drawFlags;
		int m_lod;
	};

	struct StaticPropRenderInfo_t
	{
		const matrix3x4_t* pModelToWorld;
		const model_t* pModel;
		IClientRenderable* pRenderable;
		Vector* pLightingOrigin;
		unsigned __int16 instance;
		unsigned __int8 skin;
		unsigned __int8 alpha;
	};

	struct CStudioHdr
	{
		const studiohdr_t* m_pStudioHdr;
		void* m_pVModel; // virtualmodel_t
		// ...
	};

	struct WorldListLeafData_t
	{
		unsigned __int16 leafIndex;
		__int16 waterData;
		unsigned __int16 firstTranslucentSurface;
		unsigned __int16 translucentSurfaceCount;
	};

	struct WorldListInfo_t
	{
		int m_ViewFogVolume;
		int m_LeafCount;
		bool m_bHasWater;
		WorldListLeafData_t* m_pLeafDataList;
	};

	struct VisOverrideData_t
	{
		Vector m_vecVisOrigin;
		float m_fDistToAreaPortalTolerance;
		Vector m_vPortalCorners[4];
		bool m_bTrimFrustumToPortalCorners;
		Vector m_vPortalOrigin;
		Vector m_vPortalForward;
		float m_flPortalRadius;
	};

	enum MotionBlurMode_t : __int32
	{
		MOTION_BLUR_DISABLE = 0x1,
		MOTION_BLUR_GAME = 0x2,
		MOTION_BLUR_SFM = 0x3,
	};

	struct __declspec(align(4)) CViewSetup
	{
		int x;
		int m_nUnscaledX;
		int y;
		int m_nUnscaledY;
		int width;
		int m_nUnscaledWidth;
		int height;
		int m_nUnscaledHeight;
		bool m_bOrtho;
		float m_OrthoLeft;
		float m_OrthoTop;
		float m_OrthoRight;
		float m_OrthoBottom;
		bool m_bCustomViewMatrix;
		matrix3x4_t m_matCustomViewMatrix;
		float fov;
		float fovViewmodel;
		Vector origin;
		QAngle angles;
		float zNear;
		float zFar;
		float zNearViewmodel;
		float zFarViewmodel;
		float m_flAspectRatio;
		float m_flNearBlurDepth;
		float m_flNearFocusDepth;
		float m_flFarFocusDepth;
		float m_flFarBlurDepth;
		float m_flNearBlurRadius;
		float m_flFarBlurRadius;
		int m_nDoFQuality;
		MotionBlurMode_t m_nMotionBlurMode;
		float m_flShutterTime;
		Vector m_vShutterOpenPosition;
		QAngle m_shutterOpenAngles;
		Vector m_vShutterClosePosition;
		QAngle m_shutterCloseAngles;
		float m_flOffCenterTop;
		float m_flOffCenterBottom;
		float m_flOffCenterLeft;
		float m_flOffCenterRight;
		__int8 m_bOffCenter : 1;
		__int8 m_bRenderToSubrectOfLargerScreen : 1;
		__int8 m_bDoBloomAndToneMapping : 1;
		__int8 m_bDoDepthOfField : 1;
		__int8 m_bHDRTarget : 1;
		__int8 m_bDrawWorldNormal : 1;
		__int8 m_bCullFrontFaces : 1;
		__int8 m_bCacheFullSceneState : 1;
	};

	struct IRender_vtbl;
	struct IRender
	{
		IRender_vtbl* vftable;
	};

	struct IRender_vtbl
	{
		void(__thiscall* FrameBegin)(IRender*);
		void(__thiscall* FrameEnd)(IRender*);
		void(__thiscall* ViewSetupVis)(IRender*, bool, int, const Vector*);
		void(__thiscall* ViewDrawFade)(IRender*, unsigned __int8*, IMaterial*);
		void(__thiscall* DrawSceneBegin)(IRender*);
		void(__thiscall* DrawSceneEnd)(IRender*);
		void* (__thiscall* CreateWorldList)(IRender*); // IWorldRenderList
		void(__thiscall* BuildWorldLists)(IRender*, void*, WorldListInfo_t*, int, const VisOverrideData_t*, bool, float*); // IWorldRenderList
		void(__thiscall* DrawWorldLists)(IRender*, void*, void*, unsigned int, float); // IMatRenderContext - IWorldRenderList
		const Vector* (__thiscall* ViewOrigin)(IRender*);
		const QAngle* (__thiscall* ViewAngles)(IRender*);
		const CViewSetup* (__thiscall* ViewGetCurrent)(IRender*);
		const VMatrix* (__thiscall* ViewMatrix)(IRender*);
		const VMatrix* (__thiscall* WorldToScreenMatrix)(IRender*);
		float(__thiscall* GetFramerate)(IRender*);
		float(__thiscall* GetZNear)(IRender*);
		float(__thiscall* GetZFar)(IRender*);
		float(__thiscall* GetFov)(IRender*);
		float(__thiscall* GetFovY)(IRender*);
		float(__thiscall* GetFovViewmodel)(IRender*);
		bool(__thiscall* ClipTransform)(IRender*, const Vector*, Vector*);
		bool(__thiscall* ScreenTransform)(IRender*, const Vector*, Vector*);
		void(__thiscall* Push3DView0)(IRender*, void*, const CViewSetup*, int, ITexture*, VPlane*, ITexture*); // IMatRenderContext
		void(__thiscall* Push3DView1)(IRender*, void*, const CViewSetup*, int, ITexture*, VPlane*); // IMatRenderContext
		void(__thiscall* Push2DView)(IRender*, void*, const CViewSetup*, int, ITexture*, VPlane*); // IMatRenderContext
		void(__thiscall* PopView)(IRender*, void*, VPlane*); // IMatRenderContext
		void(__thiscall* SetMainView)(IRender*, const Vector*, const QAngle*);
		void(__thiscall* ViewSetupVisEx)(IRender*, bool, int, const Vector*, unsigned int*);
		void(__thiscall* OverrideViewFrustum)(IRender*, VPlane*);
		void(__thiscall* UpdateBrushModelLightmap)(IRender*, model_t*, IClientRenderable*);
		void(__thiscall* BeginUpdateLightmaps)(IRender*);
		void(__thiscall* EndUpdateLightmaps)(IRender*);
		bool(__thiscall* InLightmapUpdate)(IRender*);
	};

	struct __declspec(align(4)) ViewStack_t
	{
		CViewSetup m_View;
		VMatrix m_matrixView;
		VMatrix m_matrixProjection;
		VMatrix m_matrixWorldToScreen;
		bool m_bIs2DView;
		bool m_bNoDraw;
	};

	struct CRender_vtbl;
	struct __declspec(align(8)) CRender : IRender
	{
		//CRender_vtbl* vftable;
		float m_yFOV;
		long double m_frameStartTime;
		float m_framerate;
		float m_zNear;
		float m_zFar;
		VMatrix m_matrixView;
		VMatrix m_matrixProjection;
		VMatrix m_matrixWorldToScreen;
		//CUtlStack<CRender::ViewStack_t, CUtlMemory<CRender::ViewStack_t, int> > m_ViewStack;
		char pad_m_ViewStack_memory[0xC];
		int m_ViewStack_size;
		ViewStack_t* m_ViewStack_m_pElements;
		//int m_iLightmapUpdateDepth;
	}; STATIC_ASSERT_OFFSET(CRender, m_ViewStack_size, 0xDC + 0xC);

	struct CRender_vtbl
	{
		void(__thiscall* FrameBegin)(IRender*);
		void(__thiscall* FrameEnd)(IRender*);
		void(__thiscall* ViewSetupVis)(IRender*, bool, int, const Vector*);
		void(__thiscall* ViewDrawFade)(IRender*, unsigned __int8*, IMaterial*);
		void(__thiscall* DrawSceneBegin)(IRender*);
		void(__thiscall* DrawSceneEnd)(IRender*);
		void* (__thiscall* CreateWorldList)(IRender*); // IWorldRenderList
		void(__thiscall* BuildWorldLists)(IRender*, void*, WorldListInfo_t*, int, const VisOverrideData_t*, bool, float*); // IWorldRenderList
		void(__thiscall* DrawWorldLists)(IRender*, void*, void*, unsigned int, float); // IWorldRenderList - IWorldRenderList
		const Vector* (__thiscall* ViewOrigin)(IRender*);
		const QAngle* (__thiscall* ViewAngles)(IRender*);
		const CViewSetup* (__thiscall* ViewGetCurrent)(IRender*);
		const VMatrix* (__thiscall* ViewMatrix)(IRender*);
		const VMatrix* (__thiscall* WorldToScreenMatrix)(IRender*);
		float(__thiscall* GetFramerate)(IRender*);
		float(__thiscall* GetZNear)(IRender*);
		float(__thiscall* GetZFar)(IRender*);
		float(__thiscall* GetFov)(IRender*);
		float(__thiscall* GetFovY)(IRender*);
		float(__thiscall* GetFovViewmodel)(IRender*);
		bool(__thiscall* ClipTransform)(IRender*, const Vector*, Vector*);
		bool(__thiscall* ScreenTransform)(IRender*, const Vector*, Vector*);
		void(__thiscall* Push3DView0)(IRender*, void*, const CViewSetup*, int, ITexture*, VPlane*, ITexture*); // IMatRenderContext
		void(__thiscall* Push3DView1)(IRender*, void*, const CViewSetup*, int, ITexture*, VPlane*); // IMatRenderContext
		void(__thiscall* Push2DView)(IRender*, void*, const CViewSetup*, int, ITexture*, VPlane*); // IMatRenderContext
		void(__thiscall* PopView)(IRender*, void*, VPlane*); // IMatRenderContext
		void(__thiscall* SetMainView)(IRender*, const Vector*, const QAngle*);
		void(__thiscall* ViewSetupVisEx)(IRender*, bool, int, const Vector*, unsigned int*);
		void(__thiscall* OverrideViewFrustum)(IRender*, VPlane*);
		void(__thiscall* UpdateBrushModelLightmap)(IRender*, model_t*, IClientRenderable*);
		void(__thiscall* BeginUpdateLightmaps)(IRender*);
		void(__thiscall* EndUpdateLightmaps)(IRender*);
		bool(__thiscall* InLightmapUpdate)(IRender*);
	};

	enum StreamSpec_t : __int32
	{
		STREAM_DEFAULT = 0x0,
		STREAM_SPECULAR1 = 0x1,
		STREAM_FLEXDELTA = 0x2,
		STREAM_MORPH = 0x3,
		STREAM_UNIQUE_A = 0x4,
		STREAM_UNIQUE_B = 0x5,
		STREAM_UNIQUE_C = 0x6,
		STREAM_UNIQUE_D = 0x7,
		STREAM_SUBDQUADS = 0x8,
	};

	struct __declspec(align(8)) VertexStreamSpec_t
	{
		unsigned __int64 iVertexDataElement;
		StreamSpec_t iStreamSpec;
	};

	struct CPrimList
	{
		int m_FirstIndex;
		int m_NumIndices;
	};

	struct MeshDesc_t : VertexDesc_t, IndexDesc_t
	{
	};

	struct CMeshBase : IMesh
	{
	};

	struct CMeshBase_vtbl
	{
		int(__thiscall* VertexCount)(IVertexBuffer*);
		unsigned __int64(__thiscall* GetVertexFormat)(IVertexBuffer*);
		bool(__thiscall* IsDynamic)(IVertexBuffer*);
		void(__thiscall* BeginCastBuffer)(IVertexBuffer*, unsigned __int64);
		void(__thiscall* EndCastBuffer)(IVertexBuffer*);
		int(__thiscall* GetRoomRemaining)(IVertexBuffer*);
		bool(__thiscall* Lock)(IVertexBuffer*, int, bool, VertexDesc_t*);
		void(__thiscall* Unlock)(IVertexBuffer*, int, VertexDesc_t*);
		void(__thiscall* Spew0)(IVertexBuffer*, int, const VertexDesc_t*);
		void(__thiscall* ValidateData0)(IVertexBuffer*, int, const VertexDesc_t*);
		void(__thiscall* SetPrimitiveType)(IMesh*, MaterialPrimitiveType_t);
		void(__thiscall* Draw0)(IMesh*, CPrimList*, int);
		void(__thiscall* Draw1)(IMesh*, int, int);
		void(__thiscall* SetColorMesh)(IMesh*, IMesh*, int);
		void(__thiscall* CopyToMeshBuilder)(IMesh*, int, int, int, int, int, void*); // CMeshBuilder
		void(__thiscall* Spew1)(IMesh*, int, int, const MeshDesc_t*);
		void(__thiscall* ValidateData1)(IMesh*, int, int, const MeshDesc_t*);
		void(__thiscall* LockMesh)(IMesh*, int, int, MeshDesc_t*, void*); // MeshBuffersAllocationSettings_t
		void(__thiscall* ModifyBegin)(IMesh*, int, int, int, int, MeshDesc_t*);
		void(__thiscall* ModifyEnd)(IMesh*, MeshDesc_t*);
		void(__thiscall* UnlockMesh)(IMesh*, int, int, MeshDesc_t*);
		void(__thiscall* ModifyBeginEx)(IMesh*, bool, int, int, int, int, MeshDesc_t*);
		void(__thiscall* SetFlexMesh)(IMesh*, IMesh*, int);
		void(__thiscall* DisableFlexMesh)(IMesh*);
		void(__thiscall* MarkAsDrawn)(IMesh*);
		void(__thiscall* DrawModulated)(IMesh*, const Vector4D*, int, int);
		unsigned int(__thiscall* ComputeMemoryUsed)(IMesh*);
		void* (__thiscall* AccessRawHardwareDataStream)(IMesh*, unsigned __int8, unsigned int, unsigned int, void*);
		void* (__thiscall* GetCachedPerFrameMeshData)(IMesh*); // ICachedPerFrameMeshData
		void(__thiscall* ReconstructFromCachedPerFrameMeshData)(IMesh*, void*); // ICachedPerFrameMeshData
		void(__thiscall* BeginPass)(CMeshBase*);
		void(__thiscall* RenderPass)(CMeshBase*, const unsigned __int8*);
		bool(__thiscall* HasColorMesh)(CMeshBase*);
		bool(__thiscall* HasFlexMesh)(CMeshBase*);
		bool(__thiscall* IsUsingVertexID)(CMeshBase*);
		VertexStreamSpec_t* (__thiscall* GetVertexStreamSpec)(CMeshBase*);
		void(__thiscall * CMeshBase_destructor)(CMeshBase*);
	};

	struct CBaseMeshDX8 : CMeshBase
	{
		bool m_bMeshLocked;
		unsigned __int64 m_VertexFormat;
	};

	struct CBaseMeshDX8_vtbl
	{
		int(__thiscall* VertexCount)(IVertexBuffer*);
		unsigned __int64(__thiscall* GetVertexFormat)(IVertexBuffer*);
		bool(__thiscall* IsDynamic)(IVertexBuffer*);
		void(__thiscall* BeginCastBuffer)(IVertexBuffer*, unsigned __int64);
		void(__thiscall* EndCastBuffer)(IVertexBuffer*);
		int(__thiscall* GetRoomRemaining)(IVertexBuffer*);
		bool(__thiscall* Lock)(IVertexBuffer*, int, bool, VertexDesc_t*);
		void(__thiscall* Unlock)(IVertexBuffer*, int, VertexDesc_t*);
		void(__thiscall* Spew0)(IVertexBuffer*, int, const VertexDesc_t*);
		void(__thiscall* ValidateData0)(IVertexBuffer*, int, const VertexDesc_t*);
		void(__thiscall* SetPrimitiveType)(IMesh*, MaterialPrimitiveType_t);
		void(__thiscall* Draw0)(IMesh*, CPrimList*, int);
		void(__thiscall* Draw1)(IMesh*, int, int);
		void(__thiscall* SetColorMesh)(IMesh*, IMesh*, int);
		void(__thiscall* CopyToMeshBuilder)(IMesh*, int, int, int, int, int, void*); // CMeshBuilder
		void(__thiscall* Spew1)(IMesh*, int, int, const MeshDesc_t*);
		void(__thiscall* ValidateData1)(IMesh*, int, int, const MeshDesc_t*);
		void(__thiscall* LockMesh)(IMesh*, int, int, MeshDesc_t*, void*); // MeshBuffersAllocationSettings_t
		void(__thiscall* ModifyBegin)(IMesh*, int, int, int, int, MeshDesc_t*);
		void(__thiscall* ModifyEnd)(IMesh*, MeshDesc_t*);
		void(__thiscall* UnlockMesh)(IMesh*, int, int, MeshDesc_t*);
		void(__thiscall* ModifyBeginEx)(IMesh*, bool, int, int, int, int, MeshDesc_t*);
		void(__thiscall* SetFlexMesh)(IMesh*, IMesh*, int);
		void(__thiscall* DisableFlexMesh)(IMesh*);
		void(__thiscall* MarkAsDrawn)(IMesh*);
		void(__thiscall* DrawModulated)(IMesh*, const Vector4D*, int, int);
		unsigned int(__thiscall* ComputeMemoryUsed)(IMesh*);
		void* (__thiscall* AccessRawHardwareDataStream)(IMesh*, unsigned __int8, unsigned int, unsigned int, void*);
		void* (__thiscall* GetCachedPerFrameMeshData)(IMesh*); // ICachedPerFrameMeshData
		void(__thiscall* ReconstructFromCachedPerFrameMeshData)(IMesh*, void*); // ICachedPerFrameMeshData
		void(__thiscall* BeginPass)(CMeshBase*);
		void(__thiscall* RenderPass)(CMeshBase*, const unsigned __int8*);
		bool(__thiscall* HasColorMesh)(CMeshBase*);
		bool(__thiscall* HasFlexMesh)(CMeshBase*);
		bool(__thiscall* IsUsingVertexID)(CMeshBase*);
		VertexStreamSpec_t* (__thiscall* GetVertexStreamSpec)(CMeshBase*);
		void(__thiscall * CMeshBase_destructor)(CMeshBase*);
		void(__thiscall* SetVertexFormat)(CBaseMeshDX8*, unsigned __int64, bool, bool);
		bool(__thiscall* IsExternal)(CBaseMeshDX8*);
		void(__thiscall* SetMaterial)(CBaseMeshDX8*, IMaterial*);
		void(__thiscall* GetColorMesh)(CBaseMeshDX8*, const IVertexBuffer**, int*);
		void(__thiscall* HandleLateCreation)(CBaseMeshDX8*);
		MaterialPrimitiveType_t(__thiscall* GetPrimitiveType)(CBaseMeshDX8*);
		void* (__thiscall* GetVertexBuffer)(CBaseMeshDX8*); // CVertexBuffer
		void* (__thiscall* GetIndexBuffer)(CBaseMeshDX8*); // CIndexBuffer
		bool(__thiscall* NeedsVertexFormatReset)(CBaseMeshDX8*, unsigned __int64);
		bool(__thiscall* HasEnoughRoom)(CBaseMeshDX8*, int, int);
		void(__thiscall* PreLock)(CBaseMeshDX8*);
	};

	struct __declspec(align(8)) CMeshDX8 : CBaseMeshDX8
	{
		void* m_pVertexBuffer; // CVertexBuffer
		void* m_pIndexBuffer; // CIndexBuffer
		CMeshDX8* m_pColorMesh;
		int m_nColorMeshVertOffsetInBytes;
		unsigned __int64 m_fmtStreamSpec;
		void* m_pVertexStreamSpec; // CArrayAutoPtr
		void* m_pVbTexCoord1; // CVertexBuffer
		IDirect3DVertexBuffer9* m_arrRawHardwareDataStreams[1];
		void* m_pFlexVertexBuffer; // CVertexBuffer
		bool m_bHasRawHardwareDataStreams;
		bool m_bHasFlexVerts;
		int m_nFlexVertOffsetInBytes;
		int m_flexVertCount;
		MaterialPrimitiveType_t m_Type;
		_D3DPRIMITIVETYPE m_Mode;
		unsigned int m_NumIndices;
		unsigned __int16 m_NumVertices;
		bool m_IsVBLocked;
		bool m_IsIBLocked;
		int m_FirstIndex;
		const char* m_pTextureGroupName;
	};
	STATIC_ASSERT_OFFSET(CMeshDX8, m_Type, 0x4C);

	struct CMeshDX8_vtbl
	{
		int(__thiscall* VertexCount)(IVertexBuffer*);
		unsigned __int64(__thiscall* GetVertexFormat)(IVertexBuffer*);
		bool(__thiscall* IsDynamic)(IVertexBuffer*);
		void(__thiscall* BeginCastBuffer)(IVertexBuffer*, unsigned __int64);
		void(__thiscall* EndCastBuffer)(IVertexBuffer*);
		int(__thiscall* GetRoomRemaining)(IVertexBuffer*);
		bool(__thiscall* Lock)(IVertexBuffer*, int, bool, VertexDesc_t*);
		void(__thiscall* Unlock)(IVertexBuffer*, int, VertexDesc_t*);
		void(__thiscall* Spew0)(IVertexBuffer*, int, const VertexDesc_t*);
		void(__thiscall* ValidateData0)(IVertexBuffer*, int, const VertexDesc_t*);
		void(__thiscall* SetPrimitiveType)(IMesh*, MaterialPrimitiveType_t);
		void(__thiscall* Draw0)(IMesh*, CPrimList*, int);
		void(__thiscall* Draw1)(IMesh*, int, int);
		void(__thiscall* SetColorMesh)(IMesh*, IMesh*, int);
		void(__thiscall* CopyToMeshBuilder)(IMesh*, int, int, int, int, int, void*); // CMeshBuilder
		void(__thiscall* Spew1)(IMesh*, int, int, const MeshDesc_t*);
		void(__thiscall* ValidateData1)(IMesh*, int, int, const MeshDesc_t*);
		void(__thiscall* LockMesh)(IMesh*, int, int, MeshDesc_t*, void*); // MeshBuffersAllocationSettings_t
		void(__thiscall* ModifyBegin)(IMesh*, int, int, int, int, MeshDesc_t*);
		void(__thiscall* ModifyEnd)(IMesh*, MeshDesc_t*);
		void(__thiscall* UnlockMesh)(IMesh*, int, int, MeshDesc_t*);
		void(__thiscall* ModifyBeginEx)(IMesh*, bool, int, int, int, int, MeshDesc_t*);
		void(__thiscall* SetFlexMesh)(IMesh*, IMesh*, int);
		void(__thiscall* DisableFlexMesh)(IMesh*);
		void(__thiscall* MarkAsDrawn)(IMesh*);
		void(__thiscall* DrawModulated)(IMesh*, const Vector4D*, int, int);
		unsigned int(__thiscall* ComputeMemoryUsed)(IMesh*);
		void* (__thiscall* AccessRawHardwareDataStream)(IMesh*, unsigned __int8, unsigned int, unsigned int, void*);
		void* (__thiscall* GetCachedPerFrameMeshData)(IMesh*); // ICachedPerFrameMeshData
		void(__thiscall* ReconstructFromCachedPerFrameMeshData)(IMesh*, void*); // ICachedPerFrameMeshData
		void(__thiscall* BeginPass)(CMeshBase*);
		void(__thiscall* RenderPass)(CMeshBase*, const unsigned __int8*);
		bool(__thiscall* HasColorMesh)(CMeshBase*);
		bool(__thiscall* HasFlexMesh)(CMeshBase*);
		bool(__thiscall* IsUsingVertexID)(CMeshBase*);
		VertexStreamSpec_t* (__thiscall* GetVertexStreamSpec)(CMeshBase*);
		void(__thiscall * CMeshBase_destructor)(CMeshBase*);
		void(__thiscall* SetVertexFormat)(CBaseMeshDX8*, unsigned __int64, bool, bool);
		bool(__thiscall* IsExternal)(CBaseMeshDX8*);
		void(__thiscall* SetMaterial)(CBaseMeshDX8*, IMaterial*);
		void(__thiscall* GetColorMesh)(CBaseMeshDX8*, const IVertexBuffer**, int*);
		void(__thiscall* HandleLateCreation)(CBaseMeshDX8*);
		MaterialPrimitiveType_t(__thiscall* GetPrimitiveType)(CBaseMeshDX8*);
		void* (__thiscall* GetVertexBuffer)(CBaseMeshDX8*); // CVertexBuffer
		void* (__thiscall* GetIndexBuffer)(CBaseMeshDX8*); // CIndexBuffer
		bool(__thiscall* NeedsVertexFormatReset)(CBaseMeshDX8*, unsigned __int64);
		bool(__thiscall* HasEnoughRoom)(CBaseMeshDX8*, int, int);
		void(__thiscall* PreLock)(CBaseMeshDX8*);
	};

	const struct CBaseHandle
	{
		unsigned int m_Index;
	};


	struct IHandleEntity_vtbl;
	struct IHandleEntity
	{
		IHandleEntity_vtbl* vftable;
	};

	struct IHandleEntity_vtbl
	{
		void(__thiscall * IHandleEntity_destructor)(IHandleEntity*);
		void(__thiscall* SetRefEHandle)(IHandleEntity*, const CBaseHandle*);
		const CBaseHandle* (__thiscall* GetRefEHandle)(IHandleEntity*);
	};

	struct IClientUnknown : IHandleEntity
	{
	};

	struct IClientNetworkable_vtbl;
	struct IClientNetworkable
	{
		IClientNetworkable_vtbl* vftable;
	};

	struct IClientNetworkable_vtbl
	{
		IClientUnknown* (__thiscall* GetIClientUnknown)(IClientNetworkable*);
		//void(__thiscall* Release)(IClientNetworkable*);
		//ClientClass* (__thiscall* GetClientClass)(IClientNetworkable*);
		//void(__thiscall* NotifyShouldTransmit)(IClientNetworkable*, ShouldTransmitState_t);
		//void(__thiscall* OnPreDataChanged)(IClientNetworkable*, DataUpdateType_t);
		//void(__thiscall* OnDataChanged)(IClientNetworkable*, DataUpdateType_t);
		//void(__thiscall* PreDataUpdate)(IClientNetworkable*, DataUpdateType_t);
		//void(__thiscall* PostDataUpdate)(IClientNetworkable*, DataUpdateType_t);
		//void(__thiscall* OnDataUnchangedInPVS)(IClientNetworkable*);
		//bool(__thiscall* IsDormant)(IClientNetworkable*);
		//int(__thiscall* entindex)(IClientNetworkable*);
		//void(__thiscall* ReceiveMessage)(IClientNetworkable*, int, bf_read*);
		//void* (__thiscall* GetDataTableBasePtr)(IClientNetworkable*);
		//void(__thiscall* SetDestroyedOnRecreateEntities)(IClientNetworkable*);
	};

	struct IClientThinkable_vtbl;
	struct IClientThinkable
	{
		IClientThinkable_vtbl* vftable;
	};

	struct IClientThinkable_vtbl
	{
		IClientUnknown* (__thiscall* GetIClientUnknown)(IClientThinkable*);
		void(__thiscall* ClientThink)(IClientThinkable*);
		struct CClientThinkHandlePtr* (__thiscall* GetThinkHandle)(IClientThinkable*);
		void(__thiscall* SetThinkHandle)(IClientThinkable*, struct CClientThinkHandlePtr*);
		void(__thiscall* Release)(IClientThinkable*);
	};

	struct IClientEntity : IClientUnknown, IClientRenderable, IClientNetworkable, IClientThinkable
	{
	};

	struct C_BaseEntity;
	struct IClientEntity_vtbl
	{
		void(__thiscall * IHandleEntity_destructor)(IHandleEntity*);
		void(__thiscall* SetRefEHandle)(IHandleEntity*, const CBaseHandle*);
		const CBaseHandle* (__thiscall* GetRefEHandle)(IHandleEntity*);
		void* (__thiscall* GetCollideable)(IClientUnknown*); // ICollideable
		IClientNetworkable* (__thiscall* GetClientNetworkable)(IClientUnknown*);
		IClientRenderable* (__thiscall* GetClientRenderable)(IClientUnknown*);
		IClientEntity* (__thiscall* GetIClientEntity)(IClientUnknown*);
		C_BaseEntity* (__thiscall* GetBaseEntity)(IClientUnknown*);
		IClientThinkable* (__thiscall* GetClientThinkable)(IClientUnknown*);
		void* (__thiscall* GetClientAlphaProperty)(IClientUnknown*); // IClientAlphaProperty
		const Vector* (__thiscall* GetAbsOrigin)(IClientEntity*);
		const QAngle* (__thiscall* GetAbsAngles)(IClientEntity*);
		void* (__thiscall* GetMouth)(IClientEntity*); // CMouthInfo
		bool(__thiscall* GetSoundSpatialization)(IClientEntity*, void*); // SpatializationInfo_t
		bool(__thiscall* IsBlurred)(IClientEntity*);
	};

	//struct VarMapping_t
	//{
	//	//CUtlVector<VarMapEntry_t, CUtlMemory<VarMapEntry_t, int> > m_Entries;
	//	char pad[0x14];
	//	int m_nInterpolatedEntries;
	//	float m_lastInterpolationTime;
	//};
	//STATIC_ASSERT_SIZE(VarMapping_t, 0x1C);

	enum ModelDataCategory_t : __int32
	{
		MODEL_DATA_LIGHTING_MODEL = 0x0,
		MODEL_DATA_STENCIL = 0x1,
		MODEL_DATA_CATEGORY_COUNT = 0x2,
	};
	
	struct IClientModelRenderable_vtbl;
	struct  IClientModelRenderable
	{
		IClientModelRenderable_vtbl* vftabl;
	};

	struct IClientModelRenderable_vtbl
	{
		bool(__thiscall* GetRenderData)(IClientModelRenderable*, void*, ModelDataCategory_t);
	};

	struct __declspec(align(4)) C_BaseEntity /*: IClientEntity, IClientModelRenderable*/
	{
		char pad_vtbls[0x14];
		const char* m_iClassname;
		void* m_hScriptInstance; // HSCRIPT__
		const char* m_iszScriptId;
		char pad_varmap[0x20];//VarMapping_t m_VarMap;
		char pad_think[0x10]; //__int128 m_pfnThink;
		char pad_touch[0x10]; //__int128 m_pfnTouch;
		int index;
		unsigned __int16 m_EntClientFlags;
		const struct model_t* model;
		void* m_clrRender; //CNetworkColor32Base<color32_s, C_BaseEntity::NetworkVar_m_clrRender> m_clrRender;
		int m_cellbits;
		int m_cellwidth;
		int m_cellX;
		int m_cellY;
		int m_cellZ;
		Vector m_vecCellOrigin;
		Vector m_vecAbsVelocity;
		Vector m_vecAbsOrigin;
		Vector m_vecOrigin;
		QAngle m_vecAngVelocity;
		QAngle m_angAbsRotation;
		QAngle m_angRotation;
		float m_flGravity;
		float m_flProxyRandomValue;
		int m_iEFlags;
		unsigned __int8 m_nWaterType;
		bool m_bDormant;
		bool m_bCanUseBrushModelFastPath;
		int m_fEffects;
		int m_iTeamNum;
		int m_nNextThinkTick;
		int m_iHealth;
		int m_fFlags;
		Vector m_vecViewOffset;
		Vector m_vecVelocity;
		Vector m_vecBaseVelocity;
		QAngle m_angNetworkAngles;
		Vector m_vecNetworkOrigin;
		float m_flFriction;
		void* m_hNetworkMoveParent; // CHandle<C_BaseEntity>
		void* m_hOwnerEntity;
		void* m_hGroundEntity;
		char m_iName[260];
		char m_iSignifierName[260];
		__int16 m_nModelIndex;
		unsigned __int8 m_nRenderFX;
		unsigned __int8 m_nRenderMode;
		unsigned __int8 m_MoveType;
		unsigned __int8 m_MoveCollide;
		unsigned __int8 m_nWaterLevel;
		char m_lifeState;
		float m_flAnimTime;
		float m_flOldAnimTime;
		float m_flSimulationTime;
		float m_flOldSimulationTime;
		unsigned __int8 m_nOldRenderMode;
		unsigned __int16 m_hRender;
		int m_VisibilityBits; // CBitVec<2>
		bool m_bReadyToDraw;
		bool m_bClientSideRagdoll;
		int m_nLastThinkTick;
		char m_takedamage;
		float m_flSpeed;
		int touchStamp;
		CBaseHandle m_RefEHandle;
		bool m_bEnabledInToolView;
		bool m_bToolRecording;
		unsigned int m_ToolHandle;
		int m_nLastRecordedFrame;
		bool m_bRecordInTools;
		void* m_pPhysicsObject; // IPhysicsObject
		bool m_bPredictionEligible;
		int m_nSimulationTick;
		char pad_thinkFunc[0x14]; //CUtlVector<thinkfunc_t, CUtlMemory<thinkfunc_t, int> > m_aThinkFunctions;
		int m_iCurrentThinkContext;
		int m_spawnflags;
		int m_iObjectCapsCache;
		bool m_bDormantPredictable;
		int m_nIncomingPacketEntityBecameDormant;
		float m_flSpawnTime;
		float m_flLastMessageTime;
		unsigned __int16 m_ModelInstance;
		unsigned __int16 m_ShadowHandle;
		int m_ShadowBits; // CBitVec<2>
		struct CClientThinkHandlePtr* m_hThink;
		unsigned __int8 m_iParentAttachment;
		unsigned __int8 m_iOldParentAttachment;
		bool m_bPredictable;
		bool m_bRenderWithViewModels;
		bool m_bDisableCachedRenderBounds;
		bool m_bDisableSimulationFix;
		float m_fadeMinDist;
		float m_fadeMaxDist;
		float m_flFadeScale;
		int m_nSplitUserPlayerPredictionSlot;
		void* m_pMoveParent; // CHandle<C_BaseEntity>
		void* m_pMoveChild;
		void* m_pMovePeer;
		void* m_pMovePrevPeer;
		void* m_hOldMoveParent;
		const char* m_ModelName;
		char pad_mcoll[0x5C]; //C_BaseEntity::NetworkVar_m_Collision m_Collision;
		char pad_mpartc[0x20]; //C_BaseEntity::NetworkVar_m_Particles m_Particles;
		void* m_pClientAlphaProperty; // CClientAlphaProperty
		float m_flElasticity;
		float m_flShadowCastDistance;
		void* m_ShadowDirUseOtherEntity; // CHandle<C_BaseEntity>
		float m_flGroundChangeTime;
		Vector m_vecOldOrigin;
		QAngle m_vecOldAngRotation;
		/*CDiscontinuousInterpolatedVar<Vector> m_iv_vecOrigin;
		CDiscontinuousInterpolatedVar<QAngle> m_iv_angRotation;
		matrix3x4_t m_rgflCoordinateFrame;
		int m_CollisionGroup;
		unsigned __int8* m_pIntermediateData[150];
		unsigned __int8* m_pIntermediateData_FirstPredicted[151];
		unsigned __int8* m_pOriginalData;
		int m_nIntermediateDataCount;
		int m_nIntermediateData_FirstPredictedShiftMarker;
		bool m_bEverHadPredictionErrorsForThisCommand;
		bool m_bIsPlayerSimulated;
		CNetworkVarBase<bool, C_BaseEntity::NetworkVar_m_bSimulatedEveryTick> m_bSimulatedEveryTick;
		CNetworkVarBase<bool, C_BaseEntity::NetworkVar_m_bAnimatedEveryTick> m_bAnimatedEveryTick;
		CNetworkVarBase<bool, C_BaseEntity::NetworkVar_m_bAlternateSorting> m_bAlternateSorting;
		unsigned __int8 m_nMinCPULevel;
		unsigned __int8 m_nMaxCPULevel;
		unsigned __int8 m_nMinGPULevel;
		unsigned __int8 m_nMaxGPULevel;
		unsigned __int8 m_iTextureFrameIndex;
		unsigned __int8 m_fBBoxVisFlags;
		bool m_bIsValidIKAttachment;
		int m_DataChangeEventRef;
		CHandle<C_BasePlayer> m_hPlayerSimulationOwner;
		CHandle<C_BaseEntity> m_hEffectEntity;
		int m_fDataObjectTypes;
		unsigned int m_AimEntsListHandle;
		int m_nCreationTick;
		float m_fRenderingClipPlane[4];
		bool m_bEnableRenderingClipPlane;
		unsigned __int16 m_ListEntry[5];
		CThreadFastMutex m_CalcAbsolutePositionMutex;
		CThreadFastMutex m_CalcAbsoluteVelocityMutex;
		bool m_bIsBlurred;*/
	};
	STATIC_ASSERT_OFFSET(C_BaseEntity, m_vecAbsOrigin, 0x9C);


	struct MeshBoneRemap_t
	{
		int m_nActualBoneIndex;
		int m_nSrcBoneIndex;
	};

	struct MeshInstanceData_t
	{
		int m_nIndexOffset;
		int m_nIndexCount;
		int m_nBoneCount;
		MeshBoneRemap_t* m_pBoneRemap;
		matrix3x4_t* m_pPoseToWorld;
		const ITexture* m_pEnvCubemap;
		MaterialLightingState_t* m_pLightingState;
		MaterialPrimitiveType_t m_nPrimType;
		const IVertexBuffer* m_pVertexBuffer;
		int m_nVertexOffsetInBytes;
		const IIndexBuffer* m_pIndexBuffer;
		const IVertexBuffer* m_pColorBuffer;
		int m_nColorVertexOffsetInBytes;
		ShaderStencilState_t* m_pStencilState;
		Vector4D m_DiffuseModulation;
		int m_nLightmapPageId;
	};

	struct BufferedState_t
	{
		D3DXMATRIX m_Transform[3];
		_D3DVIEWPORT9 m_Viewport;
		int m_BoundTexture[16];
		void* m_VertexShader;
		void* m_PixelShader;
	};

	struct InstanceInfo_t
	{
		__int8 m_bAmbientCubeCompiled : 1;
		__int8 m_bPixelShaderLocalLightsCompiled : 1;
		__int8 m_bVertexShaderLocalLightsCompiled : 1;
		__int8 m_bSetSkinConstants : 1;
		__int8 m_bSetLightVertexShaderConstants : 1;
	};

	struct CompiledLightingState_t
	{
		Vector4D m_AmbientLightCube[6];
		int m_nLocalLightCount;
		Vector4D m_PixelShaderLocalLights[6];
		Vector4D m_VertexShaderLocalLights[20];
		int m_VertexShaderLocalLightLoopControl[4];
		int m_VertexShaderLocalLightEnable[4];
	};

	struct __declspec(align(4)) LightState_t
	{
		int m_nNumLights;
		bool m_bAmbientLight;
		bool m_bStaticLight;
	};

	enum MaterialHeightClipMode_t : __int32
	{
		MATERIAL_HEIGHTCLIPMODE_DISABLE = 0x0,
		MATERIAL_HEIGHTCLIPMODE_RENDER_ABOVE_HEIGHT = 0x1,
		MATERIAL_HEIGHTCLIPMODE_RENDER_BELOW_HEIGHT = 0x2,
	};

	enum TextureBindFlags_t : __int32
	{
		TEXTURE_BINDFLAGS_SRGBREAD = 0x80000000,
		TEXTURE_BINDFLAGS_SHADOWDEPTH = 0x40000000,
		TEXTURE_BINDFLAGS_NONE = 0x0,
	};

	struct __declspec(align(4)) SamplerState_t
	{
		int m_BoundTexture;
		_D3DTEXTUREADDRESS m_UTexWrap;
		_D3DTEXTUREADDRESS m_VTexWrap;
		_D3DTEXTUREADDRESS m_WTexWrap;
		_D3DTEXTUREFILTERTYPE m_MagFilter;
		_D3DTEXTUREFILTERTYPE m_MinFilter;
		_D3DTEXTUREFILTERTYPE m_MipFilter;
		TextureBindFlags_t m_nTextureBindFlags;
		int m_nAnisotropicLevel;
		unsigned int m_bShadowFilterEnable;
		bool m_TextureEnable;
	};
	STATIC_ASSERT_SIZE(SamplerState_t, 44);

	struct VertexTextureState_t
	{
		int m_BoundVertexTexture;
		_D3DTEXTUREADDRESS m_UTexWrap;
		_D3DTEXTUREADDRESS m_VTexWrap;
		_D3DTEXTUREFILTERTYPE m_MagFilter;
		_D3DTEXTUREFILTERTYPE m_MinFilter;
		_D3DTEXTUREFILTERTYPE m_MipFilter;
	};

	enum TessellationMode_t : __int32
	{
		TESSELLATION_MODE_DISABLED = 0x0,
		TESSELLATION_MODE_ACC_PATCHES_EXTRA = 0x1,
		TESSELLATION_MODE_ACC_PATCHES_REG = 0x2,
	};

	struct D3DRENDERSTATETYPE_array
	{
		uint32_t unused0;
		uint32_t TEXTUREHANDLE;
		uint32_t ANTIALIAS;
		uint32_t TEXTUREADDRESS;
		uint32_t TEXTUREPERSPECTIVE;
		uint32_t WRAPU;
		uint32_t WRAPV;
		D3DZBUFFERTYPE xD3DRS_ZENABLE; //7;    /* D3DZBUFFERTYPE (or TRUE/FALSE for legacy) */
		D3DFILLMODE xD3DRS_FILLMODE; //8;    /* D3DFILLMODE */
		D3DSHADEMODE xD3DRS_SHADEMODE; //9;    /* D3DSHADEMODE */
		uint32_t LINEPATTERN;
		uint32_t MONOENABLE;
		uint32_t ROP2;
		uint32_t PLANEMASK;
		uint32_t xD3DRS_ZWRITEENABLE; //14;   /* TRUE to enable z writes */
		uint32_t xD3DRS_ALPHATESTENABLE; //15;   /* TRUE to enable alpha tests */
		uint32_t xD3DRS_LASTPIXEL; //16;   /* TRUE for last-pixel on lines */
		uint32_t TEXTUREMAG;
		uint32_t TEXTUREMIN;
		D3DBLEND xD3DRS_SRCBLEND; //19;   /* D3DBLEND */
		D3DBLEND xD3DRS_DESTBLEND; //20;   /* D3DBLEND */
		uint32_t TEXTUREMAPBLEND;
		D3DCULL xD3DRS_CULLMODE; //22;   /* D3DCULL */
		D3DCMPFUNC xD3DRS_ZFUNC; //23;   /* D3DCMPFUNC */
		uint32_t xD3DRS_ALPHAREF; //24;   /* D3DFIXED */
		D3DCMPFUNC xD3DRS_ALPHAFUNC; //25;   /* D3DCMPFUNC */
		uint32_t xD3DRS_DITHERENABLE; //26;   /* TRUE to enable dithering */
		uint32_t xD3DRS_ALPHABLENDENABLE; //27;   /* TRUE to enable alpha blending */
		uint32_t xD3DRS_FOGENABLE; //28;   /* TRUE to enable fog blending */
		uint32_t xD3DRS_SPECULARENABLE; //29;   /* TRUE to enable specular */
		uint32_t ZVISIBLE;
		uint32_t SUBPIXEL;
		uint32_t SUBPIXELX;
		uint32_t STIPPLEDALPHA;
		D3DCOLOR xD3DRS_FOGCOLOR; //34;   /* D3DCOLOR */
		D3DFOGMODE xD3DRS_FOGTABLEMODE; //35;   /* D3DFOGMODE */
		uint32_t xD3DRS_FOGSTART; //36;   /* Fog start (for both vertex and pixel fog) */
		uint32_t xD3DRS_FOGEND; //37;   /* Fog end      */
		uint32_t xD3DRS_FOGDENSITY; //38;   /* Fog density  */
		uint32_t STIPPLEENABLE;
		uint32_t EDGEANTIALIAS;
		uint32_t COLORKEYENABLE;
		uint32_t unused42;
		uint32_t BORDERCOLOR;
		uint32_t TEXTUREADDRESSU;
		uint32_t TEXTUREADDRESSV;
		uint32_t MIPMAPLODBIAS;
		uint32_t ZBIAS;
		uint32_t xD3DRS_RANGEFOGENABLE; //48;   /* Enables range-based fog */
		uint32_t ANISOTROPY;
		uint32_t FLUSHBATCH;
		uint32_t TRANSLUCENTSORTINDEPENDENT;
		uint32_t xD3DRS_STENCILENABLE; //52;   /* BOOL enable/disable stenciling */
		D3DSTENCILOP xD3DRS_STENCILFAIL; //53;   /* D3DSTENCILOP to do if stencil test fails */
		D3DSTENCILOP xD3DRS_STENCILZFAIL; //54;   /* D3DSTENCILOP to do if stencil test passes and Z test fails */
		D3DSTENCILOP xD3DRS_STENCILPASS; //55;   /* D3DSTENCILOP to do if both stencil and Z tests pass */
		D3DCMPFUNC xD3DRS_STENCILFUNC; //56;   /* D3DCMPFUNC fn.  Stencil Test passes if ((ref & mask) stencilfn (stencil & mask)) is true */
		uint32_t xD3DRS_STENCILREF; //57;   /* Reference value used in stencil test */
		uint32_t xD3DRS_STENCILMASK; //58;   /* Mask value used in stencil test */
		uint32_t xD3DRS_STENCILWRITEMASK; //59;   /* Write mask applied to values written to stencil buffer */
		uint32_t xD3DRS_TEXTUREFACTOR; //60;   /* D3DCOLOR used for multi-texture blend */
		uint32_t pad[67];
		uint32_t xD3DRS_WRAP0; //128;  /* wrap for 1st texture coord. set */
		uint32_t xD3DRS_WRAP1; //129;  /* wrap for 2nd texture coord. set */
		uint32_t xD3DRS_WRAP2; //130;  /* wrap for 3rd texture coord. set */
		uint32_t xD3DRS_WRAP3; //131;  /* wrap for 4th texture coord. set */
		uint32_t xD3DRS_WRAP4; //132;  /* wrap for 5th texture coord. set */
		uint32_t xD3DRS_WRAP5; //133;  /* wrap for 6th texture coord. set */
		uint32_t xD3DRS_WRAP6; //134;  /* wrap for 7th texture coord. set */
		uint32_t xD3DRS_WRAP7; //135;  /* wrap for 8th texture coord. set */
		uint32_t xD3DRS_CLIPPING; //136;
		uint32_t xD3DRS_LIGHTING; //137;
		uint32_t EXTENTS;
		uint32_t xD3DRS_AMBIENT; //139;
		uint32_t xD3DRS_FOGVERTEXMODE; //140;
		uint32_t xD3DRS_COLORVERTEX; //141;
		uint32_t xD3DRS_LOCALVIEWER; //142;
		uint32_t xD3DRS_NORMALIZENORMALS; //143;
		uint32_t COLORKEYBLENDENABLE;
		uint32_t xD3DRS_DIFFUSEMATERIALSOURCE; //145;
		uint32_t xD3DRS_SPECULARMATERIALSOURCE; //146;
		uint32_t xD3DRS_AMBIENTMATERIALSOURCE; //147;
		uint32_t xD3DRS_EMISSIVEMATERIALSOURCE; //148;
		uint32_t unused149;
		uint32_t unused150;
		uint32_t xD3DRS_VERTEXBLEND; //151;
		uint32_t xD3DRS_CLIPPLANEENABLE; //152;
		uint32_t unused153;
		uint32_t xD3DRS_POINTSIZE; //154;   /* float point size */
		uint32_t xD3DRS_POINTSIZE_MIN; //155;   /* float point size min threshold */
		uint32_t xD3DRS_POINTSPRITEENABLE; //156;   /* BOOL point texture coord control */
		uint32_t xD3DRS_POINTSCALEENABLE; //157;   /* BOOL point size scale enable */
		uint32_t xD3DRS_POINTSCALE_A; //158;   /* float point attenuation A value */
		uint32_t xD3DRS_POINTSCALE_B; //159;   /* float point attenuation B value */
		uint32_t xD3DRS_POINTSCALE_C; //160;   /* float point attenuation C value */
		uint32_t xD3DRS_MULTISAMPLEANTIALIAS; //161;  // BOOL - set to do FSAA with multisample buffer
		uint32_t xD3DRS_MULTISAMPLEMASK; //162;  // DWORD - per-sample enable/disable
		uint32_t xD3DRS_PATCHEDGESTYLE; //163;  // Sets whether patch edges will use float style tessellation
		uint32_t unused164;
		uint32_t xD3DRS_DEBUGMONITORTOKEN; //165;  // DEBUG ONLY - token to debug monitor
		uint32_t xD3DRS_POINTSIZE_MAX; //166;   /* float point size max threshold */
		uint32_t xD3DRS_INDEXEDVERTEXBLENDENABLE; //167;
		uint32_t xD3DRS_COLORWRITEENABLE; //168;  // per-channel write enable
		uint32_t unused169;
		uint32_t xD3DRS_TWEENFACTOR; //170;   // float tween factor
		D3DBLENDOP xD3DRS_BLENDOP; //171;   // D3DBLENDOP setting
		uint32_t xD3DRS_POSITIONDEGREE; //172;   // NPatch position interpolation degree. D3DDEGREE_LINEAR or D3DDEGREE_CUBIC (default)
		uint32_t xD3DRS_NORMALDEGREE; //173;   // NPatch normal interpolation degree. D3DDEGREE_LINEAR (default) or D3DDEGREE_QUADRATIC
		uint32_t xD3DRS_SCISSORTESTENABLE; //174;
		uint32_t xD3DRS_SLOPESCALEDEPTHBIAS; //175;
		uint32_t xD3DRS_ANTIALIASEDLINEENABLE; //176;
		uint32_t unused177;
		uint32_t xD3DRS_MINTESSELLATIONLEVEL; //178;
		uint32_t xD3DRS_MAXTESSELLATIONLEVEL; //179;
		uint32_t xD3DRS_ADAPTIVETESS_X; //180;
		uint32_t xD3DRS_ADAPTIVETESS_Y; //181;
		uint32_t xD3DRS_ADAPTIVETESS_Z; //182;
		uint32_t xD3DRS_ADAPTIVETESS_W; //183;
		uint32_t xD3DRS_ENABLEADAPTIVETESSELLATION; //184;
		uint32_t xD3DRS_TWOSIDEDSTENCILMODE; //185;   /* BOOL enable/disable 2 sided stenciling */
		D3DSTENCILOP xD3DRS_CCW_STENCILFAIL; //186;   /* D3DSTENCILOP to do if ccw stencil test fails */
		D3DSTENCILOP xD3DRS_CCW_STENCILZFAIL; //187;   /* D3DSTENCILOP to do if ccw stencil test passes and Z test fails */
		D3DSTENCILOP xD3DRS_CCW_STENCILPASS; //188;   /* D3DSTENCILOP to do if both ccw stencil and Z tests pass */
		D3DCMPFUNC xD3DRS_CCW_STENCILFUNC; //189;   /* D3DCMPFUNC fn.  ccw Stencil Test passes if ((ref & mask) stencilfn (stencil & mask)) is true */
		uint32_t xD3DRS_COLORWRITEENABLE1; //190;   /* Additional ColorWriteEnables for the devices that support D3DPMISCCAPS_INDEPENDENTWRITEMASKS */
		uint32_t xD3DRS_COLORWRITEENABLE2; //191;   /* Additional ColorWriteEnables for the devices that support D3DPMISCCAPS_INDEPENDENTWRITEMASKS */
		uint32_t xD3DRS_COLORWRITEENABLE3; //192;   /* Additional ColorWriteEnables for the devices that support D3DPMISCCAPS_INDEPENDENTWRITEMASKS */
		D3DCOLOR xD3DRS_BLENDFACTOR; //193;   /* D3DCOLOR used for a constant blend factor during alpha blending for devices that support D3DPBLENDCAPS_BLENDFACTOR */
		uint32_t xD3DRS_SRGBWRITEENABLE; //194;   /* Enable rendertarget writes to be DE-linearized to SRGB (for formats that expose D3DUSAGE_QUERY_SRGBWRITE) */
		uint32_t xD3DRS_DEPTHBIAS; //195;
		uint32_t unused196;
		uint32_t unused197;
		uint32_t xD3DRS_WRAP8; //198;   /* Additional wrap states for vs_3_0+ attributes with D3DDECLUSAGE_TEXCOORD */
		uint32_t xD3DRS_WRAP9; //199;
		uint32_t xD3DRS_WRAP10; //200;
		uint32_t xD3DRS_WRAP11; //201;
		uint32_t xD3DRS_WRAP12; //202;
		uint32_t xD3DRS_WRAP13; //203;
		uint32_t xD3DRS_WRAP14; //204;
		uint32_t xD3DRS_WRAP15; //205;
		uint32_t xD3DRS_SEPARATEALPHABLENDENABLE; //206;  /* TRUE to enable a separate blending function for the alpha channel */
		D3DBLEND xD3DRS_SRCBLENDALPHA; //207;  /* SRC blend factor for the alpha channel when D3DRS_SEPARATEDESTALPHAENABLE is TRUE */
		D3DBLEND xD3DRS_DESTBLENDALPHA; //208;  /* DST blend factor for the alpha channel when D3DRS_SEPARATEDESTALPHAENABLE is TRUE */
		D3DBLENDOP xD3DRS_BLENDOPALPHA; //209;  /* Blending operation for the alpha channel when D3DRS_SEPARATEDESTALPHAENABLE is TRUE */
	}; STATIC_ASSERT_SIZE(D3DRENDERSTATETYPE_array, 840);

	struct __declspec(align(8)) DynamicState_t
	{
		_D3DVIEWPORT9 m_Viewport;
		D3DXMATRIX m_Transform[11];
		unsigned __int8 m_TransformType[11];
		unsigned __int8 m_TransformChanged[11];
		InstanceInfo_t m_InstanceInfo;
		CompiledLightingState_t m_CompiledLightingState;
		MaterialLightingState_t m_LightingState;
		LightState_t m_ShaderLightState;
		bool m_bLightStateComputed;
		int m_nLocalEnvCubemapSamplers;
		int m_nLightmapSamplers;
		int m_nPaintmapSamplers;
		_D3DSHADEMODE m_ShadeMode;
		unsigned int m_ClearColor;
		unsigned int m_FogColor;
		Vector4D m_vecPixelFogColor;
		Vector4D m_vecPixelFogColorLinear;
		bool m_bFogGammaCorrectionDisabled;
		bool m_FogEnable;
		_D3DFOGMODE m_FogMode;
		float m_FogStart;
		float m_FogEnd;
		float m_FogZ;
		float m_FogMaxDensity;
		float m_HeightClipZ;
		MaterialHeightClipMode_t m_HeightClipMode;
		int m_UserClipPlaneEnabled;
		int m_UserClipPlaneChanged;
		D3DXPLANE m_UserClipPlaneWorld[6];
		D3DXPLANE m_UserClipPlaneProj[6];
		bool m_UserClipLastUpdatedUsingFixedFunction;
		bool m_FastClipEnabled;
		bool m_bFastClipPlaneChanged;
		D3DXPLANE m_FastClipPlane;
		bool m_bUserClipTransformOverride;
		D3DXMATRIX m_UserClipTransform;
		_D3DCULL m_DesiredCullMode;
		_D3DCULL m_CullMode;
		bool m_bCullEnabled;
		D3DVERTEXBLENDFLAGS m_VertexBlend; // NEW
		int m_NumBones;
		Vector4D* m_pVectorVertexShaderConstant;
		int* m_pBooleanVertexShaderConstant;
		IntVector4D* m_pIntegerVertexShaderConstant;
		Vector4D* m_pVectorPixelShaderConstant;
		int* m_pBooleanPixelShaderConstant;
		IntVector4D* m_pIntegerPixelShaderConstant;
		SamplerState_t m_SamplerState[16];
		VertexTextureState_t m_VertexTextureState[4];
		//unsigned int m_RenderState[210];
		D3DRENDERSTATETYPE_array m_RenderState;
		tagRECT m_ScissorRect;
		IDirect3DVertexDeclaration9* m_pVertexDecl;
		unsigned __int64 m_DeclVertexFormat;
		bool m_bDeclHasColorMesh;
		bool m_bDeclUsingFlex;
		bool m_bDeclUsingMorph;
		bool m_bDeclUsingPreTessPatch;
		bool m_bSRGBWritesEnabled;
		bool m_bHWMorphingEnabled;
		TessellationMode_t m_TessellationMode;
	};
	STATIC_ASSERT_OFFSET(DynamicState_t, m_SamplerState, 2172); // <6240 (offset from shaderapi 0x0> - <40 (m_TextureEnable offset)> - <4028 (pad in IShaderAPIDX8)>
	STATIC_ASSERT_OFFSET(DynamicState_t, m_RenderState, 2972);

	struct IShaderAPIDX8_vtbl
	{
		char pad[0x3D8];
		IDirect3DBaseTexture9* (__fastcall* GetD3DTexture)(void* shaderapi_ptr, void* ecx, int handle);
		void* pad97;
		void* pad98;
		void* pad99;
		void(__fastcall* GetBufferedState)(void* shaderapi_ptr, void* ecx, BufferedState_t*);
		_D3DCULL(__fastcall* GetCullMode)(void* shaderapi_ptr, void* ecx);
		void* ComputeFillRate;
		void* IsInSelectionMode;
		void* RegisterSelectionHit;
		IMaterialInternal* (__fastcall* GetBoundMaterial)(void* shaderapi_ptr, void* ecx);
	};
	STATIC_ASSERT_OFFSET(IShaderAPIDX8_vtbl, GetBufferedState, 0x3E8);

	struct IShaderAPIDX8
	{
		IShaderAPIDX8_vtbl* vtbl;
	};
	
	struct CShaderAPIDX8_vtbl
	{
		void* null;
	};

	struct CShaderAPIDx8
	{
		CShaderAPIDX8_vtbl* vtbl;
		char pad0[4028];
		DynamicState_t m_DynamicState;
		DynamicState_t m_DesiredState;
		unsigned __int8 m_pCommitFlags[2][1];
		char m_CommitFuncs_0[0x14];
		char m_CommitFuncs_1[0x14];
		CMeshBase* m_pRenderMesh;
		int m_nRenderInstanceCount;
		const MeshInstanceData_t* m_pRenderInstances;
		CompiledLightingState_t* m_pRenderCompiledState;
		InstanceInfo_t* m_pRenderInstanceInfo;
		ShaderStencilState_t m_RenderInitialStencilState;
		bool m_bRenderHasSetStencil;
		int m_nDynamicVBSize;
		IMaterialInternal* m_pMaterial;
		float m_fShadowSlopeScaleDepthBias;
		float m_fShadowDepthBias;
		__int8 m_bReadPixelsEnabled : 1;
		__int8 m_bFlipCulling : 1;
		__int8 m_bSinglePassFlashlightMode : 1;
		__int8 m_UsingTextureRenderTarget : 1;
		int m_ViewportMaxWidth;
		int m_ViewportMaxHeight;
		int m_hCachedRenderTarget;
		bool m_bUsingSRGBRenderTarget;
		int m_CurrentFrame;
		int m_ModifyTextureHandle;
		char m_ModifyTextureLockedLevel;
		unsigned __int8 m_ModifyTextureLockedFace;
	}; STATIC_ASSERT_OFFSET(CShaderAPIDx8, m_DynamicState, 0xFC0);

	struct CMaterialReference
	{
		IMaterial* m_pMaterial;
	};

	struct fourplanes_t
	{
		__m128 nX;
		__m128 nY;
		__m128 nZ;
		__m128 dist;
		__m128 xSign;
		__m128 ySign;
		__m128 zSign;
		__m128 nXAbs;
		__m128 nYAbs;
		__m128 nZAbs;
	};

	struct Frustum_t
	{
		fourplanes_t planes[2];
	}; STATIC_ASSERT_SIZE(Frustum_t, 0x140);


	struct PropPortalRenderingMaterials_t
	{
		CMaterialReference m_PortalMaterials[2];
		CMaterialReference m_PortalRenderFixMaterials[2];
		CMaterialReference m_PortalDepthDoubler;
		CMaterialReference m_PortalStaticOverlay[2];
		CMaterialReference m_PortalStaticOverlay_Tinted;
		CMaterialReference m_PortalStaticGhostedOverlay[2];
		CMaterialReference m_PortalStaticGhostedOverlay_Tinted;
		CMaterialReference m_Portal_Stencil_Hole;
		CMaterialReference m_Portal_Refract;
		unsigned int m_nDepthDoubleViewMatrixVarCache;
		unsigned int m_nStaticOverlayTintedColorGradientLightVarCache;
		Vector m_coopPlayerPortalColors[2][2];
		Vector m_singlePlayerPortalColors[2];
	};

	struct CAutoInitBasicPropPortalDrawingMaterials
	{
		char pad[0xC];
		PropPortalRenderingMaterials_t m_Materials;
	};

	enum SkyboxVisibility_t : __int32
	{
		SKYBOX_NOT_VISIBLE = 0x0,
		SKYBOX_3DSKYBOX_VISIBLE = 0x1,
		SKYBOX_2DSKYBOX_VISIBLE = 0x2,
	};

	struct FlatBasicPortal_InternalData_t
	{
		VPlane m_BoundingPlanes[14];
		VisOverrideData_t m_VisData;
		int m_iViewLeaf;
		VMatrix m_DepthDoublerTextureView[2];
		bool m_bUsableDepthDoublerConfiguration;
		SkyboxVisibility_t m_nSkyboxVisibleFromCorners;
		Vector m_ptForwardOrigin;
		Vector m_ptCorners[4];
		float m_fPlaneDist;
	}; STATIC_ASSERT_SIZE(FlatBasicPortal_InternalData_t, 0x20C);

	// same struct as C_Prop_Portal for ease of use
	// m_pLinkedPortal points to an address that is 0xC infront of the C_Prop_Portal object?
	struct CPortalRenderable_FlatBasic
	{
		char pad1[0xE04];
		FlatBasicPortal_InternalData_t m_InternallyMaintainedData;
		CPortalRenderable_FlatBasic* m_pLinkedPortal; //0x1010 
		Vector m_ptOrigin; //0x1014 
		Vector m_vForward; //0x1020 
		Vector m_vUp; //0x102C 
		Vector m_vRight; //0x1038 
		Vector m_qAbsAngle; //0x1044 
		unsigned char m_bIsPortal2; //0x1050 
		char pad_0x1051[0x3]; //0x1051
		float m_fHalfWidth; //0x1054 
		float m_fHalfHeight; //0x1058 
		char pad2[9396]; //0x11A5888 
		float m_fStaticAmount; //0x3510 
		float m_fSecondaryStaticAmount; //0x3514 
		float m_fOpenAmount; //0x3518 
	}; //Size=0x351C
	STATIC_ASSERT_OFFSET(CPortalRenderable_FlatBasic, m_pLinkedPortal, 0x1010);

	struct C_Prop_Portal_vtbl;
	struct C_Prop_Portal
	{
		//C_Prop_Portal_vtbl* vtbl;
		//char pad1[4096];
		char pad01[0xE0];
		int team_num;
		char pad02[0x2EC];
		int portal_index;
		char pad03[0xA24];
		FlatBasicPortal_InternalData_t m_InternallyMaintainedData;
		CPortalRenderable_FlatBasic* m_pLinkedPortal; //0x1004 
		Vector m_ptOrigin; //0x1008 
		Vector m_vForward; //0x1014 
		Vector m_vUp; //0x1020 
		Vector m_vRight; //0x102C 
		Vector m_qAbsAngle; //0x1038 
		unsigned char m_bIsPortal2; //0x1044 
		char pad_0x1045[0x3]; //0x1045
		float m_fHalfWidth; //0x1048 
		float m_fHalfHeight; //0x104C 
		char pad2[9332]; //0xFC5888 
		char pad3[0x20]; //0x34C4
		unsigned char m_bActivated; //0x34E4 
		unsigned char m_bOldActivatedState; //0x34E5 
		char pad_0x34E6[0x2]; //0x34E6
		float m_fNetworkHalfWidth; //0x34E8 
		float m_fNetworkHalfHeight; //0x34EC 
		unsigned char m_bIsMobile; //0x34F0 -- uh?
		char pad_0x34F1[0x13]; //0x34F1
		float m_fStaticAmount; //0x3504 
		float m_fSecondaryStaticAmount; //0x3508 
		float m_fOpenAmount; //0x350C
		C_BaseEntity* m_hFiredByPlayer;
	};
	
	STATIC_ASSERT_OFFSET(C_Prop_Portal, portal_index, 0x3D0);
	STATIC_ASSERT_OFFSET(C_Prop_Portal, m_pLinkedPortal, 0x1004);
	STATIC_ASSERT_OFFSET(C_Prop_Portal, m_fOpenAmount, 0x350C);

	struct C_Team : C_BaseEntity
	{
		char pad[0xC]; // CUtlVector<int, CUtlMemory<int, int> > m_aPlayers;
		int m_aPlayers_Size;
		int* m_aPlayers;
		char m_szTeamname[32];
		int m_iScore;
		int m_iRoundsWon;
		int m_iDeaths;
		int m_iPing;
		int m_iPacketloss;
		int m_iTeamNum;
	};

	struct C_Prop_Portal_vtbl
	{
		char pad_base[67 * 4];
		char pad[85 * 4];
		C_Team* (__thiscall* GetTeam)(C_Prop_Portal*); // 85
		int(__thiscall* GetTeamNumber)(C_Prop_Portal*);
		void(__thiscall* ChangeTeam)(C_Prop_Portal*, int);
		int(__thiscall* GetRenderTeamNumber)(C_Prop_Portal*);
		bool(__thiscall* InSameTeam)(C_Prop_Portal*, C_Prop_Portal*);
		bool(__thiscall* InLocalTeam)(C_Prop_Portal*);
		bool(__thiscall* IsValidIDTarget)(C_Prop_Portal*);
		char* (__thiscall* GetIDString)(C_Prop_Portal*);
	};

	enum RemixInstanceCategories : uint32_t
	{
		WorldUI					= 1 << 0,
		WorldMatte				= 1 << 1,
		Sky						= 1 << 2,
		Ignore					= 1 << 3,
		IgnoreLights			= 1 << 4,
		IgnoreAntiCulling		= 1 << 5,
		IgnoreMotionBlur		= 1 << 6,
		IgnoreOpacityMicromap	= 1 << 7,
		IgnoreAlphaChannel		= 1 << 8,
		Hidden					= 1 << 9,
		Particle				= 1 << 10,
		Beam					= 1 << 11,
		DecalStatic				= 1 << 12,
		DecalDynamic			= 1 << 13,
		DecalSingleOffset		= 1 << 14,
		DecalNoOffset			= 1 << 15,
		AlphaBlendToCutout		= 1 << 16,
		Terrain					= 1 << 17,
		AnimatedWater			= 1 << 18,
		ThirdPersonPlayerModel	= 1 << 19,
		ThirdPersonPlayerBody	= 1 << 20,
		IgnoreBakedLighting		= 1 << 21,
	};

	struct msurface2_t
	{
		unsigned int flags;
		cplane_t* plane;
		int firstvertindex;
		unsigned __int16 decals;
		unsigned __int16 m_ShadowDecals;
		unsigned __int16 m_nFirstOverlayFragment;
		__int16 materialSortID;
		unsigned __int16 vertBufferIndex;
		unsigned __int16 m_bDynamicShadowsEnabled : 1;
		unsigned __int16 texinfo : 15;
		void* pDispInfo; // IDispInfo
		int visframe;
	};

	struct __declspec(align(4)) CIndexBuilder : IndexDesc_t
	{
		IIndexBuffer* m_pIndexBuffer;
		int m_nMaxIndexCount;
		int m_nIndexCount;
		int m_nIndexOffset;
		int m_nCurrentIndex;
		int m_nTotalIndexCount;
		unsigned int m_nBufferOffset;
		unsigned int m_nBufferFirstIndex;
		bool m_bModify;
	};

	struct __declspec(align(4)) CVertexBuilder : VertexDesc_t
	{
		IVertexBuffer* m_pVertexBuffer;
		bool m_bModify;
		int m_nMaxVertexCount;
		int m_nVertexCount;
		int m_nCurrentVertex;
		float* m_pCurrPosition;
		float* m_pCurrNormal;
		unsigned __int8* m_pCurrColor;
		float* m_pCurrTexCoord[8];
		int m_nTotalVertexCount;
		unsigned int m_nBufferOffset;
		unsigned int m_nBufferFirstVertex;
		__int8 m_bWrittenNormal : 1;
		__int8 m_bWrittenUserData : 1;
	};

	struct CMeshBuilder : MeshDesc_t
	{
		IMesh* m_pMesh;
		MaterialPrimitiveType_t m_Type;
		bool m_bGenerateIndices;
		CIndexBuilder m_IndexBuilder;
		CVertexBuilder m_VertexBuilder;
	};

	
	struct ConCommandBase_vtbl;
	struct ConCommandBase
	{
		ConCommandBase_vtbl* vftable;
		ConCommandBase* m_pNext;
		bool m_bRegistered;
		const char* m_pszName;
		const char* m_pszHelpString;
		int m_nFlags;
	};

	struct ConCommandBase_vtbl
	{
		void(__thiscall * ConCommandBase_Destructor)(ConCommandBase*);
		bool(__thiscall* IsCommand)(ConCommandBase*);
		bool(__thiscall* IsFlagSet)(ConCommandBase*, int);
		void(__thiscall* AddFlags)(ConCommandBase*, int);
		void(__thiscall* RemoveFlags)(ConCommandBase*, int);
		int(__thiscall* GetFlags)(ConCommandBase*);
		const char* (__thiscall* GetName)(ConCommandBase*);
		const char* (__thiscall* GetHelpText)(ConCommandBase*);
		bool(__thiscall* IsRegistered)(ConCommandBase*);
		int(__thiscall* GetDLLIdentifier)(ConCommandBase*);
		void(__thiscall* Create)(ConCommandBase*, const char*, const char*, int);
		void(__thiscall* Init)(ConCommandBase*);
	};

	const struct CCommand
	{
		int m_nArgc;
		int m_nArgv0Size;
		char m_pArgSBuffer[512];
		char m_pArgvBuffer[512];
		const char* m_ppArgv[64];
	};

	struct ICommandCallback_vtbl;
	struct ICommandCallback
	{
		ICommandCallback_vtbl* vftable;
	};

	struct ICommandCallback_vtbl
	{
		void(__thiscall* CommandCallback)(ICommandCallback*, const CCommand*);
	};

	union $DBE510182F331AFA63E289BCE7D04441
	{
		void(__cdecl* m_fnCommandCallbackV1)();
		void(__cdecl* m_fnCommandCallback)(const CCommand*);
		ICommandCallback* m_pCommandCallback;
	};

	struct ICommandCompletionCallback_vtbl;
	struct ICommandCompletionCallback
	{
		ICommandCompletionCallback_vtbl* vftable;
	};

	struct ICommandCompletionCallback_vtbl
	{
		int(__thiscall* CommandCompletionCallback)(ICommandCompletionCallback*, const char*, void* CUtlString);
	};

	union $E9983340D3446DFCFEC741F54422A481
	{
		int(__cdecl* m_fnCompletionCallback)(const char*, char(*)[64]);
		ICommandCompletionCallback* m_pCommandCompletionCallback;
	};

	struct __declspec(align(4)) ConCommand : ConCommandBase
	{
		$DBE510182F331AFA63E289BCE7D04441 u1;
		$E9983340D3446DFCFEC741F54422A481 u2;
		__int8 m_bHasCompletionCallback : 1;
		__int8 m_bUsingNewCommandCallback : 1;
		__int8 m_bUsingCommandCallbackInterface : 1;
	};

	struct CGlobalVarsBase
	{
		float realtime;
		int framecount;
		float absoluteframetime;
		float curtime;
		float frametime;
		int maxClients;
		int tickcount;
		float interval_per_tick;
		float interpolation_amount;
		int sim_ticks_this_frame;
		int network_protocol;
		void* save_data;
		bool m_bclient;
		bool m_bremoteclient;
	};

	struct ViewCustomVisibility_t
	{
		int m_nNumVisOrigins;
		Vector m_rgVisOrigins[32] = {};
		VisOverrideData_t m_VisData = {};
		int m_iForceViewLeaf = -1;
	};

	struct IConVar_vtbl;
	struct IConVar
	{
		IConVar_vtbl* vtbl;
	};

	struct IConVar_vtbl
	{
		void(__thiscall* SetValue_Color)(IConVar*, Color);
		void(__thiscall* SetValue_Int)(IConVar*, int);
		void(__thiscall* SetValue_Float)(IConVar*, float);
		void(__thiscall* SetValue_String)(IConVar*, const char*);
		const char* (__thiscall* GetName)(IConVar*);
		const char* (__thiscall* GetBaseName)(IConVar*);
		bool(__thiscall* IsFlagSet)(IConVar*, int);
		int(__thiscall* GetSplitScreenPlayerSlot)(IConVar*);
	};

	struct ConVar_CVValue_t
	{
		char* m_pszString;
		int m_StringLength;
		float m_fValue;
		int m_nValue;
	};

	const struct ConVar : ConCommandBase, IConVar
	{
		ConVar* m_pParent;
		const char* m_pszDefaultValue;
		ConVar_CVValue_t m_Value;
		bool m_bHasMin;
		float m_fMinVal;
		bool m_bHasMax;
		float m_fMaxVal;
		//m_fnChangeCallbacks;
	};

	struct CCvar_vtbl;
	struct CCvar
	{
		CCvar_vtbl* vftable;
	};

	struct CCvar_vtbl
	{
		DWORD pad[6];
		void(__thiscall* RegisterConCommand)(CCvar*, ConCommandBase*);
		void(__thiscall* UnregisterConCommand)(CCvar*, ConCommandBase*);
		void(__thiscall* UnregisterConCommands)(CCvar*, int);
		const char* (__thiscall* GetCommandLineValue)(CCvar*, const char*);
		const ConCommandBase* (__thiscall* FindCommandBase_const)(CCvar*, const char*);
		ConCommandBase* (__thiscall* FindCommandBase)(CCvar*, const char*);
		const ConVar* (__thiscall* FindVar_const)(CCvar*, const char*);
		ConVar* (__thiscall* FindVar)(CCvar*, const char*);
		const ConCommand* (__thiscall* FindCommand_const)(CCvar*, const char*);
		ConCommand* (__thiscall* FindCommand)(CCvar*, const char*);
	};

	struct CUtlString
	{
		//CUtlBinaryBlock m_Storage;
		const char* string;
		char pad[10];
	}; STATIC_ASSERT_SIZE(CUtlString, 0x10);

	struct __declspec(align(4)) CChoreoActor
	{
		char m_szName[128];
		// ...
	};

	struct CChoreoScene
	{
		char pad1[0x78];
		float m_flCurrentTime;
		float m_flStartTime;
		float m_flEndTime;
		bool m_bRecalculateSceneTimes;
		float m_flEarliestTime;
		float m_flLatestTime;
		int m_nActiveEvents;
		float m_flSoundSystemLatency;
		float m_flLastActiveTime;
		void (*m_pfnPrint)(const char*, ...);
		void* m_pIChoreoEventCallback;
		void* m_pTokenizer;
		char m_szMapname[128];
		int m_nSceneFPS;
		char pad2[0x5C];
		char m_szFileName[128];
	};
	STATIC_ASSERT_OFFSET(CChoreoScene, m_flCurrentTime, 0x78);
	STATIC_ASSERT_OFFSET(CChoreoScene, m_nSceneFPS, 0x128);
	STATIC_ASSERT_OFFSET(CChoreoScene, m_szFileName, 0x188);

	struct __declspec(align(4)) CChoreoEvent
	{
		void* baseclass;
		unsigned __int8 m_fType;
		unsigned __int8 m_ccType;
		CUtlString m_Name;
		CUtlString m_Parameters;
		CUtlString m_Parameters2;
		CUtlString m_Parameters3;
		float m_flStartTime;
		float m_flEndTime;
		float m_flGestureSequenceDuration;
		int m_nNumLoops;
		int m_nLoopsRemaining;
		char pad1[0x38];
		CUtlString m_TagName;
		CUtlString m_TagWavName;
		CChoreoActor* m_pActor;
		void* m_pChannel; // CChoreoChannel
		char pad2[0x64];
		CChoreoScene* m_pSubScene;
		void* m_pMixer; // CAudioMixer
		CChoreoScene* m_pScene;
		// ....
	};
	STATIC_ASSERT_OFFSET(CChoreoEvent, m_TagName, 0x94);
	STATIC_ASSERT_OFFSET(CChoreoEvent, m_pSubScene, 0x120);
	STATIC_ASSERT_OFFSET(CChoreoEvent, m_pScene, 0x128);

	struct FadeData_t
	{
		float m_flPixelMin;
		float m_flPixelMax;
		float m_flWidth;
		float m_flFadeDistScale;
	};

	struct CPortalRect
	{
		float left;
		float top;
		float right;
		float bottom;
	};

	
	struct CSfxTable_vtbl;
	struct CSfxTable
	{
		CSfxTable_vtbl* vftable;
		int m_namePoolIndex;
		void* pSource; // CAudioSource
		__int8 m_bUseErrorFilename : 1;
		__int8 m_bIsUISound : 1;
		__int8 m_bIsLateLoad : 1;
		__int8 m_bMixGroupsCached : 1;
		__int8 m_bIsMusic : 1;
		__int8 m_bIsCreatedByQueuedLoader : 1;
		unsigned __int8 m_mixGroupCount;
		unsigned __int8 m_mixGroupList[8];
		const char* m_pDebugName;
	};

	struct CSfxTable_vtbl
	{
		const char* (__thiscall* getname)(CSfxTable*, char*, unsigned int);
	};

	struct __declspec(align(4)) StartSoundParams_t
	{
		int userdata;
		int soundsource;
		int entchannel;
		CSfxTable* pSfx;
		Vector origin;
		Vector direction;
		float fvol;
		int soundlevel; // soundlevel_t
		int flags;
		int pitch;
		float delay;
		int speakerentity;
		int initialStreamPosition;
		int skipInitialSamples;
		int m_nQueuedGUID;
		unsigned int m_nSoundScriptHash;
		const char* m_pSoundEntryName;
		KeyValues* m_pOperatorsKV;
		float opStackElapsedTime;
		float opStackElapsedStopTime;
		__int8 staticsound : 1;
		__int8 bUpdatePositions : 1;
		__int8 fromserver : 1;
		__int8 bToolSound : 1;
		__int8 m_bIsScriptHandle : 1;
		__int8 m_bDelayedStart : 1;
	};

	struct CBaseEntity_vtbl;
	struct CBaseEntity
	{
		CBaseEntity_vtbl* vtbl;
		void* m_iObjectCapsCache;
		void(__thiscall* m_pfnMoveDone)(CBaseEntity*);
		void(__thiscall* m_pfnThink)(CBaseEntity*);
		char m_Network[0x50]; // CServerNetworkProperty
		const char* m_iClassname;
		float m_flPrevAnimTime;
		float m_flAnimTime;
		float m_flSimulationTime;
		int m_nLastThinkTick;
		int touchStamp;
		char m_aThinkFunctions[0x14];
		char m_ResponseContexts[0x14];
		const char* m_iszResponseContext;
		int m_nNextThinkTick;
		int m_fEffects;
		const char* m_ModelName;
		CBaseEntity* m_pLink;
		const char* m_target;
		char m_nRenderFX;
		char m_nRenderMode;
		uint16_t m_nModelIndex;
		DWORD m_clrRender;
		int m_nSimulationTick;
		int m_fDataObjectTypes;
		int m_iEFlags;
		int m_fFlags;
		const char* m_iName;
		DWORD m_pParent;
		unsigned __int8 m_nTransmitStateOwnedCounter;
		char m_iParentAttachment;
		char m_MoveType;
		char m_MoveCollide;
		int m_hMoveParent;
		DWORD m_hMoveChild;
		DWORD m_hMovePeer;
		char m_Collision[0x5C];
		DWORD m_hOwnerEntity;
		int m_CollisionGroup;
		void* m_pPhysicsObject;
		unsigned __int8 m_nWaterTouch;
		unsigned __int8 m_nSlimeTouch;
		unsigned __int8 m_nWaterType;
		char m_nWaterLevel;
		float m_flNavIgnoreUntilTime;
		DWORD m_hGroundEntity;
		float m_flGroundChangeTime;
		Vector m_vecBaseVelocity;
		Vector m_vecAbsVelocity;
		QAngle m_vecAngVelocity;
		matrix3x4_t m_rgflCoordinateFrame;
		float m_flFriction;
		float m_flElasticity;
		float m_flLocalTime;
		float m_flVPhysicsUpdateLocalTime;
		float m_flMoveDoneTime;
		int m_nPushEnumCount;
		Vector m_vecAbsOrigin;
		QAngle m_angAbsRotation;
		Vector m_vecVelocity;
		DWORD m_pBlocker;
		int m_iTextureFrameIndex;
		bool m_bSimulatedEveryTick;
		bool m_bAnimatedEveryTick;
		bool m_bAlternateSorting;
		int m_nMinCPULevel;
		int m_nMaxCPULevel;
		int m_nMinGPULevel;
		int m_nMaxGPULevel;
		const char* m_iGlobalname;
		const char* m_iParent;
		int m_iHammerID;
		float m_flSpeed;
		int m_iMaxHealth;
		int m_iHealth;
		const char* m_iszDamageFilterName;
		DWORD m_hDamageFilter;
		/*void(__thiscall* m_pfnTouch)(CBaseEntity*, CBaseEntity*);
		void(__thiscall* m_pfnUse)(CBaseEntity*, CBaseEntity*, CBaseEntity*, int, float);
		void(__thiscall* m_pfnBlocked)(CBaseEntity*, CBaseEntity*);
		CNetworkVarBase<bool, CBaseEntity::NetworkVar_m_bClientSideRagdoll> m_bClientSideRagdoll;
		CNetworkVarBase<char, CBaseEntity::NetworkVar_m_lifeState> m_lifeState;
		CNetworkVarBase<char, CBaseEntity::NetworkVar_m_takedamage> m_takedamage;
		CNetworkVarBase<bool, CBaseEntity::NetworkVar_m_bIsPlayerSimulated> m_bIsPlayerSimulated;
		CNetworkHandleBase<CBasePlayer, CBaseEntity::NetworkVar_m_hPlayerSimulationOwner> m_hPlayerSimulationOwner;
		COutputEvent m_OnUser1;
		COutputEvent m_OnUser2;
		COutputEvent m_OnUser3;
		COutputEvent m_OnUser4;
		COutputEvent m_OnKilled;
		int m_cellwidth;
		CNetworkVarBase<int, CBaseEntity::NetworkVar_m_cellbits> m_cellbits;
		CNetworkVarBase<int, CBaseEntity::NetworkVar_m_cellX> m_cellX;
		CNetworkVarBase<int, CBaseEntity::NetworkVar_m_cellY> m_cellY;
		CNetworkVarBase<int, CBaseEntity::NetworkVar_m_cellZ> m_cellZ;
		CNetworkVectorXY_SeparateZBase<Vector, CBaseEntity::NetworkVar_m_vecOrigin> m_vecOrigin;
		CNetworkVectorXYZBase<QAngle, CBaseEntity::NetworkVar_m_angRotation> m_angRotation;
		CBaseHandle m_RefEHandle;
		CNetworkVectorXYZBase<Vector, CBaseEntity::NetworkVar_m_vecViewOffset> m_vecViewOffset;
		unsigned int m_ListByClass;
		CBaseEntity* m_pPrevByClass;
		CBaseEntity* m_pNextByClass;
		int m_iInitialTeamNum;
		CNetworkVarBase<int, CBaseEntity::NetworkVar_m_iTeamNum> m_iTeamNum;
		bool m_bDynamicModelAllowed;
		bool m_bDynamicModelPending;
		CNetworkVarBase<int, CBaseEntity::NetworkVar_m_spawnflags> m_spawnflags;
		string_t m_AIAddOn;
		float m_flGravity;
		CNetworkHandleBase<CBaseEntity, CBaseEntity::NetworkVar_m_hEffectEntity> m_hEffectEntity;
		CNetworkVarBase<float, CBaseEntity::NetworkVar_m_fadeMinDist> m_fadeMinDist;
		CNetworkVarBase<float, CBaseEntity::NetworkVar_m_fadeMaxDist> m_fadeMaxDist;
		CNetworkVarBase<float, CBaseEntity::NetworkVar_m_flFadeScale> m_flFadeScale;
		CNetworkVarBase<float, CBaseEntity::NetworkVar_m_flShadowCastDistance> m_flShadowCastDistance;
		float m_flDesiredShadowCastDistance;
		CNetworkVarBase<string_t, CBaseEntity::NetworkVar_m_iSignifierName> m_iSignifierName;
		bool m_bNetworkQuantizeOriginAndAngles;
		bool m_bLagCompensate;
		bool m_bForcePurgeFixedupStrings;
		CGlobalEvent* m_pEvent;
		int m_debugOverlays;
		TimedOverlay_t* m_pTimedOverlay;
		string_t m_iszVScripts;
		string_t m_iszScriptThinkFunction;
		CScriptScopeT<CDefScriptScopeBase> m_ScriptScope;
		HSCRIPT__* m_hScriptInstance;
		string_t m_iszScriptId;
		CScriptKeyValues* m_pScriptModelKeyValues;*/
	};
	STATIC_ASSERT_OFFSET(CBaseEntity, touchStamp, 0x74);
	STATIC_ASSERT_OFFSET(CBaseEntity, m_target, 0xB4);
	STATIC_ASSERT_OFFSET(CBaseEntity, m_nModelIndex, 0xBA);
	STATIC_ASSERT_OFFSET(CBaseEntity, m_pParent, 0xD4);
	STATIC_ASSERT_OFFSET(CBaseEntity, m_hMoveParent, 0xDC);
	STATIC_ASSERT_OFFSET(CBaseEntity, m_pPhysicsObject, 0x14C);
	STATIC_ASSERT_OFFSET(CBaseEntity, m_vecAbsVelocity, 0x16C);
	STATIC_ASSERT_OFFSET(CBaseEntity, m_vecAbsOrigin, 0x1CC);

	struct ScriptClassDesc_t
	{
		const char* m_pszScriptName;
		const char* m_pszClassname;
		const char* m_pszDescription;
		ScriptClassDesc_t* m_pBaseDesc;
		char m_FunctionBindings[0x14];
		void* (__cdecl* m_pfnConstruct)();
		void(__cdecl* m_pfnDestruct)(void*);
		void* pHelper;
		ScriptClassDesc_t* m_pNextDesc;
	};

	struct ServerClass
	{
		char* m_pNetworkName;
		void* m_pTable;
		ServerClass* m_pNext;
		int m_ClassID;
		int m_InstanceBaselineIndex;
	};

	struct CBaseEntity_vtbl
	{
		void(__thiscall* CBaseEntity_Destructor)(struct CBaseEntity*);
		void(__thiscall* SetRefEHandle)(struct CBaseEntity*, const CBaseHandle*);
		const CBaseHandle* (__thiscall* GetRefEHandle)(struct CBaseEntity*);
		void* (__thiscall* GetCollideable)(struct CBaseEntity*);
		void* (__thiscall* GetNetworkable)(struct CBaseEntity*);
		CBaseEntity* (__thiscall* GetBaseEntity)(struct CBaseEntity*);
		int(__thiscall* GetModelIndex)(struct CBaseEntity*);
		const char*(__thiscall* GetModelName)(struct CBaseEntity*);
		void(__thiscall* SetModelIndex)(struct CBaseEntity*, int);
		ServerClass* (__thiscall* GetServerClass)(CBaseEntity*);
		int(__thiscall* YouForgotToImplementOrDeclareServerClass)(CBaseEntity*);
		void* (__thiscall* GetDataDescMap)(CBaseEntity*); // datamap_t
		ScriptClassDesc_t* (__thiscall* GetScriptDesc)(CBaseEntity*);
		const char*(__thiscall* GetAIAddOn)(CBaseEntity*);
		bool(__thiscall* TestCollision)(CBaseEntity*, const Ray_t*, unsigned int, void*); // CGameTrace
		bool(__thiscall* TestHitboxes)(CBaseEntity*, const Ray_t*, unsigned int, void*);
		void(__thiscall* ComputeWorldSpaceSurroundingBox)(CBaseEntity*, Vector*, Vector*);
		bool(__thiscall* ShouldCollide)(CBaseEntity*, int, int);
		void(__thiscall* SetOwnerEntity)(CBaseEntity*, CBaseEntity*);
		void(__thiscall* SetScriptOwnerEntity)(CBaseEntity*, void*);
		int(__thiscall* ShouldTransmit)(CBaseEntity*, const void*);
		int(__thiscall* UpdateTransmitState)(CBaseEntity*);
		void(__thiscall* SetTransmit)(CBaseEntity*, void*, bool);
		const char* (__thiscall* GetTracerType)(CBaseEntity*);
		void(__thiscall* Spawn)(CBaseEntity*);
		void(__thiscall* Precache)(CBaseEntity*);
		void(__thiscall* SetModel)(CBaseEntity*, const char*);
		CStudioHdr* (__thiscall* OnNewModel)(CBaseEntity*);
		void(__thiscall* InitSharedVars)(CBaseEntity*);
		void(__thiscall* PostConstructor)(CBaseEntity*, const char*);
		void(__thiscall* PostClientActive)(CBaseEntity*);
		void(__thiscall* OnParseMapDataFinished)(CBaseEntity*);
		bool(__thiscall* KeyValue1)(CBaseEntity*, const char*, const Vector*);
		bool(__thiscall* KeyValue2)(CBaseEntity*, const char*, int);
		bool(__thiscall* KeyValue3)(CBaseEntity*, const char*, float);
		bool(__thiscall* KeyValue4)(CBaseEntity*, const char*, const char*);
		bool(__thiscall* GetKeyValue)(CBaseEntity*, const char*, char*, int);
		void(__thiscall* Activate)(CBaseEntity*);
		void(__thiscall* SetParent)(CBaseEntity*, CBaseEntity*, int);
		int(__thiscall* ObjectCaps)(CBaseEntity*);
		bool(__thiscall* AcceptInput)(CBaseEntity*, const char*, CBaseEntity*, CBaseEntity*, int, int);
		const char* (__thiscall* GetPlayerName)(CBaseEntity*);
		void(__thiscall* DrawDebugGeometryOverlays)(CBaseEntity*);
		int(__thiscall* DrawDebugTextOverlays)(CBaseEntity*);
		int(__thiscall* Save)(CBaseEntity*, void*);
		int(__thiscall* Restore)(CBaseEntity*, void*);
		bool(__thiscall* ShouldSavePhysics)(CBaseEntity*);
		void(__thiscall* OnSave)(CBaseEntity*, void*);
		void(__thiscall* OnRestore)(CBaseEntity*);
		int(__thiscall* RequiredEdictIndex)(CBaseEntity*);
		void(__thiscall* MoveDone)(CBaseEntity*);
		void(__thiscall* Think)(CBaseEntity*);
		void(__thiscall* NetworkStateChanged_m_nNextThinkTick1)(CBaseEntity*, void*);
		void(__thiscall* NetworkStateChanged_m_nNextThinkTick2)(CBaseEntity*);
		void* (__thiscall* GetBaseAnimating)(CBaseEntity*); // CBaseAnimating
		void* (__thiscall* GetBaseAnimatingOverlay)(CBaseEntity*); // CBaseAnimatingOverlay
		void* (__thiscall* GetResponseSystem)(CBaseEntity*);
		void(__thiscall* DispatchResponse)(CBaseEntity*, const char*);
		int(__thiscall* Classify)(CBaseEntity*);
		void(__thiscall* DeathNotice)(CBaseEntity*, CBaseEntity*);
		bool(__thiscall* ShouldAttractAutoAim)(CBaseEntity*, CBaseEntity*);
		float(__thiscall* GetAutoAimRadius)(CBaseEntity*);
		Vector* (__thiscall* GetAutoAimCenter)(CBaseEntity*, Vector* result);
		void* (__thiscall* GetBeamTraceFilter)(CBaseEntity*);
		bool(__thiscall* PassesDamageFilter)(CBaseEntity*, const void*);
		void(__thiscall* TraceAttack)(CBaseEntity*, const void*, const Vector*, void*);
		bool(__thiscall* CanBeHitByMeleeAttack)(CBaseEntity*, CBaseEntity*);
		int(__thiscall* OnTakeDamage)(CBaseEntity*, const void*);
		int(__thiscall* TakeHealth)(CBaseEntity*, float, int);
		bool(__thiscall* IsAlive)(CBaseEntity*);
		void(__thiscall* Event_Killed)(CBaseEntity*, const void*);
		void(__thiscall* Event_KilledOther)(CBaseEntity*, CBaseEntity*, const void*);
		int(__thiscall* BloodColor)(CBaseEntity*);
		bool(__thiscall* IsTriggered)(CBaseEntity*, CBaseEntity*);
		bool(__thiscall* IsNPC)(CBaseEntity*);
		void* (__thiscall* MyNPCPointer)(CBaseEntity*);
		void* (__thiscall* MyCombatCharacterPointer)(CBaseEntity*);
		void* (__thiscall* MyNextBotPointer)(CBaseEntity*);
		float(__thiscall* GetDelay)(CBaseEntity*);
		bool(__thiscall* IsMoving)(CBaseEntity*);
		const char* (__thiscall* DamageDecal)(CBaseEntity*, int, int);
		void(__thiscall* DecalTrace)(CBaseEntity*, void*, const char*);
		void(__thiscall* ImpactTrace)(CBaseEntity*, void*, int, char*);
		bool(__thiscall* OnControls)(CBaseEntity*, CBaseEntity*);
		bool(__thiscall* HasTarget)(CBaseEntity*, const char*);
		bool(__thiscall* IsPlayer)(CBaseEntity*);
		bool(__thiscall* IsNetClient)(CBaseEntity*);
		bool(__thiscall* IsTemplate)(CBaseEntity*);
		bool(__thiscall* IsBaseObject)(CBaseEntity*);
		bool(__thiscall* IsBaseTrain)(CBaseEntity*);
		bool(__thiscall* IsBaseCombatWeapon)(CBaseEntity*);
		void* (__thiscall* MyCombatWeaponPointer)(CBaseEntity*);
		void* (__thiscall* GetServerVehicle)(CBaseEntity*);
		bool(__thiscall* IsViewable)(CBaseEntity*);
		void(__thiscall* ChangeTeam)(CBaseEntity*, int);
		void(__thiscall* OnEntityEvent)(CBaseEntity*, int, void*);
		bool(__thiscall* CanStandOn1)(CBaseEntity*, void*);
		bool(__thiscall* CanStandOn2)(CBaseEntity*, CBaseEntity*);
		CBaseEntity* (__thiscall* GetEnemy1)(CBaseEntity*);
		CBaseEntity* (__thiscall* GetEnemy2)(CBaseEntity*);
		void(__thiscall* UpdatePaintPowersFromContacts)(CBaseEntity*);
		void(__thiscall* Use)(CBaseEntity*, CBaseEntity*, CBaseEntity*, int, float);
		void(__thiscall* StartTouch)(CBaseEntity*, CBaseEntity*);
		void(__thiscall* Touch)(CBaseEntity*, CBaseEntity*);
		void(__thiscall* EndTouch)(CBaseEntity*, CBaseEntity*);
		void(__thiscall* StartBlocked)(CBaseEntity*, CBaseEntity*);
		void(__thiscall* Blocked)(CBaseEntity*, CBaseEntity*);
		void(__thiscall* EndBlocked)(CBaseEntity*);
		void(__thiscall* PhysicsSimulate)(CBaseEntity*);
		void(__thiscall* PhysicsLandedOnGround)(CBaseEntity*, float);
		void(__thiscall* UpdateOnRemove)(CBaseEntity*);
		void(__thiscall* StopLoopingSounds)(CBaseEntity*);
		bool(__thiscall* SUB_AllowedToFade)(CBaseEntity*);
		void(__thiscall* Teleport)(CBaseEntity*, const Vector*, const QAngle*, const Vector*, bool);
		void(__thiscall* NotifySystemEvent)(CBaseEntity*, CBaseEntity*, int, const void*);
		void(__thiscall* MakeTracer)(CBaseEntity*, const Vector*, const void*, int);
		int(__thiscall* GetTracerAttachment)(CBaseEntity*);
		void(__thiscall* FireBullets)(CBaseEntity*, const void*);
		void(__thiscall* DoImpactEffect)(CBaseEntity*, void*, int);
		CBaseEntity* (__thiscall* Respawn)(CBaseEntity*);
		bool(__thiscall* IsLockedByMaster)(CBaseEntity*);
		int(__thiscall* GetMaxHealth)(CBaseEntity*);
		void(__thiscall* SetHealth)(CBaseEntity*, int);
		void(__thiscall* ModifyOrAppendCriteria)(CBaseEntity*, void*);
		void(__thiscall* ModifyOrAppendDerivedCriteria)(CBaseEntity*, void*);
		int(__thiscall* GetDamageType)(CBaseEntity*);
		float(__thiscall* GetDamage)(CBaseEntity*);
		void(__thiscall* SetDamage)(CBaseEntity*, float);
		Vector* (__thiscall* EyePosition)(CBaseEntity*, Vector* result);
		const QAngle* (__thiscall* EyeAngles)(CBaseEntity*);
		const QAngle* (__thiscall* LocalEyeAngles)(CBaseEntity*);
		Vector* (__thiscall* EarPosition)(CBaseEntity*, Vector* result);
		Vector* (__thiscall* BodyTarget)(CBaseEntity*, Vector* result, const Vector*, bool);
		Vector* (__thiscall* HeadTarget)(CBaseEntity*, Vector* result, const Vector*);
		void(__thiscall* GetVectors)(CBaseEntity*, Vector*, Vector*, Vector*);
		const Vector* (__thiscall* GetViewOffset)(CBaseEntity*);
		void(__thiscall* SetViewOffset)(CBaseEntity*, const Vector*);
		Vector* (__thiscall* GetSmoothedVelocity)(CBaseEntity*, Vector* result);
		void(__thiscall* GetVelocity)(CBaseEntity*, Vector*, Vector*);
		float(__thiscall* GetFriction)(CBaseEntity*);
		bool(__thiscall* FVisible1)(CBaseEntity*, const Vector*, int, CBaseEntity**);
		bool(__thiscall* FVisible2)(CBaseEntity*, CBaseEntity*, int, CBaseEntity**);
		bool(__thiscall* CanBeSeenBy)(CBaseEntity*, void*);
		float(__thiscall* GetAttackDamageScale)(CBaseEntity*, CBaseEntity*);
		float(__thiscall* GetReceivedDamageScale)(CBaseEntity*, CBaseEntity*);
		void(__thiscall* OnGroundChanged)(CBaseEntity*, CBaseEntity*, CBaseEntity*);
		void(__thiscall* GetGroundVelocityToApply)(CBaseEntity*, Vector*);
		bool(__thiscall* PhysicsSplash)(CBaseEntity*, const Vector*, const Vector*, float, float);
		void(__thiscall* Splash)(CBaseEntity*);
		const Vector* (__thiscall* WorldSpaceCenter)(CBaseEntity*);
		Vector* (__thiscall* GetSoundEmissionOrigin)(CBaseEntity*, Vector* result);
		bool(__thiscall* CreateVPhysics)(CBaseEntity*);
		bool(__thiscall* ForceVPhysicsCollide)(CBaseEntity*, CBaseEntity*);
		void(__thiscall* VPhysicsDestroyObject)(CBaseEntity*);
		void(__thiscall* VPhysicsUpdate)(CBaseEntity*, void*);
		int(__thiscall* VPhysicsTakeDamage)(CBaseEntity*, const void*);
		void(__thiscall* VPhysicsShadowCollision)(CBaseEntity*, int, void*);
		void(__thiscall* VPhysicsShadowUpdate)(CBaseEntity*, void*);
		void(__thiscall* VPhysicsCollision)(CBaseEntity*, int, void*);
		void(__thiscall* VPhysicsFriction)(CBaseEntity*, void*, float, int, int);
		void(__thiscall* UpdatePhysicsShadowToCurrentPosition)(CBaseEntity*, float);
		int(__thiscall* VPhysicsGetObjectList)(CBaseEntity*, void**, int);
		bool(__thiscall* VPhysicsIsFlesh)(CBaseEntity*);
		bool(__thiscall* CanPushEntity)(CBaseEntity*, CBaseEntity*);
		void* (__thiscall* HasPhysicsAttacker)(CBaseEntity*, float);
		unsigned int(__thiscall* PhysicsSolidMaskForEntity)(CBaseEntity*);
		void(__thiscall* ResolveFlyCollisionCustom)(CBaseEntity*, void*, Vector*);
		void(__thiscall* PerformCustomPhysics)(CBaseEntity*, Vector*, Vector*, QAngle*, QAngle*);
		Vector* (__thiscall* GetStepOrigin)(CBaseEntity*, Vector* result);
		QAngle* (__thiscall* GetStepAngles)(CBaseEntity*, QAngle* result);
	};

	struct HierarchicalSpawn_t
	{
		CBaseEntity* m_pEntity;
		int m_nDepth;
		CBaseEntity* m_pDeferredParent;
		const char* m_pDeferredParentAttachment;
	};

	struct StaticPropDict_t
	{
		model_t* m_pModel;
	};

	struct StaticPropDict_t_Mem
	{
		StaticPropDict_t* m_pMemory;
		int m_nAllocationCount;
		int m_nGrowSize;
	};

	struct StaticPropDict_t_vec
	{
		StaticPropDict_t_Mem m_Memory;
		std::uint32_t m_Size;
		StaticPropDict_t* m_pElements;
	};

	struct  ICollideable
	{
		void* vftable;
	};

	struct CStaticProp : IClientUnknown, IClientRenderable, ICollideable, IClientModelRenderable
	{
		Vector m_Origin;
		QAngle m_Angles;
		model_t* m_pModel;
		unsigned __int16 m_Partition;
		unsigned __int16 m_ModelInstance;
		unsigned __int8 m_Alpha;
		unsigned __int8 m_nSolidType;
		unsigned __int8 m_Skin;
		unsigned __int8 m_Flags;
		unsigned __int8 m_nMinCPULevel;
		unsigned __int8 m_nMaxCPULevel;
		unsigned __int8 m_nMinGPULevel;
		unsigned __int8 m_nMaxGPULevel;
		unsigned __int16 m_FirstLeaf;
		unsigned __int16 m_LeafCount;
		CBaseHandle m_EntHandle;
		unsigned __int16 m_RenderHandle;
		unsigned __int16 m_nReserved;
		void* m_pClientAlphaProperty;
		Vector m_RenderBBoxMin;
		Vector m_RenderBBoxMax;
		matrix3x4_t m_ModelToWorld;
		float m_flRadius;
		Vector m_WorldRenderBBoxMin;
		Vector m_WorldRenderBBoxMax;
		Vector m_LightingOrigin;
		Vector4D m_DiffuseModulation;
	};

	struct CStaticProp_Mem
	{
		CStaticProp* m_pMemory;
		int m_nAllocationCount;
		int m_nGrowSize;
	};

	struct CStaticProp_vec
	{
		StaticPropDict_t_Mem m_Memory;
		std::uint32_t m_Size;
		CStaticProp* m_pElements;
	};

	struct IStaticPropMgrEngine
	{
		void* vftable;
	};

	struct IStaticPropMgr
	{
		void* vftable;
	};

	struct  IStaticPropMgrClient : IStaticPropMgr
	{ };

	struct IStaticPropMgrServer : IStaticPropMgr
	{ };

	struct CStaticPropMgr : IStaticPropMgrEngine, IStaticPropMgrClient, IStaticPropMgrServer
	{
		//CUtlVector<CStaticPropMgr::StaticPropDict_t, CUtlMemory<CStaticPropMgr::StaticPropDict_t, int> > m_StaticPropDict;
		StaticPropDict_t_vec m_StaticPropDict;

		//CUtlVector<CStaticProp, CUtlMemory<CStaticProp, int> > m_StaticProps;
		CStaticProp_vec m_StaticProps;
	};

	// -----

	struct IMatRenderContext_vtbl;
	struct IMatRenderContext
	{
		IMatRenderContext_vtbl* vtbl;
	};

	enum MaterialMatrixMode_t : __int32
	{
		MATERIAL_VIEW = 0x0,
		MATERIAL_PROJECTION = 0x1,
		MATERIAL_MATRIX_UNUSED0 = 0x2,
		MATERIAL_MATRIX_UNUSED1 = 0x3,
		MATERIAL_MATRIX_UNUSED2 = 0x4,
		MATERIAL_MATRIX_UNUSED3 = 0x5,
		MATERIAL_MATRIX_UNUSED4 = 0x6,
		MATERIAL_MATRIX_UNUSED5 = 0x7,
		MATERIAL_MATRIX_UNUSED6 = 0x8,
		MATERIAL_MATRIX_UNUSED7 = 0x9,
		MATERIAL_MODEL = 0xA,
		NUM_MATRIX_MODES = 0xB,
	};

	struct IMatRenderContext_vtbl
	{
		int pad[29];
		void(__thiscall* GetMatrix1)(IMatRenderContext*, MaterialMatrixMode_t, matrix3x4_t*);
		void(__thiscall* GetMatrix2)(IMatRenderContext*, MaterialMatrixMode_t, VMatrix*);
	};

	struct IMaterialSystem_vtbl;
	struct IMaterialSystem
	{
		IMaterialSystem_vtbl* vtbl;
	};

	struct IMaterialSystem_vtbl
	{
		char pad[0x180];
		IMatRenderContext* (__thiscall* GetRenderContext)(IMaterialSystem*);
	};

	struct C_ServerSurvivorBotParent
	{
		void* vtbl; //0x0000 
		char pad_0x0004[0xC]; //0x0004
		int unk_one; //0x0010 
		char pad_0x0014[0x60]; //0x0014
		const char* classname_player; //0x0074 
		char pad_0x0078[0x54]; //0x0078
		int has_flashlight_0xCC; //0x00CC flag 4 = flashlight
		char pad_0x00D0[0x20]; //0x00D0
		int et_flags_0xf0; //0x00F0 
		char pad_0x00F4[0x144]; //0x00F4
		int aliveflag_or_so; //0x0238 
		char pad_0x023C[0x10]; //0x023C
		const char* modelname; //0x024C 
		char pad_0x0250[0x28]; //0x0250
		VMatrix transform_matrix; //0x0278 
		char pad_0x02B8[0x14]; //0x02B8
		Vector origin; //0x02CC 
		Vector velocity; //0x02D8 
		char pad_0x02E4[0xA4]; //0x02E4
		Vector origin2; //0x0388 
		Vector angles; //0x0394 
		char pad_0x03A0[0x3CA0]; //0x03A0
	};
	STATIC_ASSERT_OFFSET(C_ServerSurvivorBotParent, has_flashlight_0xCC, 0xCC);
	STATIC_ASSERT_OFFSET(C_ServerSurvivorBotParent, modelname, 0x24C);
	STATIC_ASSERT_OFFSET(C_ServerSurvivorBotParent, transform_matrix, 0x278);
	STATIC_ASSERT_OFFSET(C_ServerSurvivorBotParent, origin, 0x2CC);
}



