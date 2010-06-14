//	io_register.h
//
//	Author: Eric Nivel
//
//	BSD license:
//	Copyright (c) 2008, Eric Nivel
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

#ifndef	io_register_h
#define	io_register_h

#include	"object.h"


class	InputCode;
class	OutputCode;

class	IORegister{
private:
	typedef	r_code::Object	*(*ObjectBuilder)(InputCode	*);
	typedef	OutputCode		*(*CommandBuilder)(r_code::Object	*);

	static	UNORDERED_MAP<uint32,IORegister>	Register;	//	indexed by opcodes

	static	uint32	LoadClasses();
	static	uint32	ClassLoader;
	template<class	C>	static	uint32	LoadInputClass();
	template<class	C>	static	uint32	LoadOutputClass();

	ObjectBuilder	objectBuilder;	//	executed on the rMem nodes
	CommandBuilder	commandBuilder;	//	executed on the rMem nodes
public:
	static	r_code::Object	*GetObject(InputCode	*input);	//	invoked upon reception by the rMem
	static	OutputCode	*GetCommand(r_code::Object	*object);	//	invoked upon ejection by the rMem of a command to a device
	static	uint32	GetOpcode(const	char	*class_name);
	static	UNORDERED_MAP<std::string,r_code::Atom>	Classes;	//	opcodes retrieved by name
};


#include	"io_register.tpl.cpp"


#endif