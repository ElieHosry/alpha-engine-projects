// ---------------------------------------------------------------------------
// Project Name		:	Alpha Engine
// File Name		:	Graphics.cpp
// Author			:	Sun Tjen Fam, Antoine Abi Chakra, Gerald Wong
// Creation Date	:	2008/01/31
// Purpose			:	Implementation of graphics component
// History			:
// - 2008/01/31		:	- initial implementation
// - 2010/08/17 : Fixed a number of loss of precision warnings.
//                Fixed the initialization of DirectX that was causing VSync
//                    to always be turned on, causing the framerate controller
//                    to appear to not be working correctly. - Dan Weiss
// 
// 
// ---------------------------------------------------------------------------


#include "AEExport.h"
#include "AETypes.h"
#include "AEGraphics.h"
#include "AEUtil.h"
#include "AEMath.h"
#include <math.h>
#include "Point4.h"
#include "Vector4.h"
#include "Matrix4.h"
#include "3DPipelineTools.h"

#include <string>
#include <iostream>
#include <fstream>

#if defined(__EMSCRIPTEN__)
  // --- Web build: GLES2/WebGL headers + SDL_image texture decode ----------
  #include <SDL.h>
  #include <SDL_image.h>
  #include <SDL_opengles2.h>
#else
  // --- Native Windows build (unchanged) -----------------------------------
  #include <windows.h>

  // Surpress GdiPlus warnings temporarily
  #pragma warning(disable:4458)
  #include <GdiPlus.h>
  #pragma warning(default:4458)

  #ifndef GLEW_STATIC
  # define GLEW_STATIC // GLEW_STATIC to use the static version of the GLEW library
  #endif
  #include <GL/glew.h>

  #include <SDL.h>

  #include <conio.h>
#endif

#include "AEEngine.h"

#if !defined(__EMSCRIPTEN__)
//#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "openGL32.lib")
//#pragma comment (lib, "glu32.lib")
#pragma comment (lib, "Gdiplus.lib")
#endif // !__EMSCRIPTEN__

// window related variables
extern HINSTANCE	ghAESysAppInstance;
extern HWND			gAESysWindowHandle;
#if !defined(__EMSCRIPTEN__)
extern WNDCLASS		winClass;
#endif

extern const char*	gpAESysWinTitle;
extern const char*	gpAESysWinClassName;

extern SDL_Window*	gAESDLWindow;		// created in AESystem.cpp

extern f32 gResolutionRatioX;
extern f32 gResolutionRatioY;

AEGfxBlendMode gCurrentBlendMode = AE_GFX_BM_NONE;

//extern AE_API s32	gAESysAppActive;

SDL_GLContext	mGLContext = nullptr;	// OpenGL context bound to gAESDLWindow

#define GENERATE_OPENGL_LOG			0

void BruteQuit(void)
{
#if !defined(__EMSCRIPTEN__)
	_getch();			// wait for a keypress in the console (native only)
#endif
	exit(1);
}


char* ReadFromFile(const char* fileName)
{
	std::ifstream in(fileName);

	if (in.is_open())
	{
		in.seekg(0, std::ios::end);
		s32 length = (s32)in.tellg();
		in.seekg(0, std::ios::beg);

		char* buffer = new char[length];
		memset(buffer, 0, length);

		in.read(buffer, length);
		in.close();

		return buffer;
	}

	return NULL;
}


// 0 = From strings
// 1 = From files
#define SHADER_FROM_FILES		0

#if defined(__EMSCRIPTEN__)
// On WebGL every glGetError() is a synchronous CPU<->GPU round-trip that flushes
// the command pipeline - calling it per draw/frame destroys performance (it was
// ~86% of frame time here). Compile it out on the web; rely on the browser's
// own WebGL error logging during development instead.
#define CHECK_GL_ERROR(...)		((void)0)
#else
#define CHECK_GL_ERROR(...)														\
{																				\
	u32 errorsFound = 0;												\
	for (GLint glError = glGetError(); glError; glError = glGetError()) 		\
	{																			\
		PRINT("CHECK_GL_ERROR:\nLine: %d\nFunc: %s\nFile: %s\n",				\
			__LINE__, __FUNCTION__, __FILE__);									\
		PRINT("Code: 0x%0.8x\n", glError);										\
		PRINT("Mesg: " __VA_ARGS__);												\
		PRINT("\n");															\
		++errorsFound;															\
	}																			\
	if(errorsFound)																\
		BruteQuit();															\
}
#endif


