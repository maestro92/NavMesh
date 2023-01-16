
##################################################################################
############################### Render Pipeline ##################################
##################################################################################

-	#, Stringizing operator 
this converts the argument you passed in into a string.

for example 

		#define stringer( x )    cout << #x << endl;

		stringer( abcdefg )

the preprocessor produces 

		cout << "abcdefg" << endl;

as mentioned above, it converts what you passed in into strings.



-	##, Token-pastin operator




				SDLGetOpenGLFunction(glBufferData);
	------->	SDLGetOpenGLFunction(glActiveTexture);
				SDLGetOpenGLFunction(glGetStringi);



I got an error saying "glActiveTexture": redefinition; previous definition was 'function'.

I assume the error is because glActiveTexture is not an OpenGL extension function. It already
exists in OpenGL 1.1. Hence we didnt need to GetProcAddress it. 

same thing for 

				OpenGLGlobalFunction(glTexImage3D)
				OpenGLGlobalFunction(glTexSubImage3D)








################################################################
################## OpenGL 3.2 vs OpenGL 3.3 ####################
################################################################

so I was having trouble to render anything on the screen with OpenGL. 
So what I tried is that I straight up went to http://ogldev.atspace.co.uk/
tutorial 2, just to render a point, and I coudlnt even render a point out.

the rendering code is essentially 

				glDrawArrays(GL_POINTS, 0, 1);



So one difference I noticed is that in my code I have 				

				// Using 3.3 OpenGL
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

				//SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

and tutorial 2 from the link above had

				// Using 3.3 OpenGL
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

				//SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);


so apparently there is some changes with version 3.3.

as a matter of fact i couldnt even to render with SDL_GL_CONTEXT_MINOR_VERSION 2


so the idea is to find the difference between OpenGL 3.1 and OpenGL 3.2




so there is two concepts here: OpenGL Context and OpenGL Profile.
https://www.khronos.org/opengl/wiki/OpenGL_Context


so lets first take a look at the OpenGL 3.3 specs:
https://www.khronos.org/registry/OpenGL/specs/gl/glspec33.core.pdf

if you look at Appendix E, it says many features are marked as deprecated in OpenGL 3.0
and to enforce people to write modern openGL, Deprecated features are expected to be completely
removed from a future version of OpenGL

		so it is possible to create an OpenGL 3.0 context which does not support deprecated features. 
		you would want to create a forward compatible context, while a context supporting all OpenGL 3.0
		features is called a full context. 

		OpenGL 3.2 is the first version of OpenGL to define multiple profiles. There is core and compatible profiles.





you use the combination of context and profile to determine the compatbility and API allowed in the program.
so this link actually provides an explanation of the history of context and profile: 
https://sites.google.com/site/gsucomputergraphics/educational/initialization-tasks-in-an-opengl-program/opengl-context-and-profile




until version 3.0, every OpenGL is backwards compatible. OpenGL 1.1 code runs fine on OpenGL 2.1 code.
OpenGL 3.0 introduces the idea of deprecating functionality. Essentially it declared a bunch of functions as deprecated 

OpenGL 3.1 actually removed almost all of functions that was declared deprecated in OpenGL 3.0.
		
OpenGL 3.2 introduces a new context creation mechanism. Since later versions could remove functionality, it was important 
that the user be able to ask for a specific version of OpenGL. Essentially they came up with the concept of profile 
in OpenGL 3.2. Developers can use the combination of an OpenGL context and profile to declare the program_s hardware 
compatibility and whether deprecated APIs are present or allowed in this program. 


so below is my understanding:

so all OpenGL functions has like 3 status 

-	current
-	deprecated
-	removed 

usually deprecated or removed APIs are from older versions of OpenGL, but marked as deprecated or removed in the current version. 


when you create a context, you are creating it with a version.
So for instance, you are creating a OpenGL 3.2 context, this will give you access all the OpenGL 3.2 API.
if you create an OpenGL 3.3 context, this will give you access to all the OpenGL 3.3 API. 

whether you want only want the 

OpenGL 3.2 current API + deprecated API 
or OpenGL 3.2 current API + deprecated API + removed API 

you control that through the Profile. 
So if you created a compatibility profile, you will get current API + deprecated API + removed API 
so if you creted a core profile, you will get current API + deprecated API 


in the case that you only want to write your code with current API.
what you actually do is to create a forward compatible context. 
Its actually just create a normal context, but you set the "forward compatiblity" bit set. 
what this does is that it marks any profile, all functionality marked as "deprecated" to be removed.
that way you can only use the OpenGL 3.2 current API. Not even the deprecated API is allowed

ofcourse there is also the possibility that you can query newer OpenGL APIs as extensions



-	Lets try to understand it in the context of my code 

so if you look at the the SDL function at the link https://wiki.libsdl.org/SDL_GLprofile

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

you can see three options 

		SDL_GL_CONTEXT_PROFILE_CORE:			OpenGL core profile - deprecated functions are disabled

		SDL_GL_CONTEXT_PROFILE_COMPATIBILITY:	OpenGL compatibility profile - deprecated functions are allowed

		SDL_GL_CONTEXT_PROFILE_ES:				OpenGL ES profile - only a subset of the base OpenGL functionality is available

that pretty much agrees with what the specs says. 

Again the code I am comparing is 

				SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

				//SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

and

				// Using 3.3 OpenGL
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

				//SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
				SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);


so my guess is that there I am using some function that was deprecated in 3.1, but actually removed in 3.2,
and its causing my code to not render my dot. 

so what I want to do is to debug. Luckily, OpenGL actually provides debugging function that tells you which functions are in trouble
so this is essentially recreated the handmade hero day 372.

so you want to get type_glDebugMessageCallbackARB(); to work on your machine. 

Another thing is that you want to set 

				SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

properly for the debugging to work.




##############################################################
################ Mysterious Error ############################
##############################################################

so I got an error saying:
		
		GL_INVALID_OPERATION error generated. Invalid VAO/VBO/pointer usage.


https://stackoverflow.com/questions/13403807/glvertexattribpointer-raising-gl-invalid-operation

turns out I have to make a dummy VAO


still when I set my code back to version 3.2, it still wouldnt draw the point.
I am starting to suspect that you have to 


I attempted 

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

and it draws the point. 

but If I have 

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);


it wont draw the dot. So im confused.


I either stick to version 3.1
or stick to 3.2 with compatibility mode. which I dont want.

--	Update
as it turns out https://stackoverflow.com/questions/13107393/my-triangle-doesnt-render-when-i-use-opengl-core-profile-3-2
it seems like you need to have a vertex and fragment shader when using 3.2 core profile. You cant render without them.


