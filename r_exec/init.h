//	init.h
//
//	Author: Eric Nivel
//
//	BSD license:
//	Copyright (c) 2010, Eric Nivel
//	All rights reserved.
//	Redistribution and use in source and binary forms, with or without
//	modification, are permitted provided that the following conditions are met:
//
//   - Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//   - Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
//   - Neither the name of Eric Nivel nor the
//     names of their contributors may be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
//	THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
//	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//	DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
//	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef	init_h
#define	init_h

#include	"CoreLibrary/utils.h"

#include	"r_code/list.h"

#include	"r_comp/segments.h"
#include	"r_comp/compiler.h"
#include	"r_comp/preprocessor.h"

#include	"dll.h"


namespace	r_exec{

	//	Time base; either Time::Get or network-aware synced time.
	extern	r_exec_dll	uint64				(*Now)();

	//	Loaded once for all.
	//	Results from the compilation of user.classes.replicode.
	//	The latter contains all class definitions and all shared objects (e.g. ontology); does not contain any dynamic (res!=forever) objects.
	extern	r_exec_dll	r_comp::Metadata	Metadata;
	extern	r_exec_dll	r_comp::Image		Seed;

	//	A preprocessor and a compiler are maintained throughout the life of the dll to retain, respectively, macros and global references.
	//	Both functions add the compiled object to Seed.code_image.
	//	Source files: use ANSI encoding (not Unicode).
	bool	r_exec_dll	Compile(const	char	*filename,std::string	&error);
	bool	r_exec_dll	Compile(std::istream	&source_code,std::string	&error);
	
	//	Threaded decompiler, for decompiling on the fly asynchronously.
	//	Named objects are referenced but not decompiled.
	class	r_exec_dll	TDecompiler:
	public	_Object{
	private:
		static	const	uint32	ObjectsInitialSize=16;
		static	thread_ret thread_function_call	Decompile(void	*args);

		class	_Thread:
		public	Thread{
		};

					_Thread	*_thread;
		volatile	uint32	spawned;

		r_code::list<P<Code> >	objects;

		uint32	ostream_id;	// 0 is std::cout.

		std::string	header;
	public:
		TDecompiler(uint32	ostream_id,std::string	header);
		~TDecompiler();

		void	add_object(Code	*object);
		void	add_objects(const	r_code::list<P<Code> >	&objects);
		void	add_objects(const	std::vector<P<Code> >	&objects);
		void	decompile();
	};

	// Spawns an instance of output_window.exe (see output_window project) and opens a pipe between the main process and output_window.
	// Temporary solution:
	// (a) not portable,
	// (b) shall be defined in CoreLibrary instead of here,
	// (c) the stream pool management (PipeOStream::Open(), PipeOStream::Close() and PipeOStream::Get()) shall be decoupled from this implementation (it's an IDE feature),
	// (d) PipeOStream shall be based on std::ostringstream instead of std::ostream with a refined std::stringbuf (override sync() to write in the pipe).
	class	r_exec_dll	PipeOStream:
	public	std::ostream{
	private:
		static	std::vector<PipeOStream	*>	Streams;
		static	PipeOStream					NullStream;

        int m_pipeRead[2];
        int m_pipeWrite[2];
		//HANDLE	pipe_read;
		//HANDLE	pipe_write;

		void	init();	// create one child process and a pipe.
		PipeOStream();
	public:
		static	void	Open(uint8	count);		// open count streams.
		static	void	Close();				// close all streams.
		static	PipeOStream	&Get(uint8	id);	// return NullStream if id is out of range.

		~PipeOStream();

		PipeOStream	&operator	<<(std::string	&s);
		PipeOStream	&operator	<<(const	char	*s);
	};

	//	Initialize Now, compile user.classes.replicode, builds the Seed and loads the user-defined operators.
	//	Return false in case of a problem (e.g. file not found, operator not found, etc.).
	bool	r_exec_dll	Init(const	char	*user_operator_library_path,
							uint64			(*time_base)(),
							const	char	*seed_path);

	//	Alternate taking a ready-made metadata and seed (will be copied into Metadata and Seed).
	bool	r_exec_dll	Init(const	char				*user_operator_library_path,
							uint64						(*time_base)(),
							const	r_comp::Metadata	&metadata,
							const	r_comp::Image		&seed);

	uint16	r_exec_dll	GetOpcode(const	char	*name);	//	classes, operators and functions.

	std::string	r_exec_dll	GetAxiomName(const	uint16	index);	//	for constant objects (ex: self, position, and other axioms).
}


#endif