#define HALT(...)											\
{															\
	PRINT("HALT:\nLine: %d\nFunc: %s\nFile: %s\n",			\
		__LINE__, __FUNCTION__, __FILE__);					\
	if (" " # __VA_ARGS__[1]) PRINT("Mesg: " __VA_ARGS__);	\
	PRINT("\n");											\
	BruteQuit();											\
}

#pragma region Shader
struct SHADER_TYPE
{
	enum
	{
		NONE,
		COLOR,
		TEXTURE,
		FONT,

		NUM
	};
};

	static std::string gVertexShaderCode[SHADER_TYPE::NUM] = 
	{
		// NONE --------------------------------------------------------
		"",

		// COLOR --------------------------------------------------------
		"uniform mat4 uPMatrix;							\
		uniform mat4 uVMatrix;							\
		uniform mat4 uWorldMatrix; 						\
		attribute vec4 aPosition;						\
		attribute vec4 aColor;							\
		varying vec4 vColor;							\
		void main()										\
		{												\
			vColor = aColor;							\
			gl_Position = uPMatrix * uVMatrix * uWorldMatrix * aPosition;	\
		}",

		// TEXTURE --------------------------------------------------------
		"uniform mat4 uPMatrix;							\
		uniform mat4 uVMatrix;							\
		uniform mat4 uWorldMatrix;						\
		attribute vec4 aPosition;						\
		attribute vec2 aTextureCoordinate;				\
		varying vec2 vTextureCoordinate;				\
		void main()										\
		{												\
			vTextureCoordinate = aTextureCoordinate;	\
			gl_Position = uPMatrix * uVMatrix * uWorldMatrix * aPosition;	\
		}",

		// FONT --------------------------------------------------------
		"attribute vec4 aCoords;\
		varying vec2 vTextureCoordinate;\
		void main(void) { \
			gl_Position = vec4(aCoords.xy, 0, 1);\
			vTextureCoordinate = aCoords.zw;\
		}"		
	};


	static std::string gFragmentShaderCode[SHADER_TYPE::NUM] = 
	{
		// NONE -------------------------------------
		"",

		// COLOR ------------------------------------
		"/*precision mediump float;	*/				\
		uniform vec4 uBlendColor;					\
		uniform float uTransparencyValue;			\
		uniform vec4 uMultiplyColor;				\
		uniform vec4 uAddColor;				\
		varying vec4 vColor;						\
		void main()									\
		{\
			gl_FragColor.x = (vColor.x * (1.0 - uBlendColor.w) + uBlendColor.y * uBlendColor.w) * uMultiplyColor.x + uAddColor.x;\
			gl_FragColor.y = (vColor.y * (1.0 - uBlendColor.w) + uBlendColor.y * uBlendColor.w) * uMultiplyColor.y + uAddColor.y;\
			gl_FragColor.z = (vColor.z * (1.0 - uBlendColor.w) + uBlendColor.z * uBlendColor.w) * uMultiplyColor.z + uAddColor.z;\
			gl_FragColor.w = vColor.w * uTransparencyValue * uMultiplyColor.w + uAddColor.w;\
		}",

		// TEXTURE ----------------------------------
		"/*precision mediump float;*/				\
		uniform sampler2D uTexture;					\
		uniform vec4 uBlendColor;					\
		uniform vec2 uTextureOffset;				\
		uniform float uTransparencyValue;			\
		uniform vec4 uMultiplyColor;				\
		uniform vec4 uAddColor;				\
		varying vec2 vTextureCoordinate;			\
		void main()									\
		{\
			vec2 offsetTexCoord = vTextureCoordinate;\
			offsetTexCoord.x += uTextureOffset.x;	\
			offsetTexCoord.y += uTextureOffset.y;	\
			if(offsetTexCoord.x > 1.0)				\
				offsetTexCoord.x -= 1.0;			\
			if(offsetTexCoord.y > 1.0)				\
				offsetTexCoord.y -= 1.0;			\
			gl_FragColor = texture2D(uTexture, offsetTexCoord);\
			gl_FragColor.x = (gl_FragColor.x * (1.0 - uBlendColor.w) + uBlendColor.x * uBlendColor.w) * uMultiplyColor.x + uAddColor.x;\
			gl_FragColor.y = (gl_FragColor.y * (1.0 - uBlendColor.w) + uBlendColor.y * uBlendColor.w) * uMultiplyColor.y + uAddColor.y;\
			gl_FragColor.z = (gl_FragColor.z * (1.0 - uBlendColor.w) + uBlendColor.z * uBlendColor.w) * uMultiplyColor.z + uAddColor.z;\
			gl_FragColor.w *= (uTransparencyValue * uMultiplyColor.w)  + uAddColor.w;\
		}",

		// FONT ------------------------------------
		"varying vec2 vTextureCoordinate;\
		uniform vec4 uFontColor;\
		uniform sampler2D uTexture;\
		void main(void) {\
			gl_FragColor = vec4(uFontColor.x, uFontColor.y , uFontColor.z, texture2D(uTexture, vTextureCoordinate).a * uFontColor.w);\
		}"	
	};

	// gl_FragColor = vec4(1, 1, 1, texture2D(uTexture, vTextureCoordinate).a) * uFontColor;

class Shader
{
public:

	Shader()
	{
		for(u32 i = 0; i < SHADER_TYPE::NUM; ++i)
			mProgramID[i] = 0;

		mCurrentShaderType = 0;
	}

	~Shader()
	{

	}

	void Create()
	{
		//  COLOR SHADER ----------------------------------------------------------------------
		mProgramID[SHADER_TYPE::COLOR] = glCreateProgram();
#if(SHADER_FROM_FILES == 1)
		AddShaderFromFile(SHADER_TYPE::COLOR, "..\\Shaders\\shader_vertex_color.txt", GL_VERTEX_SHADER);
		AddShaderFromFile(SHADER_TYPE::COLOR, "..\\Shaders\\shader_fragment_color.txt", GL_FRAGMENT_SHADER);
#else
		AddShaderFromString(SHADER_TYPE::COLOR, gVertexShaderCode[SHADER_TYPE::COLOR].c_str(), GL_VERTEX_SHADER);
		AddShaderFromString(SHADER_TYPE::COLOR, gFragmentShaderCode[SHADER_TYPE::COLOR].c_str(), GL_FRAGMENT_SHADER);
#endif
		glLinkProgram(mProgramID[SHADER_TYPE::COLOR]);

#if(GENERATE_OPENGL_LOG == 1)
		{
			GLsizei returnedLength = 0;
			const u32 logSize = 10000;
			s8 log[logSize];
			memset(log, 0, logSize*sizeof(s8));
			glGetProgramInfoLog(mProgramID[SHADER_TYPE::COLOR], logSize, &returnedLength, log);

			FILE *fp = fopen("Shader_Color_Log.txt", "w");
			fprintf(fp, "%s", log);
			fclose(fp);
		}
#endif


		//  TEXTURE SHADER ----------------------------------------------------------------------
		mProgramID[SHADER_TYPE::TEXTURE] = glCreateProgram();
		
#if(SHADER_FROM_FILES == 1)
		AddShaderFromFile(SHADER_TYPE::TEXTURE, "..\\Shaders\\shader_vertex_texture.txt", GL_VERTEX_SHADER);
		AddShaderFromFile(SHADER_TYPE::TEXTURE, "..\\Shaders\\shader_fragment_texture.txt", GL_FRAGMENT_SHADER);
#else
		AddShaderFromString(SHADER_TYPE::TEXTURE, gVertexShaderCode[SHADER_TYPE::TEXTURE].c_str(), GL_VERTEX_SHADER);
		AddShaderFromString(SHADER_TYPE::TEXTURE, gFragmentShaderCode[SHADER_TYPE::TEXTURE].c_str(), GL_FRAGMENT_SHADER);
#endif
		glLinkProgram(mProgramID[SHADER_TYPE::TEXTURE]);
		
#if(GENERATE_OPENGL_LOG == 1)
		{
			GLsizei returnedLength = 0;
			const u32 logSize = 10000;
			s8 log[logSize];
			memset(log, 0, logSize*sizeof(s8));
			glGetProgramInfoLog(mProgramID[SHADER_TYPE::TEXTURE], logSize, &returnedLength, log);

			FILE *fp = fopen("Shader_Texture_Log.txt", "w");
			fprintf(fp, "%s", log);
			fclose(fp);
		}
#endif

		//  GLYPH SHADER ----------------------------------------------------------------------
		mProgramID[SHADER_TYPE::FONT] = glCreateProgram();

#if(SHADER_FROM_FILES == 1)
		AddShaderFromFile(SHADER_TYPE::GLYPH, "..\\Shaders\\shader_vertex_glyph.txt", GL_VERTEX_SHADER);
		AddShaderFromFile(SHADER_TYPE::GLYPH, "..\\Shaders\\shader_fragment_glyph.txt", GL_FRAGMENT_SHADER);
#else
		AddShaderFromString(SHADER_TYPE::FONT, gVertexShaderCode[SHADER_TYPE::FONT].c_str(), GL_VERTEX_SHADER);
		AddShaderFromString(SHADER_TYPE::FONT, gFragmentShaderCode[SHADER_TYPE::FONT].c_str(), GL_FRAGMENT_SHADER);
#endif
		glLinkProgram(mProgramID[SHADER_TYPE::FONT]);

#if(GENERATE_OPENGL_LOG == 1)
		{
			GLsizei returnedLength = 0;
			const u32 logSize = 10000;
			s8 log[logSize];
			memset(log, 0, logSize * sizeof(s8));
			glGetProgramInfoLog(mProgramID[SHADER_TYPE::GLYPH], logSize, &returnedLength, log);

			FILE* fp = fopen("Shader_Texture_Log.txt", "w");
			fprintf(fp, "%s", log);
			fclose(fp);
		}
#endif		
	}

	bool ApplyShader(u32 ShaderType)
	{
		if (mCurrentShaderType != ShaderType)
		{
			if (SHADER_TYPE::NONE == ShaderType)
			{
				UnApplyShader();
			}
			else
			{
				glUseProgram(mProgramID[ShaderType]);
				mCurrentShaderType = ShaderType;

				return true;	
			}
		}

		return false;
	}

	void UnApplyShader()
	{
		glUseProgram(0);
		mCurrentShaderType = SHADER_TYPE::NONE;
	}

	void Destroy()
	{
		for(u32 i = 0; i < SHADER_TYPE::NUM; ++i)
			if(mProgramID[i])
			{
				GLuint returnedShaders[SHADER_TYPE::NUM];
				GLsizei shaderCount = 0;

				memset(returnedShaders, 0, SHADER_TYPE::NUM*sizeof(GLuint));

				glGetAttachedShaders(mProgramID[i],
										SHADER_TYPE::NUM,
 										&shaderCount,
 										returnedShaders);

				for(u32 shaderIdx = 0; shaderIdx < (u32)shaderCount; ++shaderIdx)
					glDeleteShader(returnedShaders[shaderIdx]);

				glDeleteProgram(mProgramID[i]);
				CHECK_GL_ERROR();
			}

		mCurrentShaderType = 0;
	}
	                       
	GLint GetUniformLocation(const GLchar *name)
	{
		if(mCurrentShaderType)
			return glGetUniformLocation(mProgramID[mCurrentShaderType], name);

		return -1;
	}

	GLint GetAttributeLocation(const GLchar *name)
	{
		if(mCurrentShaderType)
			return glGetAttribLocation(mProgramID[mCurrentShaderType], name);

		return -1;
	}

	u32 GetCurrentShaderType()
	{
		return mCurrentShaderType;
	}

private:

	void AddShaderFromString(u32 ShaderType, const GLchar *pSource, GLint type)
	{
		GLuint id = glCreateShader(type);

#if defined(__EMSCRIPTEN__)
		// WebGL/GLES2 fragment shaders MUST declare a default float precision.
		// The engine's shaders are already GLSL ES 1.00 style (attribute/varying/
		// gl_FragColor/texture2D), so we only need to prepend the precision line.
		std::string esSource;
		if (type == GL_FRAGMENT_SHADER)
			esSource = "precision mediump float;\n";
		esSource += pSource;
		const GLchar* pES = esSource.c_str();
		glShaderSource(id, 1, &pES, NULL);
#else
		glShaderSource(id, 1, &pSource, NULL);
#endif

		glCompileShader(id);
		GLint status;
		glGetShaderiv(id, GL_COMPILE_STATUS, &status);

		if (status == GL_TRUE)
			glAttachShader(mProgramID[ShaderType], id);
		else
			HALT("FAILED Shader compilation !");
	}

	void AddShaderFromFile(u32 ShaderType, const GLchar *fileName, GLint type)
	{
		if (!mProgramID[ShaderType])
			return;

		char *temp = ReadFromFile(fileName);
		const char *source = temp;

		if (!source)
			return;

		AddShaderFromString(ShaderType, source, type);

		delete [] temp;
	}
		
	GLint mProgramID[SHADER_TYPE::NUM];
	u32 mCurrentShaderType;
};

#pragma endregion 

static Shader sgShader;


// maximum number of triangles per mesh
#define AE_GFX_TRI_NUM_MAX	8192

#define FRAME_RATE_SYNC_TO_RETRACE 1

#define VERTEX_FVF_POS_COLOR_TEXTURE (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1) 

static s32 gWIN_WIDTH		= 0;
static s32 gWIN_HEIGHT		= 0;
static f32 gWIN_CAM_DIST	= 60.0f;
static f32 gWIN_CAM_POS_X	= 0.0f;
static f32 gWIN_CAM_POS_Y	= 0.0f;


#define WIN_FOV				90.0f
#define WIN_CAM_NEAR_CLIP	1.0f
#define WIN_CAM_FAR_CLIP	1000.0f

f32	mViewMatrix[16];
f32	mProjectionMatrix[16];

struct BUFFER_TYPE
{
	enum
	{
		POSITION,
		COLOR,
		TEXTURE_COORDINATE,
		NUM
	};
};

s32 COORDS_PER_POSITION = 3;				// number of coordinates per position
s32 COORDS_PER_COLOR = 4;					// number of coordinates per color
s32 COORDS_PER_TEXTURE_COORDINATE = 2;		// number of coordinates per texture coordinate


s32 POSITION_STRIDE = COORDS_PER_POSITION * sizeof(float); 							// bytes per vertex position
s32 COLOR_STRIDE = COORDS_PER_COLOR * sizeof(float); 								// bytes per vertex color
s32 TEXTURE_COORDINATE_STRIDE = COORDS_PER_TEXTURE_COORDINATE * sizeof(float); 		// bytes per vertex color

#if !defined(__EMSCRIPTEN__)
ULONG_PTR m_gdiplusToken;		// GDI+ image loader (native texture decode)
#endif


AE_API f32	gAEWinMinX, gAEWinMinY, gAEWinMaxX, gAEWinMaxY;

// ---------------------------------------------------------------------------

struct VtxPosCol
{
	f32 x, y, z;
	u32 c;
	f32 tu, tv;
};


// ---------------------------------------------------------------------------

struct AEGfxVertexBuffer
{
	GLuint mBufferID[BUFFER_TYPE::NUM];
};


struct AEGfxSurface
{
	GLuint mBufferID;
	//s8 mpName[256];
};

// ---------------------------------------------------------------------------
// Static variables

// temporary buffer to accumulate vertices
static VtxPosCol	sTriBuffer[AE_GFX_TRI_NUM_MAX * 3];
static u32			sVertexNum;
static s32			sMeshFlag;



static void SetProjectionAndViewMatrices(void);

// ---------------------------------------------------------------------------

void AEGfxFixWindowMaxMin(void)
{
	gAEWinMinX = gWIN_CAM_POS_X - (f32)(gWIN_WIDTH) * 0.5f;
	gAEWinMaxX = gWIN_CAM_POS_X + (f32)(gWIN_WIDTH) * 0.5f;

	
	gAEWinMinY = gWIN_CAM_POS_Y - (f32)(gWIN_HEIGHT) * 0.5f;
	gAEWinMaxY = gWIN_CAM_POS_Y + (f32)(gWIN_HEIGHT) * 0.5f;
}

// ---------------------------------------------------------------------------

AE_API s32 AEGfxInit(s32 Width, s32 Height)
{
	
	gWIN_WIDTH = Width;
	gWIN_HEIGHT = Height;

	PRINT("Graphics: OpenGL\n");

	// create the OpenGL context on the SDL-owned window
	// (GL attributes were requested in AESysInit before the window was created)
	mGLContext = SDL_GL_CreateContext(gAESDLWindow);
	if (mGLContext == NULL)
	{
		PRINT("SDL_GL_CreateContext failed: %s\n", SDL_GetError());
		return 0;
	}

	// make it current on this thread
	SDL_GL_MakeCurrent(gAESDLWindow, mGLContext);

#if !defined(__EMSCRIPTEN__)
	// Desktop GL needs GLEW to load function pointers and verify GL 2.0.
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
		HALT("fail to initialize GLEW!!");

	if (!GLEW_VERSION_2_0)
		HALT("OpenGL 2.0 is not supported!!");
#endif
	// On the web, WebGL/GLES2 entry points are available directly - no loader needed.

	//glEnable(GL_DEPTH_TEST);

#if !defined(__EMSCRIPTEN__)
	// GDI+ is the native image decoder (used by AEGfxTextureLoad).
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
#endif


	
#if(GENERATE_OPENGL_LOG == 1)
	FILE *fp = fopen("OpenGL.txt", "w");

	fprintf(fp, "OpenGL version supported: %s\n\n", glGetString(GL_VERSION));
	fprintf(fp, "OpenGL shader version supported: %s\n\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	const s8* str = (s8 *)glGetString(GL_EXTENSIONS);
	u32 i, size = strlen(str);
	s8 *pStr = new s8[size + 1];
	memcpy(pStr, str, size);

	for(i = 0; i < size; ++i)
		if(' ' == pStr[i])
			pStr[i] = '\n';

	pStr[size] = 0;
	fprintf(fp, "Extensions:\n%s\n\n", pStr);
	fclose(fp);
	delete [] pStr;
#endif

	memset(mViewMatrix, 0, 16*sizeof(float));
	memset(mProjectionMatrix, 0, 16*sizeof(float));

	AEGfxFixWindowMaxMin();

	sgShader.Create();

	AEGfxFontSystemStart();

	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
	AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
	AEGfxSetTransparency(1.0f);

	glClearColor		(1.0f, 1.0f, 1.0f, 1.0f);

	CHECK_GL_ERROR();
	
	return 1;
}


AE_API void AEGfxSetVSync(s32 vsync)
{
	// 0 = off, 1 = on. SDL replaces the old WGL_EXT_swap_control path and maps
	// directly to eglSwapInterval / WebGL behaviour in the browser build later.
	SDL_GL_SetSwapInterval(!!vsync);
}

// ---------------------------------------------------------------------------

AE_API s32 AEGfxGetWindowWidth()
{
	return gWIN_WIDTH;
}

// ---------------------------------------------------------------------------

AE_API s32 AEGfxGetWindowHeight()
{
	return gWIN_HEIGHT;
}

// ---------------------------------------------------------------------------

AE_API void AEGfxReset()
{
	// reset the index and flag
	sVertexNum  = 0;
	sMeshFlag = false;
}

// ---------------------------------------------------------------------------

//AE_API void AEGfxUpdate()
//{
//}

// ---------------------------------------------------------------------------
AE_API void AEGfxExit()
{
	AEGfxFontSystemEnd();
	sgShader.Destroy();

	if (mGLContext)
	{
		SDL_GL_DeleteContext(mGLContext);
		mGLContext = nullptr;
	}

#if !defined(__EMSCRIPTEN__)
	Gdiplus::GdiplusShutdown(m_gdiplusToken);
#endif
}

// ---------------------------------------------------------------------------

AE_API void AEGfxStart()
{
	// check for OpenGL error
	//CHECK_GL_ERROR();

	// set the clear color and depth value
	glClear				(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	
	// set the viewport
	glViewport			(0, 0, gWIN_WIDTH, gWIN_HEIGHT);
}



// ---------------------------------------------------------------------------

AE_API void AEGfxEnd()
{
	CHECK_GL_ERROR();
	sgShader.UnApplyShader();
	CHECK_GL_ERROR();

	// swap the frame buffers (SDL owns the window surface).
	// NOTE: the old glFinish() was dropped - it stalls the GPU and is
	// actively harmful in the WebGL build; the frame-rate controller paces us.
	SDL_GL_SwapWindow(gAESDLWindow);
}


// ---------------------------------------------------------------------------


AE_API void AEGfxSetBackgroundColor(f32 Red, f32 Green, f32 Blue)
{
	glClearColor(Red, Green, Blue, 1.0f);
}


// ---------------------------------------------------------------------------

AE_API void AEGfxSetRenderMode(AEGfxRenderMode RenderMode)
{
	CHECK_GL_ERROR();

	switch(RenderMode)
	{
	case AE_GFX_RM_NONE:
		sgShader.ApplyShader(SHADER_TYPE::NONE);
		break;

	case AE_GFX_RM_COLOR:
		if(sgShader.ApplyShader(SHADER_TYPE::COLOR))
			SetProjectionAndViewMatrices();
		break;

	case AE_GFX_RM_TEXTURE:
		if(sgShader.ApplyShader(SHADER_TYPE::TEXTURE))
			SetProjectionAndViewMatrices();
		break;

	case AE_GFX_RM_NUM:		// sentinel - not a real render mode
	default:
		break;
	}
}

// ---------------------------------------------------------------------------

AE_API void	AEGfxSetBlendMode(AEGfxBlendMode BlendMode)
{
    if (BlendMode != gCurrentBlendMode) {
        gCurrentBlendMode = BlendMode;
        switch(BlendMode)
        {
            case AE_GFX_BM_NONE:
                glDisable(GL_BLEND);
                break;
            case AE_GFX_BM_BLEND:
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);//GL_ONE_MINUS_SRC_ALPHA);
                break;
            case AE_GFX_BM_ADD:
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);//GL_ONE_MINUS_SRC_ALPHA);
                break;
			case AE_GFX_BM_MULTIPLY:
				glEnable(GL_BLEND);
				glBlendFunc(GL_DST_COLOR, GL_ZERO);
				//@note: technically same as: glBlendFunc(GL_ZERO, GL_SRC_COLOR);
				break;

			case AE_GFX_BM_NUM:		// sentinel - not a real blend mode
			default:
				break;
        }
    }
}

// ---------------------------------------------------------------------------

AE_API f32 AEGfxGetWinMinX(void)
{
	return gAEWinMinX;
}		 
		   
// ---------------------------------------------------------------------------

AE_API f32 AEGfxGetWinMaxX(void)
{
	return gAEWinMaxX;
}

// ---------------------------------------------------------------------------

AE_API f32 AEGfxGetWinMinY(void)
{
	return gAEWinMinY;
}

// ---------------------------------------------------------------------------

AE_API f32 AEGfxGetWinMaxY(void)
{
	return gAEWinMaxY;
}

// ---------------------------------------------------------------------------

AE_API void AEGfxSetTransform(f32 pTransform[3][3])
{
	static float worldMatrix[16];

	memset(worldMatrix, 0, 16 * sizeof(float));

	worldMatrix[0] = pTransform[0][0];		worldMatrix[1] = pTransform[1][0];									
	worldMatrix[4] = pTransform[0][1];		worldMatrix[5] = pTransform[1][1];					
	worldMatrix[10] = 1.0f;
	worldMatrix[12] = pTransform[0][2];	worldMatrix[13] = pTransform[1][2];
																										worldMatrix[15] = 1.0f;
	s32 mWorldMatrixHandle = sgShader.GetUniformLocation("uWorldMatrix");
	glUniformMatrix4fv(mWorldMatrixHandle, 1, 0, worldMatrix);
}

// ---------------------------------------------------------------------------

AE_API void AEGfxSetTransform3D(f32 pTransform[4][4])
{
	static float worldMatrix[16];

	worldMatrix[4] = pTransform[0][1];		worldMatrix[5] = pTransform[1][1];	worldMatrix[6] = pTransform[2][1];	worldMatrix[7] = pTransform[3][1];						
	worldMatrix[8] = pTransform[0][2];		worldMatrix[9] = pTransform[1][2];	worldMatrix[10] = pTransform[2][2];	worldMatrix[11] = pTransform[3][2];						
	worldMatrix[12] = pTransform[0][3];		worldMatrix[13] = pTransform[1][3];	worldMatrix[14] = pTransform[2][3];	worldMatrix[15] = pTransform[3][3];	

	s32 mWorldMatrixHandle = sgShader.GetUniformLocation("uWorldMatrix");
	glUniformMatrix4fv(mWorldMatrixHandle, 1, 0, worldMatrix);
}

// ---------------------------------------------------------------------------

// Set a global transparency value for the material
AE_API void AEGfxSetTransparency(f32 Alpha)	
{
	s32 trHandle = sgShader.GetUniformLocation("uTransparencyValue");
	glUniform1f(trHandle, Alpha);

	CHECK_GL_ERROR();
}

// ---------------------------------------------------------------------------

// Sets a color that will be blended with the original material
AE_API void AEGfxSetBlendColor(f32 Red, f32 Green, f32 Blue, f32 Alpha)
{
	s32 blendColorHandler = sgShader.GetUniformLocation("uBlendColor");
	glUniform4f(blendColorHandler, Red, Green, Blue, Alpha);

	CHECK_GL_ERROR();
}

// Sets a color that will be used to multiply with the original material
AE_API void AEGfxSetColorToMultiply(float Red, float Green, float Blue, float Alpha)
{
	s32 handle = sgShader.GetUniformLocation("uMultiplyColor");
	glUniform4f(handle, Red, Green, Blue, Alpha);

	CHECK_GL_ERROR();
}

// Sets a color that will be used to add with the original material
AE_API void AEGfxSetColorToAdd(float Red, float Green, float Blue, float Alpha)
{
	s32 handle = sgShader.GetUniformLocation("uAddColor");
	glUniform4f(handle, Red, Green, Blue, Alpha);

	CHECK_GL_ERROR();
}


// ---------------------------------------------------------------------------

AE_API void	AEGfxTriListInitialize	(AEGfxVertexList *pVertexList)
{
	pVertexList->mpVtxBuffer = new AEGfxVertexBuffer();
}

// ---------------------------------------------------------------------------

AE_API void	AEGfxMeshStart()
{
	sMeshFlag = true;
}

// ---------------------------------------------------------------------------

AE_API void			AEGfxTriAdd		(f32 x0, f32 y0, u32 c0, f32 tu0, f32 tv0,
									 f32 x1, f32 y1, u32 c1, f32 tu1, f32 tv1,
									 f32 x2, f32 y2, u32 c2, f32 tu2, f32 tv2)
{
	AEGfxVertexAdd(x0, y0, c0, tu0, tv0);
	AEGfxVertexAdd(x1, y1, c1, tu1, tv1);
	AEGfxVertexAdd(x2, y2, c2, tu2, tv2);
}


// ---------------------------------------------------------------------------


AE_API void			AEGfxVertexAdd		(f32 x, f32 y, u32 c, f32 tu, f32 tv)
{
	sTriBuffer[sVertexNum ].x = x;
	sTriBuffer[sVertexNum].y = y;
	sTriBuffer[sVertexNum].z = 0.0f;
	sTriBuffer[sVertexNum].c = c;
	sTriBuffer[sVertexNum].tu = tu;
	sTriBuffer[sVertexNum].tv = tv;

	//sTriNum++;
	++sVertexNum;
}


// ---------------------------------------------------------------------------

AE_API AEGfxVertexList* AEGfxMeshEnd()
{
	AEGfxVertexList* pVertexList;

	// allocate triangle list
	pVertexList = new AEGfxVertexList();
	AEGfxTriListInitialize(pVertexList);
	//AE_ASSERT_PARM(pTriList);

	//pTriList->vtxNum = sTriNum * 3;
	pVertexList->vtxNum = sVertexNum;

	// Create a buffer
	glGenBuffers(BUFFER_TYPE::NUM, pVertexList->mpVtxBuffer->mBufferID);

	GLfloat *pPositions = new GLfloat[pVertexList->vtxNum * COORDS_PER_POSITION];
	GLfloat *pColors = new GLfloat[pVertexList->vtxNum * COORDS_PER_COLOR];
	GLfloat *pTexCoords = new GLfloat[pVertexList->vtxNum * COORDS_PER_TEXTURE_COORDINATE];

	for(u32 vertexIdx = 0; vertexIdx < pVertexList->vtxNum; ++vertexIdx)
	{
		pPositions[vertexIdx * COORDS_PER_POSITION] = sTriBuffer[vertexIdx].x;
		pPositions[vertexIdx * COORDS_PER_POSITION + 1] = sTriBuffer[vertexIdx].y;
		pPositions[vertexIdx * COORDS_PER_POSITION + 2] = sTriBuffer[vertexIdx].z;

		pColors[vertexIdx * COORDS_PER_COLOR] = (static_cast<float>((sTriBuffer[vertexIdx].c >> 16) & 255)) / 255.0f;		// Red
		pColors[vertexIdx * COORDS_PER_COLOR + 1] = (static_cast<float>((sTriBuffer[vertexIdx].c >> 8) & 255)) / 255.0f;	// Green
		pColors[vertexIdx * COORDS_PER_COLOR + 2] = (static_cast<float>((sTriBuffer[vertexIdx].c >> 0) & 255)) / 255.0f;	// Blue
		pColors[vertexIdx * COORDS_PER_COLOR + 3] = (static_cast<float>((sTriBuffer[vertexIdx].c >> 24) & 255)) / 255.0f;	// Alpha


		pTexCoords[vertexIdx * COORDS_PER_TEXTURE_COORDINATE] = sTriBuffer[vertexIdx].tu;
		pTexCoords[vertexIdx * COORDS_PER_TEXTURE_COORDINATE + 1] = sTriBuffer[vertexIdx].tv;
	}


	// Bind the created buffer 0: Position 
	glBindBuffer(GL_ARRAY_BUFFER, pVertexList->mpVtxBuffer->mBufferID[BUFFER_TYPE::POSITION]);

	// Copy the data
	glBufferData(GL_ARRAY_BUFFER, pVertexList->vtxNum * POSITION_STRIDE, pPositions, GL_STATIC_DRAW);

	// Unbind the buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glFinish();

	// Bind the created buffer 1: Color
	glBindBuffer(GL_ARRAY_BUFFER, pVertexList->mpVtxBuffer->mBufferID[BUFFER_TYPE::COLOR]);

	// Copy the data
	glBufferData(GL_ARRAY_BUFFER, pVertexList->vtxNum * COLOR_STRIDE, pColors, GL_STATIC_DRAW);

	// Unbind the buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glFinish();

	// Bind the created buffer 2: Texture Coordinate
	glBindBuffer(GL_ARRAY_BUFFER, pVertexList->mpVtxBuffer->mBufferID[BUFFER_TYPE::TEXTURE_COORDINATE]);

	// Copy the data
	glBufferData(GL_ARRAY_BUFFER, pVertexList->vtxNum * TEXTURE_COORDINATE_STRIDE, pTexCoords, GL_STATIC_DRAW);

	// Unbind the buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glFinish();
	
	CHECK_GL_ERROR();

	delete [] pPositions;
	delete [] pColors;
	delete [] pTexCoords;

	// reset the index and flag
	//sTriNum  = 0;
	sVertexNum = 0;
	sMeshFlag = false;

	return pVertexList;
}

// ---------------------------------------------------------------------------

AE_API void AEGfxMeshDraw(AEGfxVertexList* pVertexList, AEGfxMeshDrawMode MeshDrawMode)
{
	GLenum drawType = 0;

	switch(MeshDrawMode)
	{
	case AE_GFX_MDM_POINTS:
		drawType = GL_POINTS;
		break;

	case AE_GFX_MDM_LINES:
		drawType = GL_LINES;
		break;

	case AE_GFX_MDM_LINES_STRIP:
		drawType = GL_LINE_STRIP;
		break;

	case AE_GFX_MDM_TRIANGLES:
		drawType = GL_TRIANGLES;
		break;

	default:
		PRINT("Mesh Draw Mode %u is invalid!\n", MeshDrawMode);
		return;
	}

	// Bind buffer 0: Position
	s32 positionHandle = sgShader.GetAttributeLocation("aPosition");
	glEnableVertexAttribArray(positionHandle);
	glBindBuffer(GL_ARRAY_BUFFER, pVertexList->mpVtxBuffer->mBufferID[BUFFER_TYPE::POSITION]);
	glVertexAttribPointer(positionHandle, 
							COORDS_PER_POSITION,
							GL_FLOAT,
							false,
							0, 
							0);

	if(sgShader.GetCurrentShaderType() == SHADER_TYPE::COLOR)
	{
		// Bind buffer 1: Color
		glBindBuffer(GL_ARRAY_BUFFER, pVertexList->mpVtxBuffer->mBufferID[BUFFER_TYPE::COLOR]);
		s32 colorHandle = sgShader.GetAttributeLocation("aColor");
		glEnableVertexAttribArray(colorHandle);
		glVertexAttribPointer(colorHandle, 
								COORDS_PER_COLOR,
								GL_FLOAT,
								false,
								0, 
								0);

		
		// Draw the triangles
		glDrawArrays(drawType, 0, pVertexList->vtxNum);
		glDisableVertexAttribArray(colorHandle);

	}

	else if(sgShader.GetCurrentShaderType() == SHADER_TYPE::TEXTURE)
	{
		// Bind buffer 2: Texture Coordinate
		glBindBuffer(GL_ARRAY_BUFFER, pVertexList->mpVtxBuffer->mBufferID[BUFFER_TYPE::TEXTURE_COORDINATE]);
		s32 texCoordHandle = sgShader.GetAttributeLocation("aTextureCoordinate");
		glEnableVertexAttribArray(texCoordHandle);
		glVertexAttribPointer(texCoordHandle, 
								COORDS_PER_TEXTURE_COORDINATE,
								GL_FLOAT,
								false,
								0, 
								0);

		// Draw the triangles
		glDrawArrays(drawType, 0, pVertexList->vtxNum);
		glDisableVertexAttribArray(texCoordHandle);
	}

	glDisableVertexAttribArray(positionHandle);

	// Unbind the buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);

}

// ---------------------------------------------------------------------------

AE_API void AEGfxMeshFree(AEGfxVertexList* pVertexList)
{
	glDeleteBuffers(BUFFER_TYPE::NUM, pVertexList->mpVtxBuffer->mBufferID);

	delete pVertexList->mpVtxBuffer;
	delete pVertexList;
}

//
// NOTE(gerald): These are almost never used by users and lecturers.
// I understand what these functions are trying to do, it should be consistent
// with the mesh building API. Otherwise it is too confusing for users
// to try to figure out how to use them properly.
//
// ---------------------------------------------------------------------------
#if 0
AE_API void AEGfxPoint(f32 x0, f32 y0, f32 z0, u32 c0)
{
	sTriBuffer[0].x = x0;	sTriBuffer[0].y = y0;	sTriBuffer[0].z = z0;	sTriBuffer[0].c = c0;
}

// ---------------------------------------------------------------------------


AE_API void	AEGfxLine(f32 x0, f32 y0, f32 z0, f32 r0, f32 g0, f32 b0, f32 a0, f32 x1, f32 y1, f32 z1, f32 r1, f32 g1, f32 b1, f32 a1)
{	
	GLfloat *pPositions = new GLfloat[2 * COORDS_PER_POSITION];
	GLfloat *pColors = new GLfloat[2 * COORDS_PER_COLOR];

	pPositions[0] = x0;		pPositions[1] = y0;		pPositions[2] = z0;
	pPositions[3] = x1;		pPositions[4] = y1;		pPositions[5] = z1;

	pColors[0] = r0;		pColors[1] = g0;		pColors[2] = b0;		pColors[3] = a0;
	pColors[4] = r1;		pColors[5] = g1;		pColors[6] = b1;		pColors[7] = a1;
	
	// Bind buffer 0: Position
	s32 positionHandle = sgShader.GetAttributeLocation("aPosition");
	glEnableVertexAttribArray(positionHandle);
	glVertexAttribPointer(positionHandle, 
							COORDS_PER_POSITION,
							GL_FLOAT,
							false,
							0, 
							pPositions);

	if(sgShader.GetCurrentShaderType() == SHADER_TYPE::COLOR)
	{
		// Bind buffer 1: Color
		//glBindBuffer(GL_ARRAY_BUFFER, pTriList->mpVtxBuffer->mBufferID[BUFFER_TYPE::COLOR]);
		s32 colorHandle = sgShader.GetAttributeLocation("aColor");
		glEnableVertexAttribArray(colorHandle);
		glVertexAttribPointer(colorHandle, 
								COORDS_PER_COLOR,
								GL_FLOAT,
								false,
								0, 
								pColors);

		
		// Draw the triangles
		glDrawArrays(GL_LINES, 0, 2);
		glDisableVertexAttribArray(colorHandle);
	}

	glDisableVertexAttribArray(positionHandle);

	delete [] pPositions;
	delete [] pColors;

	// Unbind the buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// ---------------------------------------------------------------------------

AE_API void AEGfxTri(f32 x0, f32 y0, f32 z0, u32 c0, f32 x1, f32 y1, f32 z1, u32 c1, f32 x2, f32 y2, f32 z2, u32 c2)
{
	sTriBuffer[0].x = x0;	sTriBuffer[0].y = y0;	sTriBuffer[0].z = z0;	sTriBuffer[0].c = c0;
	sTriBuffer[1].x = x1;	sTriBuffer[1].y = y1;	sTriBuffer[1].z = z1;	sTriBuffer[1].c = c1;
	sTriBuffer[2].x = x2;	sTriBuffer[2].y = y2;	sTriBuffer[2].z = z2;	sTriBuffer[2].c = c2;
}

// ---------------------------------------------------------------------------

AE_API void AEGfxQuad(f32 x0, f32 y0, f32 z0, u32 c0, f32 x1, f32 y1, f32 z1, u32 c1,
	f32 x2, f32 y2, f32 z2, u32 c2, f32 x3, f32 y3, f32 z3, u32 c3)
{
	sTriBuffer[0].x = x0;	sTriBuffer[0].y = y0;	sTriBuffer[0].z = z0;	sTriBuffer[0].c = c0;
	sTriBuffer[1].x = x1;	sTriBuffer[1].y = y1;	sTriBuffer[1].z = z1;	sTriBuffer[1].c = c1;
	sTriBuffer[2].x = x3;	sTriBuffer[2].y = y3;	sTriBuffer[2].z = z3;	sTriBuffer[2].c = c3;
	sTriBuffer[3].x = x2;	sTriBuffer[3].y = y2;	sTriBuffer[3].z = z2;	sTriBuffer[3].c = c2;
}

// ---------------------------------------------------------------------------

AE_API void AEGfxBox(f32 x0, f32 y0, f32 z0, f32 sizeX,  f32 sizeY, f32 sizeZ, u32 c0, u32 c1)
{
	sizeX *= 0.5f;
	sizeY *= 0.5f;
	sizeZ *= 0.5f;

	sTriBuffer[0].x = x0 - sizeX;	sTriBuffer[0].y = y0 - sizeY;	sTriBuffer[0].z = z0 - sizeZ;	sTriBuffer[0].c = c1;
	sTriBuffer[1].x = x0 - sizeX;	sTriBuffer[1].y = y0 - sizeY;	sTriBuffer[1].z = z0 + sizeZ;	sTriBuffer[1].c = c1;
	sTriBuffer[2].x = x0 - sizeX;	sTriBuffer[2].y = y0 + sizeY;	sTriBuffer[2].z = z0 + sizeZ;	sTriBuffer[2].c = c0;
	sTriBuffer[3].x = x0 - sizeX;	sTriBuffer[3].y = y0 + sizeY;	sTriBuffer[3].z = z0 - sizeZ;	sTriBuffer[3].c = c0;
	sTriBuffer[4].x = x0 + sizeX;	sTriBuffer[4].y = y0 + sizeY;	sTriBuffer[4].z = z0 - sizeZ;	sTriBuffer[4].c = c0;
	sTriBuffer[5].x = x0 + sizeX;	sTriBuffer[5].y = y0 - sizeY;	sTriBuffer[5].z = z0 - sizeZ;	sTriBuffer[5].c = c1;
	sTriBuffer[6].x = x0 + sizeX;	sTriBuffer[6].y = y0 - sizeY;	sTriBuffer[6].z = z0 + sizeZ;	sTriBuffer[6].c = c1;
	sTriBuffer[7].x = x0 - sizeX;	sTriBuffer[7].y = y0 - sizeY;	sTriBuffer[7].z = z0 + sizeZ;	sTriBuffer[7].c = c1;

	sTriBuffer[0].x = x0 + sizeX;	sTriBuffer[0].y = y0 + sizeY;	sTriBuffer[0].z = z0 + sizeZ;	sTriBuffer[0].c = c0;
	sTriBuffer[1].x = x0 - sizeX;	sTriBuffer[1].y = y0 - sizeY;	sTriBuffer[1].z = z0 + sizeZ;	sTriBuffer[1].c = c1;
	sTriBuffer[2].x = x0 + sizeX;	sTriBuffer[2].y = y0 - sizeY;	sTriBuffer[2].z = z0 + sizeZ;	sTriBuffer[2].c = c1;
	sTriBuffer[3].x = x0 + sizeX;	sTriBuffer[3].y = y0 - sizeY;	sTriBuffer[3].z = z0 - sizeZ;	sTriBuffer[3].c = c1;
	sTriBuffer[4].x = x0 + sizeX;	sTriBuffer[4].y = y0 + sizeY;	sTriBuffer[4].z = z0 - sizeZ;	sTriBuffer[4].c = c0;
	sTriBuffer[5].x = x0 - sizeX;	sTriBuffer[5].y = y0 + sizeY;	sTriBuffer[5].z = z0 - sizeZ;	sTriBuffer[5].c = c0;
	sTriBuffer[6].x = x0 - sizeX;	sTriBuffer[6].y = y0 + sizeY;	sTriBuffer[6].z = z0 + sizeZ;	sTriBuffer[6].c = c0;
	sTriBuffer[7].x = x0 - sizeX;	sTriBuffer[7].y = y0 - sizeY;	sTriBuffer[7].z = z0 + sizeZ;	sTriBuffer[7].c = c1;
}

// ---------------------------------------------------------------------------

AE_API void AEGfxSphere(f32 x0, f32 y0, f32 z0, f32 radius, u32 c0, u32 c1, u32 division)
{
	u32 triCtr = 0;

	// clamp the division value
	if (division < 4)
		division = 4;
	else if (division > 32)
		division = 32;

	s32 betaStart = -(s32)(division) / 4, 
		betaEnd   =  division / 4;
	f32 betaMult  = PI / (betaEnd - betaStart);
	s32	alphaEnd  = division;
	f32 alphaMult = TWO_PI / division;

	for (s32 beta = betaStart; beta < betaEnd; beta++)
	{
		//u32 triIdx = triCtr;

		for (s32 alpha = 0; alpha <= alphaEnd; alpha++)
		{
			//AE_ASSERT_MESG(triCtr < AE_GFX_TRI_NUM_MAX * 3, "vertex buffer overflow!!");
			sTriBuffer[triCtr].x = x0 + radius * cosf((beta + 1) * betaMult) * sinf(alpha * alphaMult);
			sTriBuffer[triCtr].y = y0 + radius * sinf((beta + 1) * betaMult);
			sTriBuffer[triCtr].z = z0 + radius * cosf((beta + 1) * betaMult) * cosf(alpha * alphaMult);
			sTriBuffer[triCtr].c = AEGfxColInterp(c0, c1, fabs((f32)(beta + 1) / betaEnd));
			triCtr++;

			//AE_ASSERT_MESG(triCtr < AE_GFX_TRI_NUM_MAX * 3, "vertex buffer overflow!!");
			sTriBuffer[triCtr].x = x0 + radius * cosf(beta * betaMult) * sinf(alpha * alphaMult);
			sTriBuffer[triCtr].y = y0 + radius * sinf(beta * betaMult);
			sTriBuffer[triCtr].z = z0 + radius * cosf(beta * betaMult) * cosf(alpha * alphaMult);
			sTriBuffer[triCtr].c = AEGfxColInterp(c0, c1, fabs((f32)(beta) / betaEnd));
			triCtr++;
		}
	}
}

// ---------------------------------------------------------------------------

AE_API void AEGfxAxis(f32 scale)
{
	AEGfxSphere	(0.0f, 0.0f, 0.0f, scale * 0.1f, 0xFF808080, 0xFFFFFFFF, 8);
	AEGfxLine	(0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, scale * 0.8f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
	AEGfxLine	(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, scale * 0.8f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	AEGfxLine	(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, scale * 0.8f, 0.0f, 0.0f, 1.0f, 1.0f);
}

#endif
// ---------------------------------------------------------------------------

AE_API u32 AEGfxColInterp(u32 c0, u32 c1, f32 t)
{
	s32 a0 = (c0 >> 24) & 0x00FF, 
		r0 = (c0 >> 16) & 0x00FF, 
		g0 = (c0 >>  8) & 0x00FF, 
		b0 = (c0      ) & 0x00FF, 
		a1 = (c1 >> 24) & 0x00FF, 
		r1 = (c1 >> 16) & 0x00FF, 
		g1 = (c1 >>  8) & 0x00FF, 
		b1 = (c1      ) & 0x00FF, 
		a2, r2, g2, b2;

	t  = AEClamp(t, 0.0f, 1.0f);

	a2 = a0 + (a1 - a0) * (s32)t;
	r2 = r0 + (r1 - r0) * (s32)t;
	g2 = g0 + (g1 - g0) * (s32)t;
	b2 = b0 + (b1 - b0) * (s32)t;

	a2 = (a2 < 0) ? (0) : ((a2 > 255) ? (255) : (a2));
	r2 = (r2 < 0) ? (0) : ((r2 > 255) ? (255) : (r2));
	g2 = (g2 < 0) ? (0) : ((g2 > 255) ? (255) : (g2));
	b2 = (b2 < 0) ? (0) : ((b2 > 255) ? (255) : (b2));

	return ((a2 << 24) | (r2 << 16) | (g2 << 8) | b2);
}

// ---------------------------------------------------------------------------

AE_API void				AEGfxTextureInitialize	(AEGfxTexture *pTexture)
{
	pTexture->mpSurface = new AEGfxSurface();
}

// ---------------------------------------------------------------------------

AE_API AEGfxTexture*	AEGfxTextureLoad(const char* pFileName)
{
	// File name size limit
	if(strlen(pFileName) > 255)
		return NULL;

	AEGfxTexture *pTexture = nullptr;

#if !defined(__EMSCRIPTEN__)
	// ----- Native: decode with GDI+ -----------------------------------------
	// Load the file
	unsigned fileNameSize = (unsigned)(strlen(pFileName));
	wchar_t *pFileNameWCHAR = new wchar_t[fileNameSize + 1];

	for (u32 k = 0; k < fileNameSize ; ++k)
		pFileNameWCHAR[k] = pFileName[k];

	pFileNameWCHAR[fileNameSize] = 0;		// last character

	Gdiplus::Bitmap bmp(pFileNameWCHAR);
	delete [] pFileNameWCHAR;

	if (bmp.GetLastStatus() == Gdiplus::Ok)
	{
		// Allocate memory
		pTexture = new AEGfxTexture();
		AEGfxTextureInitialize(pTexture);

		memset(pTexture->mpName, 0, 256 * sizeof(s8));
		memcpy(pTexture->mpName, pFileName, strlen(pFileName) + 1);

		Gdiplus::Rect rect(0, 0, bmp.GetWidth(), bmp.GetHeight());

		Gdiplus::BitmapData data;
		bmp.LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &data);

		// Allocate texture
		glGenTextures(1, &pTexture->mpSurface->mBufferID);

		//glPixelStorei(GL_UNPACK_ROW_LENGTH, data.Width);
		//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		// Bind created texture
		glBindTexture(GL_TEXTURE_2D, pTexture->mpSurface->mBufferID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		// Load the data
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, data.Width, data.Height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, data.Scan0);

		// Unlock the bits
		bmp.UnlockBits(&data);

		// Unbind the texture
		glBindTexture(GL_TEXTURE_2D, 0);
	}
#else
	// ----- Web: decode with SDL_image, upload as plain RGBA (WebGL/GLES2) ----
	// GLES2 has no GL_RGBA8 internal format and no GL_BGRA_EXT, so we convert
	// to a tightly-packed RGBA byte layout and use GL_RGBA for both args.
	SDL_Surface* loaded = IMG_Load(pFileName);
	if (loaded)
	{
		SDL_Surface* rgba = SDL_ConvertSurfaceFormat(loaded, SDL_PIXELFORMAT_ABGR8888, 0);
		SDL_FreeSurface(loaded);

		if (rgba)
		{
			pTexture = new AEGfxTexture();
			AEGfxTextureInitialize(pTexture);

			memset(pTexture->mpName, 0, 256 * sizeof(s8));
			memcpy(pTexture->mpName, pFileName, strlen(pFileName) + 1);

			glGenTextures(1, &pTexture->mpSurface->mBufferID);
			glBindTexture(GL_TEXTURE_2D, pTexture->mpSurface->mBufferID);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rgba->w, rgba->h, 0,
				GL_RGBA, GL_UNSIGNED_BYTE, rgba->pixels);

			glBindTexture(GL_TEXTURE_2D, 0);

			SDL_FreeSurface(rgba);
		}
	}
#endif

	return pTexture;
}

// ---------------------------------------------------------------------------

AE_API void AEGfxTextureSet(AEGfxTexture *pTexture, f32 offset_x, f32 offset_y)			// Use NULL for 'no texture'
{
	if(pTexture)
	{
		s32 texSamplerHandler = sgShader.GetUniformLocation("uTexture");
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pTexture->mpSurface->mBufferID);
		glUniform1i(texSamplerHandler, 0);
		CHECK_GL_ERROR();

		while(offset_x > 1.0f)
			offset_x -= 1.0f;

		while(offset_y > 1.0f)
			offset_y -= 1.0f;

		s32 texOffsetHandler = sgShader.GetUniformLocation("uTextureOffset");
		glUniform2f(texOffsetHandler, offset_x, offset_y);
		CHECK_GL_ERROR();
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

// ---------------------------------------------------------------------------

AE_API void AEGfxTextureUnload(AEGfxTexture *pTexture)
{
	if(pTexture)
	{
		glDeleteTextures(1, &pTexture->mpSurface->mBufferID);
	}

	delete pTexture->mpSurface;
	delete pTexture;
	pTexture = NULL;
}

// ---------------------------------------------------------------------------

AE_API AEGfxTexture*	AEGfxTextureLoadFromMemory(u8 *pColors, u32 Width, u32 Height)
{
	AEGfxTexture *pTexture = new AEGfxTexture();
	AEGfxTextureInitialize(pTexture);

	// Allocate texture
	glGenTextures(1, &pTexture->mpSurface->mBufferID);

	//glPixelStorei(GL_UNPACK_ROW_LENGTH, Width);
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	// Bind created texture
	glBindTexture(GL_TEXTURE_2D, pTexture->mpSurface->mBufferID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Load the data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pColors);

	
	// Unbind the texture
	glBindTexture(GL_TEXTURE_2D, 0);

	return pTexture;
}


// ---------------------------------------------------------------------------

AE_API void AEGfxSaveTextureToFile(AEGfxTexture* pTexture, s8 *pFileName)
{
	if(NULL == pTexture || NULL == pFileName)
		return;
	
	PRINT("AEGfxSaveTextureToFile not supported in the OpenGL version of the Alpha Engine!\n");
}

// ---------------------------------------------------------------------------
AE_API void	AEGfxSetTextureMode(AEGfxTextureMode TextureMode)
{	

	// Filtering
	switch(TextureMode)
	{
	case AE_GFX_TM_PRECISE:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		break;

	case AE_GFX_TM_AVERAGE:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;

	default:
		PRINT("Unsupported texture mode!\n");
	}
}

// ---------------------------------------------------------------------------

AE_API void AEGfxSetCamPosition(f32 X, f32 Y)
{
	gWIN_CAM_POS_X = X;
	gWIN_CAM_POS_Y = Y;

	AEGfxFixWindowMaxMin();
}

// ---------------------------------------------------------------------------

AE_API void AEGfxGetCamPosition(f32 *pX, f32 *pY)
{
	*pX = gWIN_CAM_POS_X;
	*pY = gWIN_CAM_POS_Y;
}

// ---------------------------------------------------------------------------


void SetProjectionAndViewMatrices(void)
{
	// load the projection matrix
	/*
	glMatrixMode		(GL_PROJECTION);
    glLoadIdentity		();
	gluPerspective		(WIN_FOV, (float)(gWIN_WIDTH)/(float)(gWIN_HEIGHT), WIN_CAM_NEAR_CLIP, WIN_CAM_FAR_CLIP);
	
	
	float m[16];
	memset(m, 0, 16*sizeof(float));
	glGetFloatv(GL_PROJECTION_MATRIX, (GLfloat*)(m));
	*/


	int updatedWidthVP = (int)((f32)AEGfxGetWindowWidth() * gResolutionRatioX);
	int updatedHeightVP = (int)((f32)AEGfxGetWindowHeight() * gResolutionRatioY);
	int vpositionX = 0; 
	int vpositionY = 0;
	//float ratioX = 1.0f;
	//float ratioY = 1.0f;

	if(AESysIsFullScreen())
	{
		////Get the screen resolution
		//int width = GetSystemMetrics(SM_CXSCREEN);
		//int height = GetSystemMetrics(SM_CYSCREEN);

		//ratioX = ((float)((float)width / (float)GetWindowWidth()));
		//ratioY = ((float)((float)height / (float)GetWindowHeight()));

		////multiply the width and the height of the current viewport by the ration between the window size and the screen size
		//updatedWidthVP = (int)((float)GetWindowWidth() * ratioX);
		//updatedHeightVP = (int)((float)GetWindowHeight() * ratioY);
		
		//save the ratio and use it with the viewport position also (I GUESS)

		vpositionX = (int)((f32)(vpositionX) * gResolutionRatioX);
		vpositionY = (int)((f32)(vpositionY) * gResolutionRatioY);
	}


	/*
	float cot = 1.0f / tanf(WIN_FOV / 360.0f * PI);
	float ar = (float)(gWIN_WIDTH)/(float)(gWIN_HEIGHT);
	mProjectionMatrix[0] = cot / ar;
    mProjectionMatrix[1] = 0.0f;
    mProjectionMatrix[2] = 0.0f;
    mProjectionMatrix[3] = 0.0f;
    mProjectionMatrix[4] = 0.0f;
    mProjectionMatrix[5] = cot;
    mProjectionMatrix[6] = 0.0f;
    mProjectionMatrix[7] = 0.0f;
    mProjectionMatrix[8] = 0.0f;
    mProjectionMatrix[9] = 0.0f;
    mProjectionMatrix[10] = (-WIN_CAM_FAR_CLIP - WIN_CAM_NEAR_CLIP) / (WIN_CAM_FAR_CLIP - WIN_CAM_NEAR_CLIP);
    mProjectionMatrix[11] = -1.0f;
    mProjectionMatrix[12] = 0.0f;
    mProjectionMatrix[13] = 0.0f;
    mProjectionMatrix[14] = (-2.0f * WIN_CAM_FAR_CLIP * WIN_CAM_NEAR_CLIP) / (WIN_CAM_FAR_CLIP - WIN_CAM_NEAR_CLIP);
    mProjectionMatrix[15] = 0.0f;
	*/
	//Matrix4 projectionMatrix = MtxTranspose(MtxPerspectiveProjection(WIN_FOV, (float)(gWIN_WIDTH)/(float)(gWIN_HEIGHT),  WIN_CAM_NEAR_CLIP, WIN_CAM_FAR_CLIP));
	/*Matrix4 projectionMatrix = MtxTranspose(MtxOrthogonalProjection((float)(gWIN_WIDTH), (float)(gWIN_HEIGHT),  WIN_CAM_NEAR_CLIP, WIN_CAM_FAR_CLIP));
	s32 mProjMatrixHandle = sgShader.GetUniformLocation("uPMatrix");
	glUniformMatrix4fv(mProjMatrixHandle, 1, 0, projectionMatrix.v);*/

	
	Matrix4 projectionMatrix = MtxTranspose(MtxOrthogonalProjection((float)(updatedWidthVP), (float)(updatedHeightVP),  WIN_CAM_NEAR_CLIP, WIN_CAM_FAR_CLIP));
	int mProjMatrixHandle = sgShader.GetUniformLocation("uPMatrix");
	glUniformMatrix4fv(mProjMatrixHandle, 1, 0, projectionMatrix.v);

	
	glViewport((GLint)vpositionX, (GLint)vpositionY, updatedWidthVP, updatedHeightVP);
	
	// check for OpenGL error
	CHECK_GL_ERROR();

	// load the model -> view matrix
	//glMatrixMode		(GL_MODELVIEW);
	//glLoadIdentity		();

	//sCamera.LoadMatrix();

	//printf("FIX LOOK AT\n");
	// load the camera matrix onto the stack
	
	
	//gluLookAt(gWIN_CAM_POS_X, gWIN_CAM_POS_Y, gWIN_CAM_DIST, gWIN_CAM_POS_X, gWIN_CAM_POS_Y, 0.0f, 0.0f, 1.0f, 0.0f);




	// get the top matrix in the stack
	// * it's supposed to be the world-to-view space transformation matrix
	//glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat*)(mViewMatrix));
	Matrix4 viewMatrix = MtxTranspose(MtxLookAt(Point4(gWIN_CAM_POS_X, gWIN_CAM_POS_Y, gWIN_CAM_DIST),
														Point4(gWIN_CAM_POS_X, gWIN_CAM_POS_Y, 0.0f),
														Vector4(0.0f, 1.0f, 0.0f)));


	//s32 mViewMatrixHandle = sgShader.GetUniformLocation("uVMatrix");
	//glUniformMatrix4fv(mViewMatrixHandle, 1, 0, viewMatrix.v);

	
	Matrix4 sc;
	sc.Identity();
	sc.m00 = gResolutionRatioX;
	sc.m11 = gResolutionRatioY;

	viewMatrix *= sc;

	int mViewMatrixHandle = sgShader.GetUniformLocation("uVMatrix");
	glUniformMatrix4fv(mViewMatrixHandle, 1, 0, viewMatrix.v);


	//glBegin(GL_TRIANGLES);

	//	glColor3f(1, 0, 0);
	//	glVertex3f(0, 0, 0);
	//	glVertex3f(1, 0, 0);
	//	glVertex3f(0, 1, 0);
	//glEnd();

	// check for OpenGL error
	CHECK_GL_ERROR();
}

// ---------------------------------------------------------------------------

#pragma region Font

// ---------------------------------------------------------------------------

#define MAX_ATLAS_SIZE 1024

#include <ft2build.h>
#include FT_FREETYPE_H

static FT_Library		gFreetypeLibrary = nullptr;
static GLuint			gVBOFont;

// ---------------------------------------------------------------------------

u32 AEMaxInt(u32 x, u32 y)
{
	return (x < y) ? y : x;
}

// ---------------------------------------------------------------------------

struct AEGlyphAtlas
{
	GLuint AtlasTexture;
	u32 Width = 0;
	u32 Height = 0;

	struct GlyphInfo
	{
		f32 ax;	// advance.x
		f32 ay;	// advance.y

		f32 bw;	// bitmap.width;
		f32 bh;	// bitmap.height;

		f32 bl;	// bitmap_left;
		f32 bt;	// bitmap_top;

		f32 tx;	// x offset of glyph in texture coordinates
		f32 ty;	// y offset of glyph in texture coordinates
	} glyph[128];		// character information

	AEGlyphAtlas(FT_Face face, int sizeF)
	{
		FT_Set_Pixel_Sizes(face, 0, sizeF);
		FT_GlyphSlot g = face->glyph;

		u32 row_width = 0;
		u32 row_height = 0;
		this->Width = 0;
		this->Height = 0;
		const u32 padding = 2;

		memset(glyph, 0, sizeof glyph);

		/* Find minimum size for a texture holding all visible ASCII characters */
		for (u8 i = 32; i < 128; i++) 
		{
			if (FT_Load_Char(face, i, FT_LOAD_RENDER)) 
			{
				PRINT("\n - Could not generate a glyph for %c\n", i);
			 	// fprintf(stderr, "Loading character %c failed!\n", i);
				continue;
			}
			u32 glyph_width_with_padding = g->bitmap.width + (padding * 2);
			u32 glyph_height_with_padding = g->bitmap.rows + (padding * 2);
			if (row_width + glyph_width_with_padding >= MAX_ATLAS_SIZE)
			{
				this->Width = AEMaxInt(this->Width, row_width);
				this->Height += row_height;
				row_width = 0;
				row_height = 0;
			}
			row_width += glyph_width_with_padding;
			row_height = AEMaxInt(row_height, glyph_height_with_padding);
		}

		this->Width = AEMaxInt(Width, row_width);
		this->Height += row_height;

		CHECK_GL_ERROR();
		// Create a texture that will be used to hold all ASCII
		glActiveTexture(GL_TEXTURE0);
		glGenTextures(1, &AtlasTexture);
		//s32 textureHandler = sgShader.GetUniformLocation("uTexture");
		glBindTexture(GL_TEXTURE_2D, AtlasTexture);
		//glUniform1i(textureHandlertrans, 0);		
		CHECK_GL_ERROR();


		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, Width, Height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, 0);

		// We require 1 byte alignment when uploading texture data
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		// Clamping to edges is important to prevent artifacts when scaling
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// Linear filtering usually looks best for text
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		/* Paste all glyph bitmaps into the texture, remembering the offset */
		int ox = 0;
		int oy = 0;

		row_height = 0;

		for (u8 i = 32; i < 128; i++) 
		{
			if (FT_Load_Char(face, i, FT_LOAD_RENDER)) 
			{
				//fprintf(stderr, "Loading character %c failed!\n", i);
				continue;
			}

			u32 glyph_width_with_padding = g->bitmap.width + (padding * 2);
			u32 glyph_height_with_padding = g->bitmap.rows + (padding * 2);

			if (ox + glyph_width_with_padding >= MAX_ATLAS_SIZE)
			{
				oy += row_height;
				row_height = 0;
				ox = 0;
			}

			// Skip glyphs with no bitmap (e.g. space): WebGL rejects glTexSubImage2D
		// with a NULL pixel pointer / zero size (INVALID_VALUE: "no pixels").
		// The advance/metrics below are still recorded so spacing stays correct.
		if (g->bitmap.buffer != nullptr && g->bitmap.width > 0 && g->bitmap.rows > 0)
		{
			glTexSubImage2D(GL_TEXTURE_2D, 0, ox, oy, g->bitmap.width, g->bitmap.rows, GL_ALPHA, GL_UNSIGNED_BYTE, g->bitmap.buffer);
		}

			glyph[i].ax = (f32)(g->advance.x >> 6); // advance width
			glyph[i].ay = (f32)(g->advance.y >> 6); // advance height

			glyph[i].bw = (f32)g->bitmap.width;     // glyph width
			glyph[i].bh = (f32)g->bitmap.rows;      // glyph height

			glyph[i].bl = (f32)g->bitmap_left;
			glyph[i].bt = (f32)g->bitmap_top;

			glyph[i].tx = ox / (f32)Width;          // texture u
			glyph[i].ty = oy / (f32)Height;         // texture v

			row_height = AEMaxInt(row_height, glyph_height_with_padding);
			ox += glyph_width_with_padding;
		}
	}

	~AEGlyphAtlas()
	{
		glDeleteTextures(1, &AtlasTexture);
	}
};

// ---------------------------------------------------------------------------

class AEFont
{
public:
	s8	FontID = -1;

	static AEFont* Add(const char* fontName, u32 fontHeight)
	{
		FT_Face face;
		
		// Loading the TypeFace
		if (FT_New_Face(gFreetypeLibrary, fontName, 0, &face))
		{
			PRINT("Error: Could not load font %s\n", fontName);
			return nullptr;
		}
		
		PRINT("Font %s, size %d, ", fontName, fontHeight);
		
		AEFont* newFont = new AEFont(face);

		if (m_pFontsRoot == nullptr)
		{
			m_pFontsRoot = newFont;
		}
		else
		{
			AEFont* last = GetLast();
			last->NextFont = newFont;
			newFont->PreviousFont = last;			
		}

		// Add an Atlas		
		newFont->mAtlas = new AEGlyphAtlas(face, fontHeight);
		PRINT("generated a %d x %d (%d kb) atlas.\n", newFont->mAtlas->Width, newFont->mAtlas->Height, newFont->mAtlas->Width * newFont->mAtlas->Height / 1024);

		return newFont;
	}

	static void Remove(AEFont* font)
	{
		if (font == nullptr)
			return;
		
		AEFont* prev = font->PreviousFont;
		AEFont* next = font->NextFont;

		if(prev != nullptr)	prev->NextFont		= next;		
		if(next != nullptr) next->PreviousFont	= prev;

		if (font == m_pFontsRoot)
			m_pFontsRoot = next;

		delete font;
	}
	
	static AEFont* GetRoot()
	{
		return m_pFontsRoot;
	}

	static AEFont* GetLast()
	{
		if (m_pFontsRoot == nullptr)
			return nullptr;
		
		AEFont* it = m_pFontsRoot;
		while (it->NextFont != nullptr)
		{
			it = it->NextFont;
		}
		
		return it;
	}
	
	static AEFont* GetFont(s8 fontID)
	{
		AEFont* pFont = GetRoot();
		while (pFont != nullptr)
		{
			if (pFont->FontID == fontID)
				return pFont;

			pFont = pFont->NextFont;
		}

		return nullptr;
	}

	static void CleanUp()
	{
		AEFont* pFont = GetRoot();
		while(pFont!=nullptr)
		{
			Remove(pFont);
			pFont = GetRoot();
		}
	}

	FT_Face GetFace() const
	{
		return mFreetypeFace;
	}

	AEGlyphAtlas* GetGlyphAtlas() const
	{
		return mAtlas;
	}

protected:
	AEFont(FT_Face face)
	:FontID(m_NextID)
	,mFreetypeFace(face)
	,NextFont(nullptr)
	,PreviousFont(nullptr)
	{
		FontID = m_NextID;
		m_NextID++;
	}

	~AEFont()
	{
		if (mFreetypeFace != nullptr)
		{
			FT_Done_Face(mFreetypeFace);
			mFreetypeFace = nullptr;
		}
		
		if (mAtlas != nullptr)
		{
			delete mAtlas;
			mAtlas = nullptr;
		}
	}
	
private:
	static AEFont*	m_pFontsRoot;
	static s8		m_NextID;

	FT_Face			mFreetypeFace = nullptr;
	AEGlyphAtlas*	mAtlas			= nullptr;
	AEFont*			NextFont		= nullptr;
	AEFont*			PreviousFont	= nullptr;
};

s8		AEFont::m_NextID = 1;
AEFont* AEFont::m_pFontsRoot = nullptr;

// ---------------------------------------------------------------------------

AE_API s8 AEGfxCreateFont(const char* fontName, int sizeF)
{
	AEFont* pNewFont = AEFont::Add(fontName, sizeF);

	if (pNewFont == nullptr)
		return -1;
	return pNewFont->FontID;
}

// ---------------------------------------------------------------------------

AE_API void AEGfxDestroyFont(s8 fontId)
{
	AEFont* pFont = AEFont::GetFont(fontId);

	if (pFont == nullptr)
	{
		PRINT("Error: Trying to destroy a font with an unknown fontId (%d)\n", fontId);
		return;
	}
	
	AEFont::Remove(pFont);
}


AE_API void AEGfxPrint(s8 fontId, const char* pStr, f32 screenX, f32 screenY, f32 scale, f32 red, f32 green, f32 blue, f32 alpha)
{
	AEFont* pFont = AEFont::GetFont(fontId);
	if(pFont == nullptr)
	{
		PRINT("Error: Cannot print msg (%s). FontId (%d) coud not be found\n", pStr, fontId);
		return;
	}
	
	// Enable the Glyph Shader
	u32 oldShader = sgShader.GetCurrentShaderType();
	sgShader.ApplyShader(SHADER_TYPE::FONT);

    AEGfxBlendMode oldBlendMode = gCurrentBlendMode;
	AEGfxSetBlendMode(AE_GFX_BM_BLEND);

	// Binding the TextColor
	s32 textColorHandler = sgShader.GetUniformLocation("uFontColor");
	glUniform4f(textColorHandler, red, green, blue, alpha);

	// Binding the Atlas Texture
	s32 textureHandler = sgShader.GetUniformLocation("uTexture");
	glBindTexture(GL_TEXTURE_2D, pFont->GetGlyphAtlas()->AtlasTexture);
	glUniform1i(textureHandler, 0);


	// Bind buffer 0: Position
	s32 CoordsHandle = sgShader.GetAttributeLocation("aCoords");
	glEnableVertexAttribArray(CoordsHandle);
	glBindBuffer(GL_ARRAY_BUFFER, gVBOFont);
	glVertexAttribPointer(CoordsHandle, 4, GL_FLOAT, GL_FALSE, 0, 0);

	// Setting up Overlay scaling calculation
	f32 sx = 2.0f / AEGfxGetWindowWidth() * scale;
	f32 sy = 2.0f / AEGfxGetWindowHeight() * scale;
	//f32 fx = screenX * sx - 1.f;
	//f32 fy = screenY * sy - 1.f;
	f32 fx = screenX;
	f32 fy = screenY;
	
	size_t msgLen = strlen(pStr);
	Vector4* coords = (Vector4*) calloc(6 * msgLen, sizeof(Vector4));
	if (!coords) {
		PRINT("Error: Cannot print msg (%s). Memory allocation failed\n", pStr);
		return;
	}
	const size_t coordsSize = 6 * msgLen * sizeof(Vector4);
	u32 c = 0;

	AEGlyphAtlas::GlyphInfo* glyph = nullptr;
	s32 ah = pFont->GetGlyphAtlas()->Height;
	s32 aw = pFont->GetGlyphAtlas()->Width;

	for (const char* p = pStr; *p; p++)
	{
		glyph = &(pFont->GetGlyphAtlas()->glyph[*p]);
		
		// Calculate the vertex and texture coordinates
		f32 x2 =  fx + glyph->bl * sx;
		f32 y2 = -fy - glyph->bt * sy;
		f32 w = glyph->bw * sx;
		f32 h = glyph->bh * sy;

		// Advance the cursor to the start of the next character
		fx += glyph->ax * sx;
		fy += glyph->ay * sy;

		// Skip glyphs that have no pixels
		if (!w || !h)
			continue;

		coords[c++] = {x2,		-y2,		glyph->tx,					glyph->ty };
		coords[c++] = {x2 + w,	-y2,		glyph->tx + glyph->bw / aw,	glyph->ty };
		coords[c++] = {x2,		-y2 - h,	glyph->tx,					glyph->ty + glyph->bh / ah };
		coords[c++] = {x2 + w,	-y2,		glyph->tx + glyph->bw / aw,	glyph->ty };
		coords[c++] = {x2,		-y2 - h,	glyph->tx,					glyph->ty + glyph->bh / ah };
		coords[c++] = {x2 + w,	-y2 - h,	glyph->tx + glyph->bw / aw,	glyph->ty + glyph->bh / ah };
	}

	/* Draw all the character on the screen in one go */
	glBufferData(GL_ARRAY_BUFFER, coordsSize, coords, GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, c);

	free(coords);
	
	glDisableVertexAttribArray(CoordsHandle);

    AEGfxSetBlendMode(oldBlendMode);
	
  //
  // NOTE(momo): 
  //
	// Using the Render Mode Method, so it can reapply the matrices
	switch (oldShader)
	{
		case SHADER_TYPE::NONE:
			AEGfxSetRenderMode(AEGfxRenderMode::AE_GFX_RM_NONE);
			break;

		case SHADER_TYPE::COLOR:
			AEGfxSetRenderMode(AEGfxRenderMode::AE_GFX_RM_COLOR);
			break;

		case SHADER_TYPE::TEXTURE:
			AEGfxSetRenderMode(AEGfxRenderMode::AE_GFX_RM_TEXTURE);
			break;

		// Just in case
		default:
			sgShader.ApplyShader(oldShader);
	}
}

// ---------------------------------------------------------------------------

AE_API void AEGfxGetPrintSize(s8 fontId, const char* pStr, f32 scale, f32 *out_width, f32 *out_height)
{
	f32 width = 0;
	f32 height = 0;
	
	AEFont* pFont = AEFont::GetFont(fontId);
	if (pFont == nullptr)
	{
		PRINT("Error: Cannot get size of msg (%s). FontId (%d) coud not be found\n", pStr, fontId);
		return;
	}
	
	// Setting up Overlay scaling calculation
	f32 sx = 2.0f / AEGfxGetWindowWidth() * scale;
	f32 sy = 2.0f / AEGfxGetWindowHeight() * scale;

	AEGlyphAtlas::GlyphInfo* glyph = nullptr;

	for (const char* p = pStr; *p; p++)
	{
		glyph = &(pFont->GetGlyphAtlas()->glyph[*p]);

		width += glyph->ax * sx;
		height = AEMax( glyph->bh * sy , height);
	}

    if (out_width) (*out_width) = width;
    if (out_height) (*out_height) = height;
}

// ---------------------------------------------------------------------------

AE_API void AEGfxFontSystemStart()
{
	if (FT_Init_FreeType(&gFreetypeLibrary))
		HALT("Freetype not loaded");

	// Setting up the VertexBufferObject
	glGenBuffers(1, &gVBOFont);	
}

// ---------------------------------------------------------------------------

AE_API void AEGfxFontSystemEnd()
{
	glDeleteBuffers(1, &gVBOFont);
	
	if (AEFont::GetRoot() != nullptr)
		AEFont::CleanUp();
	
	FT_Done_FreeType(gFreetypeLibrary);
}



